#include "worldgen.h"
#include "blocks.h"
#include <math.h>

/* Minimal FBM noise without external dependencies */
static float hash2(int x, int z) {
    int n = x + z * 57;
    n = (n << 13) ^ n;
    return (1.f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.f);
}

static float smooth(float t) { return t*t*(3.f-2.f*t); }

static float noise2(float x, float z) {
    int ix = (int)floorf(x), iz = (int)floorf(z);
    float fx = x - ix, fz = z - iz;
    float a = hash2(ix,   iz  );
    float b = hash2(ix+1, iz  );
    float c = hash2(ix,   iz+1);
    float d = hash2(ix+1, iz+1);
    float sx = smooth(fx), sz = smooth(fz);
    return a + (b-a)*sx + (c-a)*sz + (a-b-c+d)*sx*sz;
}

static float fbm(float x, float z, int octaves) {
    float val = 0.f, amp = 1.f, freq = 1.f, max = 0.f;
    for (int i = 0; i < octaves; i++) {
        val += noise2(x*freq, z*freq) * amp;
        max += amp;
        amp  *= 0.5f;
        freq *= 2.f;
    }
    return val / max;
}

/* Simple tree: trunk + leaf cube */
static void place_tree(Chunk *c, int lx, int base_y, int lz) {
    int height = 4 + (int)(fabsf(hash2(lx*7, lz*13)) * 3.f);
    for (int y = base_y; y < base_y + height && y < CHUNK_H; y++)
        chunk_set(c, lx, y, lz, BLOCK_WOOD);
    /* leaves 3x3x3 on top */
    for (int dy = -1; dy <= 2; dy++)
    for (int dx = -2; dx <= 2; dx++)
    for (int dz = -2; dz <= 2; dz++) {
        int nx = lx+dx, ny = base_y+height+dy, nz = lz+dz;
        if (nx>=0 && nx<CHUNK_W && ny>=0 && ny<CHUNK_H && nz>=0 && nz<CHUNK_D)
            if (chunk_get(c, nx, ny, nz) == BLOCK_AIR)
                chunk_set(c, nx, ny, nz, BLOCK_LEAVES);
    }
}

void worldgen_generate(Chunk *c) {
    float ox = (float)(c->cx * CHUNK_W);
    float oz = (float)(c->cz * CHUNK_D);

    for (int x = 0; x < CHUNK_W; x++) {
        for (int z = 0; z < CHUNK_D; z++) {
            float wx = ox + x, wz = oz + z;

            /* heightmap */
            float continent = fbm(wx * 0.003f, wz * 0.003f, 4);
            float detail    = fbm(wx * 0.02f,  wz * 0.02f,  3);
            int height = (int)(50.f + continent * 30.f + detail * 8.f);
            if (height < 2)  height = 2;
            if (height >= CHUNK_H) height = CHUNK_H - 1;

            /* bedrock */
            chunk_set(c, x, 0, z, BLOCK_BEDROCK);

            /* stone */
            for (int y = 1; y < height - 3; y++)
                chunk_set(c, x, y, z, BLOCK_STONE);

            /* dirt layers */
            for (int y = height - 3; y < height; y++)
                chunk_set(c, x, y, z, BLOCK_DIRT);

            /* surface */
            if (height < 40)
                chunk_set(c, x, height, z, BLOCK_SAND);
            else
                chunk_set(c, x, height, z, BLOCK_GRASS);

            /* cave pass: 3D noise carves stone */
            for (int y = 1; y < height - 3; y++) {
                float cave = fbm(wx * 0.05f, (float)y * 0.05f + wz * 0.05f, 2);
                if (cave > 0.55f)
                    chunk_set(c, x, y, z, BLOCK_AIR);
            }

            /* trees */
            if (height >= 40 && chunk_get(c, x, height, z) == BLOCK_GRASS) {
                float r = fabsf(fbm(wx * 0.3f, wz * 0.3f, 1));
                if (r > 0.78f && x >= 2 && x <= CHUNK_W-3 && z >= 2 && z <= CHUNK_D-3)
                    place_tree(c, x, height + 1, z);
            }
        }
    }
}
