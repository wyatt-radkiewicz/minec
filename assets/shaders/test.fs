#version 330 core

out vec4 FragColor;

uniform sampler2DArray diffuse;

in VS_OUT {
	vec2 uv;
	float id;
} fs_in;

void main() {
	FragColor = texture(diffuse, vec3(fs_in.uv, fs_in.id));
}
