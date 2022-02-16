#version 330 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 norm;

out VS_OUT {
	vec4 frag_pos;
	vec2 frag_uv;
	vec3 frag_norm;
} vs_out;

uniform mat4 Mm;
uniform mat4 Mv;
uniform mat4 Mp;

void main() {
    gl_Position = Mp * Mv * Mm * pos;
    vs_out.frag_pos = Mm* pos;
    vs_out.frag_uv = uv;
    vs_out.frag_norm = mat3(transpose(inverse(Mm))) * norm;
}