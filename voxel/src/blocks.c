#include "blocks.h"

BlockDef g_blocks[BLOCK_COUNT];

/* Atlas layout (16-tile wide atlas, each tile 16x16):
   0  = blank/error (magenta)
   1  = grass top
   2  = grass side
   3  = dirt
   4  = stone
   5  = wood top
   6  = wood side
   7  = leaves
   8  = sand
   9  = bedrock
   10 = water
*/

void blocks_init(void) {
    /* AIR */
    g_blocks[BLOCK_AIR] = (BlockDef){
        .name="air", .tex_top=0, .tex_side=0, .tex_bottom=0, .flags=BLOCK_FLAG_TRANSPARENT
    };
    /* GRASS */
    g_blocks[BLOCK_GRASS] = (BlockDef){
        .name="grass", .tex_top=1, .tex_side=2, .tex_bottom=3, .flags=BLOCK_FLAG_SOLID
    };
    /* DIRT */
    g_blocks[BLOCK_DIRT] = (BlockDef){
        .name="dirt", .tex_top=3, .tex_side=3, .tex_bottom=3, .flags=BLOCK_FLAG_SOLID
    };
    /* STONE */
    g_blocks[BLOCK_STONE] = (BlockDef){
        .name="stone", .tex_top=4, .tex_side=4, .tex_bottom=4, .flags=BLOCK_FLAG_SOLID
    };
    /* WOOD */
    g_blocks[BLOCK_WOOD] = (BlockDef){
        .name="wood", .tex_top=5, .tex_side=6, .tex_bottom=5, .flags=BLOCK_FLAG_SOLID
    };
    /* LEAVES */
    g_blocks[BLOCK_LEAVES] = (BlockDef){
        .name="leaves", .tex_top=7, .tex_side=7, .tex_bottom=7,
        .flags=BLOCK_FLAG_SOLID|BLOCK_FLAG_TRANSPARENT
    };
    /* SAND */
    g_blocks[BLOCK_SAND] = (BlockDef){
        .name="sand", .tex_top=8, .tex_side=8, .tex_bottom=8, .flags=BLOCK_FLAG_SOLID
    };
    /* WATER */
    g_blocks[BLOCK_WATER] = (BlockDef){
        .name="water", .tex_top=10, .tex_side=10, .tex_bottom=10,
        .flags=BLOCK_FLAG_TRANSPARENT
    };
    /* BEDROCK */
    g_blocks[BLOCK_BEDROCK] = (BlockDef){
        .name="bedrock", .tex_top=9, .tex_side=9, .tex_bottom=9, .flags=BLOCK_FLAG_SOLID
    };
}
