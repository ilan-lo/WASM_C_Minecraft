#include <stdio.h>
#include "platform.h"
#include "input.h"
#include "blocks.h"
#include "world.h"
#include "player.h"
#include "renderer.h"

#ifdef VOXEL_WEB
#include <emscripten.h>
#endif

static Platform  s_platform;
static World     s_world;
static Renderer  s_renderer;
static Player    s_player;

static void frame(void) {
    platform_begin_frame(&s_platform);
    if (g_quit) {
#ifdef VOXEL_WEB
        emscripten_cancel_main_loop();
#endif
        return;
    }
    if (g_reload_shaders) renderer_reload_shaders(&s_renderer);
    world_update(&s_world, s_player.body.pos.x, s_player.body.pos.z);
    player_update(&s_player, &s_world, s_platform.dt);
    renderer_render(&s_renderer, &s_world, &s_player,
                    g_window_w, g_window_h, s_platform.dt);
    platform_swap(&s_platform);
}

int main(void) {
    if (!platform_init(&s_platform, 1280, 720, "Voxel")) return 1;

    blocks_init();
    world_init(&s_world);
    world_update(&s_world, 0, 0);
    player_init(&s_player, 0, 80, 0);

    if (!renderer_init(&s_renderer)) {
        fprintf(stderr, "Renderer init failed\n");
        return 1;
    }

#ifdef VOXEL_WEB
    /* Browser controls the loop — 0 = use requestAnimationFrame rate, 1 = simulate infinite loop */
    emscripten_set_main_loop(frame, 0, 1);
#else
    while (!g_quit) frame();
    renderer_free(&s_renderer);
    platform_free(&s_platform);
#endif
    return 0;
}
