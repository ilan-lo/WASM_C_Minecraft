#pragma once
#include <SDL2/SDL.h>

typedef struct {
    SDL_Window   *window;
    SDL_GLContext gl_ctx;
    uint64_t      last_ticks;
    float         dt;
} Platform;

int  platform_init(Platform *p, int w, int h, const char *title);
void platform_begin_frame(Platform *p);
void platform_swap(Platform *p);
void platform_free(Platform *p);
