#include "physics.h"
#include "blocks.h"
#include <math.h>

#define GRAVITY      -28.0f
#define MAX_FALL_VEL -50.0f

/* Check if AABB at 'pos' with half_ext overlaps any solid block */
static int aabb_overlaps_world(World *w, vec3 pos, vec3 half) {
    int x0 = (int)floorf(pos.x - half.x);
    int x1 = (int)floorf(pos.x + half.x);
    int y0 = (int)floorf(pos.y);
    int y1 = (int)floorf(pos.y + half.y * 2.f - 0.001f);
    int z0 = (int)floorf(pos.z - half.z);
    int z1 = (int)floorf(pos.z + half.z);
    for (int y = y0; y <= y1; y++)
    for (int x = x0; x <= x1; x++)
    for (int z = z0; z <= z1; z++) {
        uint8_t id = world_get_block(w, x, y, z);
        if (block_is_solid(id) && !block_is_transparent(id))
            return 1;
    }
    return 0;
}

void physics_update(RigidBody *body, World *w, float dt) {
    /* gravity */
    body->vel.y += GRAVITY * dt;
    if (body->vel.y < MAX_FALL_VEL) body->vel.y = MAX_FALL_VEL;

    body->on_ground = 0;

    /* resolve each axis independently */
    /* Y */
    {
        vec3 np = (vec3){body->pos.x, body->pos.y + body->vel.y * dt, body->pos.z};
        if (!aabb_overlaps_world(w, np, body->half_ext)) {
            body->pos.y = np.y;
        } else {
            if (body->vel.y < 0) body->on_ground = 1;
            body->vel.y = 0.f;
        }
    }
    /* X */
    {
        vec3 np = (vec3){body->pos.x + body->vel.x * dt, body->pos.y, body->pos.z};
        if (!aabb_overlaps_world(w, np, body->half_ext))
            body->pos.x = np.x;
        else
            body->vel.x = 0.f;
    }
    /* Z */
    {
        vec3 np = (vec3){body->pos.x, body->pos.y, body->pos.z + body->vel.z * dt};
        if (!aabb_overlaps_world(w, np, body->half_ext))
            body->pos.z = np.z;
        else
            body->vel.z = 0.f;
    }
}
