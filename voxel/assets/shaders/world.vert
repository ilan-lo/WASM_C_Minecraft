#version 330 core

layout(location=0) in vec3  a_pos;       /* local chunk position (int16, not normalized) */
layout(location=1) in vec2  a_tile;      /* atlas tile col/row (uint8, not normalized)   */
layout(location=2) in vec2  a_corner;    /* corner offset 0 or 1 per axis (uint8)        */
layout(location=3) in float a_face;      /* face index 0-5 (uint8, not normalized)       */

uniform mat4 u_mvp;
uniform vec3 u_chunk_offset;
uniform int  u_atlas_size;   /* tiles per row in atlas, e.g. 16 */

out vec2  v_uv;
out float v_light;

void main() {
    vec3 world_pos = a_pos + u_chunk_offset;
    gl_Position = u_mvp * vec4(world_pos, 1.0);

    /* UV: (tile_col + corner_u) / atlas_size */
    float tile_size = 1.0 / float(u_atlas_size);
    v_uv = (a_tile + a_corner) * tile_size;

    /* Face lighting */
    int face = int(a_face + 0.5);
    if      (face == 0) v_light = 1.00;   /* top */
    else if (face == 1) v_light = 0.55;   /* bottom */
    else if (face == 2 || face == 3) v_light = 0.75;  /* X sides */
    else                             v_light = 0.85;   /* Z sides */
}
