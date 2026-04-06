#include "chunk.h"
#include <string.h>

void chunk_init_gl(Chunk *c) {
    glGenVertexArrays(1, &c->vao);
    glGenBuffers(1, &c->vbo);
    c->mesh_ready = 0;
    c->vertex_count = 0;
}

void chunk_free_gl(Chunk *c) {
    if (c->vao) { glDeleteVertexArrays(1, &c->vao); c->vao = 0; }
    if (c->vbo) { glDeleteBuffers(1, &c->vbo);      c->vbo = 0; }
    c->mesh_ready = 0;
}
