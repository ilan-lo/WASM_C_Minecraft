#include "minimap.h"
#include "blocks.h"
#include <math.h>
#include <string.h>
#include <stddef.h>

/* RGBA colors per block type — matches the placeholder atlas palette */
static const uint32_t block_color[BLOCK_COUNT] = {
    [BLOCK_AIR]     = 0x00000000,
    [BLOCK_GRASS]   = 0xFF3DA035,
    [BLOCK_DIRT]    = 0xFF4A2D1A,
    [BLOCK_STONE]   = 0xFF787878,
    [BLOCK_WOOD]    = 0xFF5C3A1E,
    [BLOCK_LEAVES]  = 0xFF2D7A2D,
    [BLOCK_SAND]    = 0xFFD2C278,
    [BLOCK_WATER]   = 0xFFB04020,   /* shown as ABGR in memory on little-endian */
    [BLOCK_BEDROCK] = 0xFF404040,
};

/* Slight height shading: brighter = higher terrain */
static uint32_t shade(uint32_t rgba, int y) {
    float f = 0.6f + 0.4f * ((float)y / (float)CHUNK_H);
    uint8_t r = (uint8_t)(((rgba >>  0) & 0xFF) * f);
    uint8_t g = (uint8_t)(((rgba >>  8) & 0xFF) * f);
    uint8_t b = (uint8_t)(((rgba >> 16) & 0xFF) * f);
    uint8_t a =           (rgba >> 24) & 0xFF;
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
}

typedef struct { float x, y, u, v; } MMVert;

void minimap_init(Minimap *m) {
    memset(m, 0, sizeof(Minimap));
    m->last_px = m->last_pz = -9999.f;

    /* texture */
    glGenTextures(1, &m->tex);
    glBindTexture(GL_TEXTURE_2D, m->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MM_PIXELS, MM_PIXELS, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* VAO for a textured quad */
    glGenVertexArrays(1, &m->vao);
    glGenBuffers(1, &m->vbo);
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(MMVert), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MMVert), (void*)offsetof(MMVert,x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MMVert), (void*)offsetof(MMVert,u));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void minimap_update(Minimap *m, World *w, Player *p) {
    float px = p->body.pos.x, pz = p->body.pos.z;

    /* only re-scan when player has moved at least 2 blocks */
    if (fabsf(px - m->last_px) < 2.f && fabsf(pz - m->last_pz) < 2.f) return;
    m->last_px = px; m->last_pz = pz;

    int cx = (int)floorf(px);
    int cz = (int)floorf(pz);
    int half = MM_BLOCKS / 2;

    for (int pzi = 0; pzi < MM_PIXELS; pzi++) {
        for (int pxi = 0; pxi < MM_PIXELS; pxi++) {
            int wx = cx + pxi - half;
            int wz = cz + pzi - half;

            /* find topmost non-air block */
            uint8_t id = BLOCK_AIR;
            int surface_y = 0;
            for (int y = CHUNK_H - 1; y >= 0; y--) {
                id = world_get_block(w, wx, y, wz);
                if (id != BLOCK_AIR) { surface_y = y; break; }
            }

            uint32_t col = block_color[id];
            if (id != BLOCK_AIR) col = shade(col, surface_y);

            m->pixels[pzi * MM_PIXELS + pxi] = col;
        }
    }

    /* player dot — 3x3 red cross at center */
    int mid = MM_PIXELS / 2;
    uint32_t red = 0xFF0000FF;   /* RGBA: R=255, A=255 in little-endian ABGR → actually store as RGBA bytes */
    for (int d = -1; d <= 1; d++) {
        if (mid+d >= 0 && mid+d < MM_PIXELS) {
            m->pixels[(mid)    * MM_PIXELS + (mid+d)] = red;
            m->pixels[(mid+d)  * MM_PIXELS + (mid)]   = red;
        }
    }

    glBindTexture(GL_TEXTURE_2D, m->tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, MM_PIXELS, MM_PIXELS,
                    GL_RGBA, GL_UNSIGNED_BYTE, m->pixels);
}

void minimap_render(Minimap *m, int win_w, int win_h) {
    /* top-right corner with a small margin */
    float margin = 10.f;
    float x1 = (float)win_w - margin - MM_SCREEN;
    float y1 = (float)win_h - margin - MM_SCREEN;
    float x2 = x1 + MM_SCREEN;
    float y2 = y1 + MM_SCREEN;

    MMVert verts[6] = {
        {x1,y1, 0,0}, {x2,y1, 1,0}, {x2,y2, 1,1},
        {x1,y1, 0,0}, {x2,y2, 1,1}, {x1,y2, 0,1},
    };

    glBindTexture(GL_TEXTURE_2D, m->tex);
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
