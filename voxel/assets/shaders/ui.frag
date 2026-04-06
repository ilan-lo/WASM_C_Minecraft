#version 330 core

in vec2 v_uv;
in vec4 v_color;

uniform sampler2D u_tex;
uniform int       u_use_tex;

out vec4 frag_color;

void main() {
    if (u_use_tex == 1) {
        vec4 t = texture(u_tex, v_uv);
        frag_color = t * v_color;
    } else {
        frag_color = v_color;
    }
}
