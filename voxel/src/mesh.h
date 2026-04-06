#pragma once
#include "chunk.h"
#include "world.h"

/* Vertex layout (12 bytes):
   x,y,z   - int16 local chunk position
   tu,tv   - uint8 atlas tile col/row
   cu,cv   - uint8 corner within tile (0 or 1 for each axis)
   face    - uint8 face index 0-5 (for lighting)
   _pad    - uint8 padding */
typedef struct {
    int16_t x, y, z;
    uint8_t tu, tv;     /* atlas tile col/row */
    uint8_t cu, cv;     /* corner offset 0 or 1 */
    uint8_t face;
    uint8_t _pad;
} Vertex;

void mesh_build(Chunk *c, World *w);
