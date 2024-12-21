#version 460 core

in vec2 uv;

out vec4 out_color;

uniform sampler2D spriteTexture;

void main() {

	vec4 color = texture(spriteTexture, uv);

	out_color = color;
}

