#ifndef MIO_INCL_UTIL
#define MIO_INCL_UTIL

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#define TAG_FONT 'F'

#define strsep xstrsep
#define strlcpy xstrlcpy
#define strlcat xstrlcat

#define SLUM(x) ((x)*(x)) /* approximate sRGB to Linear conversion */
#define SRGB(r,g,b) SLUM(r),SLUM(g),SLUM(b)
#define SRGBA(r,g,b,a) SRGB(r,g,b),(a)
#define LRGB(r,g,b) sqrtf(r),sqrtf(g),sqrtf(b)
#define LRGBA(r,g,b,a) LRGB(r,g,b),(a)

typedef float mat4[16];

#undef nelem
#define nelem(x) (sizeof(x)/sizeof(x)[0])

int chartorune(int *rune, char *str);
int runetochar(char *str, int *rune);

/* archive data file loading */

unsigned char *load_file(const char *filename, int *lenp);

/* resource cache */

struct cache;

void *lookup(struct cache *cache, const char *key);
struct cache *insert(struct cache *cache, const char *key, void *value);
void print_cache(struct cache *cache);

/* shaders */

enum {
    ATT_POSITION,
    ATT_NORMAL,
    ATT_TANGENT,
    ATT_TEXCOORD,
    ATT_COLOR,
    ATT_BLEND_INDEX,
    ATT_BLEND_WEIGHT,
    ATT_LIGHTMAP,
    ATT_SPLAT,
    ATT_WIND,
};

enum {
    FRAG_COLOR = 0,
    FRAG_NORMAL = 0,
    FRAG_ALBEDO = 1,
};

enum {
    MAP_COLOR = GL_TEXTURE0,
    MAP_GLOSS,
    MAP_NORMAL,
    MAP_SHADOW,
    MAP_DEPTH,
    MAP_EMISSION,
    MAP_LIGHT,
    MAP_SPLAT,
};

int compile_shader(const char *vert_src, const char *frag_src);

static char *xstrsep(char **stringp, const char *delim)
{
    char *ret = *stringp;
    if (ret == NULL) return NULL;
    if ((*stringp = strpbrk(*stringp, delim)) != NULL)
        *((*stringp)++) = '\0';
    return ret;
}

static int xstrlcpy(char *dst, const char *src, int siz)
{
    register char *d = dst;
    register const char *s = src;
    register int n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';      /* NUL-terminate dst */
            while (*s++)
                ;
    }

    return(s - src - 1);    /* count does not include NUL */
}

static int xstrlcat(char *dst, const char *src, int siz)
{
    register char *d = dst;
    register const char *s = src;
    register int n = siz;
    int dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (*d != '\0' && n-- != 0)
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return dlen + strlen(s);
    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return dlen + (s - src);    /* count does not include NUL */
}

#endif
