#version 330 core

in vec3 in_pos;
in vec2 in_uv;
in mat4 in_model;

layout (std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};

out VS_OUT {
	vec2 uv;
	float id;
} vs_out;

void main() {
	gl_Position = proj * view * in_model * vec4(in_pos, 1.0);
	vs_out.uv = in_uv;
	vs_out.id = gl_InstanceID;
}

