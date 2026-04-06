#include "player.h"
#include "input.h"
#include <math.h>
#include <string.h>

void player_init(Player *p, float x, float y, float z) {
    memset(p, 0, sizeof(Player));
    p->body.pos      = (vec3){x, y, z};
    p->body.half_ext = (vec3){0.3f, 0.9f, 0.3f};
    p->eye_offset    = (vec3){0.f, 1.62f, 0.f};
    p->yaw           = 0.f;
    p->pitch         = 0.f;
    /* default hotbar */
    p->hotbar[0] = BLOCK_GRASS;
    p->hotbar[1] = BLOCK_DIRT;
    p->hotbar[2] = BLOCK_STONE;
    p->hotbar[3] = BLOCK_WOOD;
    p->hotbar[4] = BLOCK_LEAVES;
    p->hotbar[5] = BLOCK_SAND;
    p->hotbar[6] = BLOCK_BEDROCK;
    p->hotbar[7] = BLOCK_WATER;
    p->hotbar[8] = BLOCK_GRASS;
}

/* Ray-cast for block targeting */
static void raycast(Player *p, World *w) {
    p->has_target = 0;
    vec3 eye = player_eye(p);
    vec3 dir = player_forward(p);
    float step = 0.05f;
    int32_t last_x = (int32_t)floorf(eye.x);
    int32_t last_y = (int32_t)floorf(eye.y);
    int32_t last_z = (int32_t)floorf(eye.z);

    for (float t = 0; t < REACH; t += step) {
        float fx = eye.x + dir.x * t;
        float fy = eye.y + dir.y * t;
        float fz = eye.z + dir.z * t;
        int32_t bx = (int32_t)floorf(fx);
        int32_t by = (int32_t)floorf(fy);
        int32_t bz = (int32_t)floorf(fz);

        uint8_t id = world_get_block(w, bx, by, bz);
        if (block_is_solid(id)) {
            p->has_target = 1;
            p->target_x = bx; p->target_y = by; p->target_z = bz;
            p->place_x  = last_x; p->place_y = last_y; p->place_z = last_z;
            return;
        }
        last_x = bx; last_y = by; last_z = bz;
    }
}

void player_update(Player *p, World *w, float dt) {
#ifdef VOXEL_WEB
    p->yaw   += g_mouse_dx * MOUSE_SENSITIVITY;
    p->pitch -= g_mouse_dy * MOUSE_SENSITIVITY;
#else
    p->yaw   -= g_mouse_dx * MOUSE_SENSITIVITY;
    p->pitch += g_mouse_dy * MOUSE_SENSITIVITY;
#endif
    if (p->pitch >  1.5f) p->pitch =  1.5f;
    if (p->pitch < -1.5f) p->pitch = -1.5f;

    /* hotbar slots */
    for (int i = 0; i < HOTBAR_SLOTS; i++)
        if (g_actions_pressed[ACTION_SLOT_1 + i]) p->slot = i;

    /* movement direction */
    float cy = cosf(p->yaw), sy = sinf(p->yaw);
    vec3 fwd  = {sy, 0.f, -cy};
    vec3 right = {cy, 0.f,  sy};

    vec3 move = {0,0,0};
    if (g_actions[ACTION_FORWARD]) move = vec3_add(move, fwd);
    if (g_actions[ACTION_BACK])    move = vec3_sub(move, fwd);
    if (g_actions[ACTION_RIGHT])   move = vec3_add(move, right);
    if (g_actions[ACTION_LEFT])    move = vec3_sub(move, right);

    float speed = PLAYER_SPEED;
    if (g_actions[ACTION_SPRINT]) speed *= PLAYER_SPRINT_MUL;

    float len = vec3_len(move);
    if (len > 0.001f) move = vec3_scale(move, speed / len);

    p->body.vel.x = move.x;
    p->body.vel.z = move.z;

    /* jump */
    if (g_actions_pressed[ACTION_JUMP] && p->body.on_ground)
        p->body.vel.y = JUMP_IMPULSE;

    physics_update(&p->body, w, dt);

    /* block targeting */
    raycast(p, w);

    /* break / place */
    if (g_mouse_btn[0] && p->has_target)
        world_set_block(w, p->target_x, p->target_y, p->target_z, BLOCK_AIR);

    if (g_mouse_btn[2] && p->has_target) {
        uint8_t place_id = p->hotbar[p->slot];
        world_set_block(w, p->place_x, p->place_y, p->place_z, place_id);
    }
}
