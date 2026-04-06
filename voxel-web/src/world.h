#pragma once
#include "chunk.h"

#define MAX_CHUNKS      512
#define RENDER_DISTANCE 4     /* chunks in each direction */
#define MAP_SIZE        1024  /* hash map capacity, must be power of 2 */

typedef struct {
    int32_t cx, cz;
    Chunk  *chunk;
} ChunkEntry;

typedef struct {
    Chunk       pool[MAX_CHUNKS];
    int         pool_used;
    ChunkEntry  map[MAP_SIZE];   /* open-addressing hash map */
} World;

void    world_init(World *w);
Chunk  *world_get_chunk(World *w, int32_t cx, int32_t cz);
Chunk  *world_get_or_create(World *w, int32_t cx, int32_t cz);
uint8_t world_get_block(World *w, int32_t x, int32_t y, int32_t z);
void    world_set_block(World *w, int32_t x, int32_t y, int32_t z, uint8_t id);
void    world_update(World *w, float px, float pz); /* load/unload chunks */
