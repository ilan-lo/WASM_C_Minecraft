#pragma once
#include <math.h>
#include "math.h"
#include "physics.h"
#include "world.h"
#include "blocks.h"

#define HOTBAR_SLOTS 9
#define PLAYER_SPEED       5.0f
#define PLAYER_SPRINT_MUL  1.6f
#define JUMP_IMPULSE       8.5f
#define MOUSE_SENSITIVITY  0.002f
#define REACH              5.0f

typedef struct {
    RigidBody body;
    float     yaw, pitch;         /* radians */
    vec3      eye_offset;         /* (0, 1.62, 0) */

    /* block targeting */
    int       has_target;
    int32_t   target_x, target_y, target_z;   /* block to break */
    int32_t   place_x,  place_y,  place_z;    /* block to place */

    uint8_t   hotbar[HOTBAR_SLOTS];
    int       slot;
} Player;

void player_init(Player *p, float x, float y, float z);
void player_update(Player *p, World *w, float dt);

/* Returns eye position */
static inline vec3 player_eye(const Player *p) {
    return vec3_add(p->body.pos, p->eye_offset);
}

/* Returns look direction */
static inline vec3 player_forward(const Player *p) {
    return (vec3){
        cosf(p->pitch) * sinf(p->yaw),
        sinf(p->pitch),
       -cosf(p->pitch) * cosf(p->yaw)
    };
}
