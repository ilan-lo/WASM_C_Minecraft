#include "world.h"
#include "worldgen.h"
#include "blocks.h"
#include <string.h>
#include <stdlib.h>

/* ---- hash map helpers ---- */

static uint32_t chunk_hash(int32_t cx, int32_t cz) {
    uint32_t h = (uint32_t)(cx * 1664525 + cz * 1013904223);
    h ^= h >> 16;
    return h & (MAP_SIZE - 1);
}

static Chunk *map_get(ChunkEntry *map, int32_t cx, int32_t cz) {
    uint32_t idx = chunk_hash(cx, cz);
    for (int i = 0; i < MAP_SIZE; i++) {
        uint32_t slot = (idx + i) & (MAP_SIZE - 1);
        if (!map[slot].chunk) return NULL;
        if (map[slot].cx == cx && map[slot].cz == cz)
            return map[slot].chunk;
    }
    return NULL;
}

static void map_put(ChunkEntry *map, int32_t cx, int32_t cz, Chunk *chunk) {
    uint32_t idx = chunk_hash(cx, cz);
    for (int i = 0; i < MAP_SIZE; i++) {
        uint32_t slot = (idx + i) & (MAP_SIZE - 1);
        if (!map[slot].chunk || (map[slot].cx == cx && map[slot].cz == cz)) {
            map[slot].cx    = cx;
            map[slot].cz    = cz;
            map[slot].chunk = chunk;
            return;
        }
    }
}

static void map_remove(ChunkEntry *map, int32_t cx, int32_t cz) {
    uint32_t idx = chunk_hash(cx, cz);
    for (int i = 0; i < MAP_SIZE; i++) {
        uint32_t slot = (idx + i) & (MAP_SIZE - 1);
        if (!map[slot].chunk) return;
        if (map[slot].cx == cx && map[slot].cz == cz) {
            map[slot].chunk = NULL;
            /* rehash subsequent entries in this probe chain */
            for (int j = 1; j < MAP_SIZE; j++) {
                uint32_t next = (slot + j) & (MAP_SIZE - 1);
                if (!map[next].chunk) break;
                ChunkEntry e = map[next];
                map[next].chunk = NULL;
                map_put(map, e.cx, e.cz, e.chunk);
            }
            return;
        }
    }
}

/* ---- pool allocation ---- */

static Chunk *pool_alloc(World *w) {
    if (w->pool_used >= MAX_CHUNKS) return NULL;
    Chunk *c = &w->pool[w->pool_used++];
    memset(c, 0, sizeof(Chunk));
    return c;
}

/* ---- public API ---- */

void world_init(World *w) {
    memset(w, 0, sizeof(World));
}

Chunk *world_get_chunk(World *w, int32_t cx, int32_t cz) {
    return map_get(w->map, cx, cz);
}

Chunk *world_get_or_create(World *w, int32_t cx, int32_t cz) {
    Chunk *c = map_get(w->map, cx, cz);
    if (c) return c;
    c = pool_alloc(w);
    if (!c) return NULL;
    c->cx = cx; c->cz = cz;
    c->loaded = 1;
    c->dirty  = 1;
    worldgen_generate(c);
    chunk_init_gl(c);
    map_put(w->map, cx, cz, c);
    return c;
}

uint8_t world_get_block(World *w, int32_t x, int32_t y, int32_t z) {
    if (y < 0 || y >= CHUNK_H) return 0;
    int32_t cx = (int32_t)((x < 0) ? (x - CHUNK_W + 1) / CHUNK_W : x / CHUNK_W);
    int32_t cz = (int32_t)((z < 0) ? (z - CHUNK_D + 1) / CHUNK_D : z / CHUNK_D);
    Chunk *c = map_get(w->map, cx, cz);
    if (!c) return 0;
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    return chunk_get(c, lx, y, lz);
}

void world_set_block(World *w, int32_t x, int32_t y, int32_t z, uint8_t id) {
    if (y < 0 || y >= CHUNK_H) return;
    int32_t cx = (int32_t)((x < 0) ? (x - CHUNK_W + 1) / CHUNK_W : x / CHUNK_W);
    int32_t cz = (int32_t)((z < 0) ? (z - CHUNK_D + 1) / CHUNK_D : z / CHUNK_D);
    Chunk *c = map_get(w->map, cx, cz);
    if (!c) return;
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    chunk_set(c, lx, y, lz, id);
    /* mark neighbors dirty if on edge */
    if (lx == 0)           { Chunk *n = map_get(w->map, cx-1, cz); if(n) n->dirty=1; }
    if (lx == CHUNK_W - 1) { Chunk *n = map_get(w->map, cx+1, cz); if(n) n->dirty=1; }
    if (lz == 0)           { Chunk *n = map_get(w->map, cx, cz-1); if(n) n->dirty=1; }
    if (lz == CHUNK_D - 1) { Chunk *n = map_get(w->map, cx, cz+1); if(n) n->dirty=1; }
}

void world_update(World *w, float px, float pz) {
    int32_t pcx = (int32_t)(px / CHUNK_W);
    int32_t pcz = (int32_t)(pz / CHUNK_D);

    /* load missing chunks within render distance */
    for (int dz = -RENDER_DISTANCE; dz <= RENDER_DISTANCE; dz++) {
        for (int dx = -RENDER_DISTANCE; dx <= RENDER_DISTANCE; dx++) {
            world_get_or_create(w, pcx + dx, pcz + dz);
        }
    }
}
