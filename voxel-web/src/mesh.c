#include "mesh.h"
#include "blocks.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define MAX_VERTS (CHUNK_W * CHUNK_H * CHUNK_D * 6 * 6)

static const int normals[6][3] = {
    { 0, 1, 0}, { 0,-1, 0},
    { 1, 0, 0}, {-1, 0, 0},
    { 0, 0, 1}, { 0, 0,-1}
};

static uint8_t get_block(World *w, Chunk *c, int x, int y, int z) {
    if (y < 0 || y >= CHUNK_H) return 0;
    if (x >= 0 && x < CHUNK_W && z >= 0 && z < CHUNK_D)
        return chunk_get(c, x, y, z);
    int32_t wx = c->cx * CHUNK_W + x;
    int32_t wz = c->cz * CHUNK_D + z;
    return world_get_block(w, wx, y, wz);
}

/* v0..v3: CCW winding viewed from outside the face.
   cu/cv: corner UV (0 or 1 per axis) so the tile maps correctly. */
#define V(px,py,pz,u,v) \
    (Vertex){(int16_t)(px),(int16_t)(py),(int16_t)(pz), tu,tv, (u),(v), face, 0}

static void push_face(Vertex *verts, int *n,
    Vertex v0, Vertex v1, Vertex v2, Vertex v3)
{
    verts[(*n)++] = v0; verts[(*n)++] = v1; verts[(*n)++] = v2;
    verts[(*n)++] = v0; verts[(*n)++] = v2; verts[(*n)++] = v3;
}

void mesh_build(Chunk *c, World *w) {
    Vertex *verts = malloc(sizeof(Vertex) * MAX_VERTS);
    if (!verts) return;
    int count = 0;

    for (int y = 0; y < CHUNK_H; y++)
    for (int z = 0; z < CHUNK_D; z++)
    for (int x = 0; x < CHUNK_W; x++) {
        uint8_t id = chunk_get(c, x, y, z);
        if (!block_is_solid(id)) continue;

        BlockDef *def = &g_blocks[id];

        for (int f = 0; f < 6; f++) {
            uint8_t nid = get_block(w, c,
                x + normals[f][0],
                y + normals[f][1],
                z + normals[f][2]);
            if (block_is_solid(nid) && !block_is_transparent(nid)) continue;

            uint8_t tex  = (f == 0) ? def->tex_top :
                           (f == 1) ? def->tex_bottom : def->tex_side;
            uint8_t tu   = tex % 16;
            uint8_t tv   = tex / 16;
            uint8_t face = (uint8_t)f;

            /* CCW winding from outside. cv=0 = top of tile, cv=1 = bottom.
               Verified via cross-product: each triple gives the correct outward normal. */
            switch (f) {
            case 0: /* +Y top: normal cross check (0,0,1)x(1,0,0)=(0,1,0) ✓ */
                push_face(verts, &count,
                    V(x,  y+1,z,   0,0),
                    V(x,  y+1,z+1, 0,1),
                    V(x+1,y+1,z+1, 1,1),
                    V(x+1,y+1,z,   1,0));
                break;
            case 1: /* -Y bottom: normal (0,-1,0) */
                push_face(verts, &count,
                    V(x,  y,z,   0,0),
                    V(x+1,y,z,   1,0),
                    V(x+1,y,z+1, 1,1),
                    V(x,  y,z+1, 0,1));
                break;
            case 2: /* +X right: normal (1,0,0) */
                push_face(verts, &count,
                    V(x+1,y,  z,   0,1),
                    V(x+1,y+1,z,   0,0),
                    V(x+1,y+1,z+1, 1,0),
                    V(x+1,y,  z+1, 1,1));
                break;
            case 3: /* -X left: normal (-1,0,0) */
                push_face(verts, &count,
                    V(x,y,  z+1, 0,1),
                    V(x,y+1,z+1, 0,0),
                    V(x,y+1,z,   1,0),
                    V(x,y,  z,   1,1));
                break;
            case 4: /* +Z front: normal (0,0,1) */
                push_face(verts, &count,
                    V(x,  y,  z+1, 0,1),
                    V(x+1,y,  z+1, 1,1),
                    V(x+1,y+1,z+1, 1,0),
                    V(x,  y+1,z+1, 0,0));
                break;
            case 5: /* -Z back: normal (0,0,-1) */
                push_face(verts, &count,
                    V(x+1,y,  z, 0,1),
                    V(x,  y,  z, 1,1),
                    V(x,  y+1,z, 1,0),
                    V(x+1,y+1,z, 0,0));
                break;
            }
        }
    }

    glBindVertexArray(c->vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * count, verts, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_SHORT,         GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,tu));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,cu));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,face));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    c->vertex_count = count;
    c->mesh_ready   = 1;
    c->dirty        = 0;
    free(verts);
}
