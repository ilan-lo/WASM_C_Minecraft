#pragma once
#include "math.h"
#include "world.h"

typedef struct {
    vec3    pos;        /* center bottom of AABB */
    vec3    vel;
    vec3    half_ext;   /* half-widths: default {0.3, 0.9, 0.3} */
    int     on_ground;
} RigidBody;

void physics_update(RigidBody *body, World *w, float dt);
