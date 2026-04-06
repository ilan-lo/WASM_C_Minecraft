#pragma once
#include <SDL2/SDL.h>

typedef enum {
    ACTION_FORWARD = 0,
    ACTION_BACK,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_JUMP,
    ACTION_SPRINT,
    ACTION_PLACE,
    ACTION_BREAK,
    ACTION_SLOT_1, ACTION_SLOT_2, ACTION_SLOT_3, ACTION_SLOT_4,
    ACTION_SLOT_5, ACTION_SLOT_6, ACTION_SLOT_7, ACTION_SLOT_8, ACTION_SLOT_9,
    ACTION_COUNT
} Action;

extern int g_actions[ACTION_COUNT];         /* held this frame */
extern int g_actions_pressed[ACTION_COUNT]; /* pressed this frame only */
extern int g_mouse_dx, g_mouse_dy;
extern int g_mouse_btn[3];       /* left, middle, right — pressed this frame */
extern int g_quit;
extern int g_window_w, g_window_h;
extern int g_reload_shaders;
extern int g_mouse_captured;

void input_capture_mouse(SDL_Window *win, int capture);
void input_begin_frame(void);
void input_process_event(SDL_Event *e);
