#version 300 es

layout(location=0) in vec3  a_pos;
layout(location=1) in vec2  a_tile;
layout(location=2) in vec2  a_corner;
layout(location=3) in float a_face;

uniform mat4 u_mvp;
uniform mat4 u_mv;         /* model-view (no projection) for depth */
uniform vec3 u_chunk_offset;
uniform int  u_atlas_size;

out vec2  v_uv;
out float v_light;
out float v_depth;         /* view-space depth for fog */

void main() {
    vec3 world_pos = a_pos + u_chunk_offset;
    gl_Position = u_mvp * vec4(world_pos, 1.0);
    v_depth = -(u_mv * vec4(world_pos, 1.0)).z;   /* positive distance from camera */

    float tile_size = 1.0 / float(u_atlas_size);
    v_uv = (a_tile + a_corner) * tile_size;

    int face = int(a_face + 0.5);
    if      (face == 0) v_light = 1.00;
    else if (face == 1) v_light = 0.55;
    else if (face == 2 || face == 3) v_light = 0.75;
    else                             v_light = 0.85;
}
