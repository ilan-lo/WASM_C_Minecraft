#include <stdio.h>
#include "platform.h"
#include "input.h"
#include "blocks.h"
#include "world.h"
#include "worldgen.h"
#include "player.h"
#include "renderer.h"

int main(void) {
    Platform platform;
    if (!platform_init(&platform, 1280, 720, "Voxel")) return 1;

    blocks_init();

    /* Static allocation — avoid large stack frames */
    static World    world;
    static Renderer renderer;
    static Player   player;

    world_init(&world);

    /* Find a good spawn height */
    float spawn_x = 0.f, spawn_z = 0.f;
    /* Pre-generate spawn area so we have height data */
    world_update(&world, spawn_x, spawn_z);
    float spawn_y = 80.f;   /* worldgen centers around ~50-70, 80 is safely above */

    player_init(&player, spawn_x, spawn_y, spawn_z);

    if (!renderer_init(&renderer)) {
        fprintf(stderr, "Renderer init failed\n");
        return 1;
    }

    printf("Controls:\n");
    printf("  WASD       - move\n");
    printf("  Mouse      - look\n");
    printf("  Space      - jump\n");
    printf("  LShift     - sprint\n");
    printf("  LMB        - break block\n");
    printf("  RMB        - place block\n");
    printf("  1-9        - select hotbar slot\n");
    printf("  R          - reload shaders\n");
    printf("  Escape     - quit\n");

    while (!g_quit) {
        platform_begin_frame(&platform);

        if (g_reload_shaders) renderer_reload_shaders(&renderer);

        world_update(&world, player.body.pos.x, player.body.pos.z);
        player_update(&player, &world, platform.dt);
        renderer_render(&renderer, &world, &player,
                        g_window_w, g_window_h, platform.dt);
        platform_swap(&platform);
    }

    renderer_free(&renderer);
    platform_free(&platform);
    return 0;
}
