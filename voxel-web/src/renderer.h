#pragma once
#ifdef VOXEL_WEB
#include <GLES3/gl3.h>
#else
#include <glad/gl.h>
#endif
#include "math.h"
#include "world.h"
#include "player.h"
#include "minimap.h"

typedef struct {
    GLuint world_prog;
    GLuint ui_prog;
    GLuint atlas_tex;
    int    atlas_size;    /* tiles per row */

    /* UI geometry (crosshair, hotbar) */
    GLuint ui_vao, ui_vbo;

    /* uniforms cache */
    GLint  u_mvp, u_mv, u_chunk_offset, u_atlas_size, u_atlas, u_fog_color, u_fog_start, u_fog_end;
    GLint  u_ui_proj, u_ui_tex, u_ui_use_tex;

    Minimap minimap;
} Renderer;

int  renderer_init(Renderer *r);
void renderer_reload_shaders(Renderer *r);
void renderer_render(Renderer *r, World *w, Player *p, int win_w, int win_h, float dt);
void renderer_free(Renderer *r);
