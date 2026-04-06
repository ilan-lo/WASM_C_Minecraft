#pragma once
#ifdef VOXEL_WEB
#include <GLES3/gl3.h>
#else
#include <glad/gl.h>
#endif
#include "world.h"
#include "player.h"

#define MM_BLOCKS  128   /* world blocks covered (radius = MM_BLOCKS/2) */
#define MM_PIXELS  128   /* texture resolution */
#define MM_SCREEN  160   /* rendered size in screen pixels */

typedef struct {
    GLuint tex;
    GLuint vao, vbo;
    uint32_t pixels[MM_PIXELS * MM_PIXELS];  /* RGBA scratch buffer */
    float last_px, last_pz;                  /* position at last update */
} Minimap;

void minimap_init(Minimap *m);
void minimap_update(Minimap *m, World *w, Player *p);
void minimap_render(Minimap *m, int win_w, int win_h);
