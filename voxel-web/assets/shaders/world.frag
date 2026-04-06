#version 300 es
precision mediump float;

in vec2  v_uv;
in float v_light;
in float v_depth;

uniform sampler2D u_atlas;
uniform vec3      u_fog_color;
uniform float     u_fog_start;
uniform float     u_fog_end;

out vec4 frag_color;

void main() {
    vec4 tex = texture(u_atlas, v_uv);
    if (tex.a < 0.1) discard;

    vec3 color = tex.rgb * v_light;

    float fog = clamp((v_depth - u_fog_start) / (u_fog_end - u_fog_start), 0.0, 1.0);
    color = mix(color, u_fog_color, fog);

    frag_color = vec4(color, tex.a);
}
