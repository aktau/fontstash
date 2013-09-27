#include "mio_util.h"

struct entry {
    char *name;
    int offset;
};

struct archive {
    FILE *file;
    int count;
    struct entry *table;
    struct archive *next;
};

static inline int getshort(FILE *file)
{
    int a = getc(file);
    int b = getc(file);
    return a | b << 8;
}

static inline int getlong(FILE *file)
{
    int a = getc(file);
    int b = getc(file);
    int c = getc(file);
    int d = getc(file);
    return a | b << 8 | c << 16 | d << 24;
}

static int cmpentry(const void *a_, const void *b_)
{
    const struct entry *a = a_;
    const struct entry *b = b_;
    return strcmp(a->name, b->name);
}

unsigned char *read_file(const char *filename, int *lenp)
{
    unsigned char *data;
    int len;
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, 2);
    len = ftell(file);
    fseek(file, 0, 0);
    data = malloc(len);
    fread(data, 1, len, file);
    fclose(file);
    if (lenp) *lenp = len;
    return data;
}

/*
 * Virtual filesystem -- look for files in registered directories and archives.
 */

struct directory
{
    char *name;
    struct directory *next;
};

static struct directory *dir_head = NULL;
static struct archive *zip_head = NULL;

unsigned char *load_file(const char *filename, int *lenp)
{
    struct directory *dir = dir_head;
    struct archive *zip = zip_head;
    unsigned char *data;
    char buf[512];

    data = read_file(filename, lenp);

    while (!data && dir) {
        strlcpy(buf, dir->name, sizeof buf);
        strlcat(buf, filename, sizeof buf);
        data = read_file(buf, lenp);
        dir = dir->next;
    }

    return data;
}
