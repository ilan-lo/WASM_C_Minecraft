#include "platform.h"
#include "input.h"
#ifdef VOXEL_WEB
#include <GLES3/gl3.h>
#else
#include <glad/gl.h>
#endif
#include <stdio.h>

int platform_init(Platform *p, int w, int h, const char *title) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
#ifdef VOXEL_WEB
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);   /* WebGL2 = GLES 3.0 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    p->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!p->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 0;
    }
    p->gl_ctx = SDL_GL_CreateContext(p->window);
    if (!p->gl_ctx) {
        fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
        return 0;
    }
    SDL_GL_SetSwapInterval(1);

#ifndef VOXEL_WEB
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "gladLoadGL failed\n");
        return 0;
    }
#endif

    input_capture_mouse(p->window, 1);
    p->last_ticks = SDL_GetPerformanceCounter();
    g_window_w = w;
    g_window_h = h;
    return 1;
}

void platform_begin_frame(Platform *p) {
    uint64_t now  = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();
    p->dt = (float)(now - p->last_ticks) / (float)freq;
    if (p->dt > 0.1f) p->dt = 0.1f;
    p->last_ticks = now;

    input_begin_frame();
    SDL_Event e;
    while (SDL_PollEvent(&e)) input_process_event(&e);
}

void platform_swap(Platform *p) {
    SDL_GL_SwapWindow(p->window);
}

void platform_free(Platform *p) {
    SDL_GL_DeleteContext(p->gl_ctx);
    SDL_DestroyWindow(p->window);
    SDL_Quit();
}
