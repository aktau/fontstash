//
// Copyright (c) 2009 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "mio_util.h"
#include "mio_stash.h"

#define LINEAR_FIRST

#ifdef LINEAR_FIRST
#define CLEAR_R 0.55f
#define CLEAR_G 0.55f
#define CLEAR_B 0.57f
#define CLEAR_A 1.0f
#else
#define CLEAR_R 0.3f
#define CLEAR_G 0.3f
#define CLEAR_B 0.32f
#define CLEAR_A 1.0f
#endif

#define CLEAR_COLOR CLEAR_R, CLEAR_G, CLEAR_B, CLEAR_A

static float xtranslate = 0.0f;
static float ytranslate = 0.0f;
static int alt          = 0;
static int multisample  = 0;
static int srgb         = 0;
static int premul       = 1;
static float scale      = 1.0f;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
            case GLFW_KEY_LEFT: xtranslate -= 5.f; break;
            case GLFW_KEY_RIGHT: xtranslate += 5.f; break;
            case GLFW_KEY_DOWN: ytranslate -= 5.f; break;
            case GLFW_KEY_UP: ytranslate += 5.f; break;
            case GLFW_KEY_A:
                alt = !alt;
                printf("alt is now %d\n", alt);
                break;
            case GLFW_KEY_M:
                multisample = !multisample;
                printf("multisample is now %d\n", multisample);
                if (multisample) glEnable(GL_MULTISAMPLE);
                else glDisable(GL_MULTISAMPLE);
                break;
            case GLFW_KEY_S:
                srgb = !srgb;
                printf("srgb is now %d\n", srgb);
                if (srgb) {
                    printf("clear specified as linearRGB(%3.2f, %3.2f, %3.2f, %3.2f), will become sRGB(%3.2f, %3.2f, %3.2f, %3.2f)\n", CLEAR_COLOR, SRGBA(CLEAR_R, CLEAR_G, CLEAR_B, CLEAR_A));
                }
                else {
                    printf("clear specified as sRGB(%3.2f, %3.2f, %3.2f, %3.2f), should be linearized lRGB(%3.2f, %3.2f, %3.2f, %3.2f)\n", CLEAR_COLOR, LRGBA(CLEAR_R, CLEAR_G, CLEAR_B, CLEAR_A));
                }
                if (srgb) glEnable(GL_FRAMEBUFFER_SRGB);
                else glDisable(GL_FRAMEBUFFER_SRGB);
                break;
            case GLFW_KEY_P:
                premul = !premul;
                printf("premul is now %d\n", premul);
                if (premul) glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case GLFW_KEY_EQUAL:
            case GLFW_KEY_MINUS:
                scale += (key == GLFW_KEY_EQUAL) ? 0.1f : -0.1f;
                printf("scale is now %f\n", scale);
                break;
            default:
                printf("Unknown key: %d, scancode: %d, mods: %d\n", key, scancode, mods);
        }
    }
}

static void render_grid(struct font *font, float color, int width, int height) {
    text_set_font(font, 16.0f);

    int count = 10;
    char buffer[512];

    for (int i = 0; i < width; i += width / count) {
        for (int j = 0; j < height; j += height / count) {
            snprintf(buffer, 512, "(%3d,%3d)", i, j);
            text_show(i, j, buffer);
        }
    }
}

static void render_text(struct font *font, mat4 clip_from_view, mat4 view_from_world, float scale, float color, int width, int height, int alt, float xoffset, float yoffset) {
    text_begin(clip_from_view, view_from_world, alt);
    text_set_color(color, color, color, 1);
    text_set_font(font, floorf(200.0f * scale));

    text_show(100 + xoffset, 300 + yoffset, "The quick brown fox");

    render_grid(font, color, width, height);

    text_end();
}

int main() {
    int width = 640, height = 480;

    enum {
        FONT_NORMAL = 0,
        FONT_ITALIC = 1,
        FONT_BOLD = 2,
        FONT_JAPANESE = 3,
    };

    GLFWwindow* window;
    const GLFWvidmode* mode;
    struct fontstash* stash = NULL;
    struct glstash* gl = NULL;

    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // glfwWindowHint(GLFW_SAMPLES, 16);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    window = glfwCreateWindow(width, height, "(Mio) Font Stash", NULL, NULL);
    glfwSetWindowPos(window, mode->width / 2 + 20, mode->height / 2 - height / 2);

    glfwMakeContextCurrent(window);

    struct font *font = load_font("fonts/DroidSerif-Regular.ttf");

    // glEnable(GL_MULTISAMPLE);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);

    /* premultiplied alpha by default */
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {
        float sx = 100, sy = 100, dx, dy, t, lh = 0;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(CLEAR_COLOR);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float far = 1.0f, near = -1.0f;

        /* projection matrix */
        mat4 clip_from_view = {
           2.f / (float) width, 0, 0, 0,
           0, -2.f / (float) height, 0, 0,
           0, 0, -2.f/(far - near), 0,
           -1, 1, -(far + near)/(far - near), 1
        };

        /* modelview matrix */
        mat4 view_from_world = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            -xtranslate, -ytranslate, 0, 1
        };

        dx = sx; dy = sy;

        render_text(font, clip_from_view, view_from_world, scale, 1.0f, width, height, alt, 0, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_font(font);

    glfwTerminate();
    return 0;
}
