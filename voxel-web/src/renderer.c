#include "renderer.h"
#include "mesh.h"
#include "blocks.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stddef.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* ---- shader loading ---- */

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

static GLuint compile_shader(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "Shader error: %s\n", log);
        glDeleteShader(s); return 0;
    }
    return s;
}

static GLuint load_program(const char *vert_path, const char *frag_path) {
    char *vs = read_file(vert_path);
    char *fs = read_file(frag_path);
    if (!vs || !fs) { free(vs); free(fs); return 0; }
    GLuint v = compile_shader(GL_VERTEX_SHADER, vs);
    GLuint f = compile_shader(GL_FRAGMENT_SHADER, fs);
    free(vs); free(fs);
    if (!v || !f) { glDeleteShader(v); glDeleteShader(f); return 0; }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v); glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v); glDeleteShader(f);
    GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "Link error: %s\n", log);
        glDeleteProgram(prog); return 0;
    }
    return prog;
}

/* ---- atlas texture ---- */

static GLuint load_atlas(const char *path, int *tiles_per_row) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &w, &h, &ch, 4);
    if (!data) {
        fprintf(stderr, "Cannot load atlas %s — generating placeholder\n", path);
        /* 16x16 tiles, 16 tiles wide = 256x256 RGBA placeholder */
        int aw = 256, ah = 256;
        data = calloc(aw * ah * 4, 1);
        /* fill each tile with a distinct color */
        static const unsigned char palette[][4] = {
            {255,0,255,255},    /* 0 error/magenta */
            {100,160,70,255},   /* 1 grass top */
            {100,160,70,255},   /* 2 grass side (tinted) */
            {120,80,40,255},    /* 3 dirt */
            {130,130,130,255},  /* 4 stone */
            {160,100,50,255},   /* 5 wood top */
            {140,90,45,255},    /* 6 wood side */
            {60,160,60,200},    /* 7 leaves */
            {210,195,140,255},  /* 8 sand */
            {80,80,80,255},     /* 9 bedrock */
            {60,100,200,180},   /* 10 water */
        };
        int tile_px = 16;
        for (int ti = 0; ti < 11; ti++) {
            int tx = (ti % 16) * tile_px;
            int ty = (ti / 16) * tile_px;
            for (int py = 0; py < tile_px; py++)
            for (int px2 = 0; px2 < tile_px; px2++) {
                int idx = ((ty + py) * aw + (tx + px2)) * 4;
                /* add some pixel noise */
                int r = palette[ti][0] + (px2+py)%20 - 10;
                int g = palette[ti][1] + (px2*py)%15 - 7;
                int b = palette[ti][2] + px2%10 - 5;
                data[idx+0] = (unsigned char)(r<0?0:r>255?255:r);
                data[idx+1] = (unsigned char)(g<0?0:g>255?255:g);
                data[idx+2] = (unsigned char)(b<0?0:b>255?255:b);
                data[idx+3] = palette[ti][3];
            }
        }
        w = aw; h = ah; ch = 4;
        *tiles_per_row = 16;
    } else {
        *tiles_per_row = w / 16;  /* assume 16px tiles */
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(data);
    return tex;
}

/* ---- UI VAO setup ---- */

typedef struct { float x, y, u, v, r, g, b, a; } UIVert;

static void ui_vao_init(Renderer *r) {
    glGenVertexArrays(1, &r->ui_vao);
    glGenBuffers(1, &r->ui_vbo);
    glBindVertexArray(r->ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->ui_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVert), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVert), (void*)8);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVert), (void*)16);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

static void cache_uniforms(Renderer *r) {
    r->u_mvp         = glGetUniformLocation(r->world_prog, "u_mvp");
    r->u_mv          = glGetUniformLocation(r->world_prog, "u_mv");
    r->u_chunk_offset= glGetUniformLocation(r->world_prog, "u_chunk_offset");
    r->u_atlas_size  = glGetUniformLocation(r->world_prog, "u_atlas_size");
    r->u_atlas       = glGetUniformLocation(r->world_prog, "u_atlas");
    r->u_fog_color   = glGetUniformLocation(r->world_prog, "u_fog_color");
    r->u_fog_start   = glGetUniformLocation(r->world_prog, "u_fog_start");
    r->u_fog_end     = glGetUniformLocation(r->world_prog, "u_fog_end");
    r->u_ui_proj     = glGetUniformLocation(r->ui_prog,    "u_proj");
    r->u_ui_tex      = glGetUniformLocation(r->ui_prog,    "u_tex");
    r->u_ui_use_tex  = glGetUniformLocation(r->ui_prog,    "u_use_tex");
}

/* ---- public API ---- */

int renderer_init(Renderer *r) {
    memset(r, 0, sizeof(Renderer));
    r->world_prog = load_program("assets/shaders/world.vert", "assets/shaders/world.frag");
    r->ui_prog    = load_program("assets/shaders/ui.vert",   "assets/shaders/ui.frag");
    if (!r->world_prog || !r->ui_prog) return 0;
    r->atlas_tex  = load_atlas("assets/textures/atlas.png", &r->atlas_size);
    cache_uniforms(r);
    ui_vao_init(r);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return 1;
}

void renderer_reload_shaders(Renderer *r) {
    GLuint wp = load_program("assets/shaders/world.vert", "assets/shaders/world.frag");
    GLuint up = load_program("assets/shaders/ui.vert",    "assets/shaders/ui.frag");
    if (wp) { glDeleteProgram(r->world_prog); r->world_prog = wp; }
    if (up) { glDeleteProgram(r->ui_prog);    r->ui_prog = up; }
    cache_uniforms(r);
    fprintf(stderr, "Shaders reloaded.\n");
}

static void draw_ui(Renderer *r, Player *p, int win_w, int win_h) {
    glUseProgram(r->ui_prog);
    mat4 proj = mat4_ortho(0, (float)win_w, 0, (float)win_h, -1, 1);
    glUniformMatrix4fv(r->u_ui_proj, 1, GL_FALSE, proj.m);
    glUniform1i(r->u_ui_use_tex, 0);
    glDisable(GL_DEPTH_TEST);

    /* crosshair */
    float cx = win_w * 0.5f, cy = win_h * 0.5f;
    float cs = 10.f;
    UIVert verts[24];
    int n = 0;
#define QUAD(x0,y0,x1,y1,ri,gi,bi,ai) \
    verts[n++]=(UIVert){x0,y0,0,0,ri,gi,bi,ai}; \
    verts[n++]=(UIVert){x1,y0,0,0,ri,gi,bi,ai}; \
    verts[n++]=(UIVert){x1,y1,0,0,ri,gi,bi,ai}; \
    verts[n++]=(UIVert){x0,y0,0,0,ri,gi,bi,ai}; \
    verts[n++]=(UIVert){x1,y1,0,0,ri,gi,bi,ai}; \
    verts[n++]=(UIVert){x0,y1,0,0,ri,gi,bi,ai};

    QUAD(cx-cs, cy-1.5f, cx+cs, cy+1.5f, 1,1,1,0.8f)   /* horizontal */
    QUAD(cx-1.5f, cy-cs, cx+1.5f, cy+cs, 1,1,1,0.8f)   /* vertical */

    /* hotbar background */
    float hbar_w = HOTBAR_SLOTS * 42.f;
    float hbar_x = (win_w - hbar_w) * 0.5f;
    float hbar_y = 10.f;
    QUAD(hbar_x, hbar_y, hbar_x+hbar_w, hbar_y+42.f, 0,0,0,0.4f)

    /* selected slot highlight */
    float sx = hbar_x + p->slot * 42.f;
    QUAD(sx, hbar_y, sx+42.f, hbar_y+42.f, 1,1,1,0.3f)
#undef QUAD

    glBindVertexArray(r->ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->ui_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UIVert)*n, verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, n);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void renderer_render(Renderer *r, World *w, Player *p, int win_w, int win_h, float dt) {
    (void)dt;
    /* sky color */
    static const float sky[] = {0.53f, 0.81f, 0.98f};
    glClearColor(sky[0], sky[1], sky[2], 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, win_w, win_h);

    /* rebuild one dirty chunk per frame */
    for (int i = 0; i < w->pool_used; i++) {
        Chunk *c = &w->pool[i];
        if (c->loaded && c->dirty) { mesh_build(c, w); break; }
    }

    /* view + projection */
    vec3 eye = player_eye(p);
    vec3 fwd = player_forward(p);
    vec3 center = vec3_add(eye, fwd);
    vec3 up = {0,1,0};
    mat4 view = mat4_lookat(eye, center, up);
    mat4 proj = mat4_perspective(RAD(70.f), (float)win_w / (float)win_h, 0.05f, 1000.f);

    glUseProgram(r->world_prog);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->atlas_tex);
    glUniform1i(r->u_atlas, 0);
    glUniform1i(r->u_atlas_size, r->atlas_size);
    glUniform3f(r->u_fog_color, sky[0], sky[1], sky[2]);
    glUniform1f(r->u_fog_start, (RENDER_DISTANCE - 2) * CHUNK_W * 0.8f);
    glUniform1f(r->u_fog_end,    RENDER_DISTANCE       * CHUNK_W * 0.9f);

    /* draw all loaded chunks */
    for (int i = 0; i < w->pool_used; i++) {
        Chunk *c = &w->pool[i];
        if (!c->loaded || !c->mesh_ready || c->vertex_count == 0) continue;

        vec3 offset = {
            (float)(c->cx * CHUNK_W),
            0.f,
            (float)(c->cz * CHUNK_D)
        };
        mat4 mvp = mat4_mul(proj, view);
        glUniformMatrix4fv(r->u_mvp, 1, GL_FALSE, mvp.m);
        glUniformMatrix4fv(r->u_mv,  1, GL_FALSE, view.m);
        glUniform3f(r->u_chunk_offset, offset.x, offset.y, offset.z);

        glBindVertexArray(c->vao);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_count);
    }
    glBindVertexArray(0);

    draw_ui(r, p, win_w, win_h);
}

void renderer_free(Renderer *r) {
    glDeleteProgram(r->world_prog);
    glDeleteProgram(r->ui_prog);
    glDeleteTextures(1, &r->atlas_tex);
    glDeleteVertexArrays(1, &r->ui_vao);
    glDeleteBuffers(1, &r->ui_vbo);
}
