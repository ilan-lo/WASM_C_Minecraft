#pragma once
#include <stdint.h>

#define BLOCK_AIR    0
#define BLOCK_GRASS  1
#define BLOCK_DIRT   2
#define BLOCK_STONE  3
#define BLOCK_WOOD   4
#define BLOCK_LEAVES 5
#define BLOCK_SAND   6
#define BLOCK_WATER  7
#define BLOCK_BEDROCK 8
#define BLOCK_COUNT  256

#define BLOCK_FLAG_SOLID       (1<<0)
#define BLOCK_FLAG_TRANSPARENT (1<<1)

typedef struct {
    const char *name;
    uint8_t     tex_top;    /* atlas tile index */
    uint8_t     tex_side;
    uint8_t     tex_bottom;
    uint8_t     flags;
} BlockDef;

extern BlockDef g_blocks[BLOCK_COUNT];

void blocks_init(void);

static inline int block_is_solid(uint8_t id) {
    return (g_blocks[id].flags & BLOCK_FLAG_SOLID) != 0;
}
static inline int block_is_transparent(uint8_t id) {
    return (g_blocks[id].flags & BLOCK_FLAG_TRANSPARENT) != 0;
}
