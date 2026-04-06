#pragma once
#include <stdint.h>
#include <glad/gl.h>

#define CHUNK_W   16
#define CHUNK_H   128   /* reduced from 256 for faster prototype */
#define CHUNK_D   16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

typedef struct {
    int32_t cx, cz;          /* chunk grid coordinates */
    uint8_t blocks[CHUNK_VOL];
    GLuint  vao, vbo;
    int     vertex_count;
    int     dirty;            /* 1 = needs remeshing */
    int     loaded;
    int     mesh_ready;       /* 1 = VAO is uploaded */
} Chunk;

/* Inline block indexing — Y-major */
static inline int chunk_idx(int x, int y, int z) {
    return y * CHUNK_W * CHUNK_D + z * CHUNK_W + x;
}

static inline uint8_t chunk_get(const Chunk *c, int x, int y, int z) {
    if (x<0||x>=CHUNK_W||y<0||y>=CHUNK_H||z<0||z>=CHUNK_D) return 0;
    return c->blocks[chunk_idx(x,y,z)];
}

static inline void chunk_set(Chunk *c, int x, int y, int z, uint8_t id) {
    if (x<0||x>=CHUNK_W||y<0||y>=CHUNK_H||z<0||z>=CHUNK_D) return;
    c->blocks[chunk_idx(x,y,z)] = id;
    c->dirty = 1;
}

void chunk_init_gl(Chunk *c);
void chunk_free_gl(Chunk *c);
