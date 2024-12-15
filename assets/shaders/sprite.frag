#version 460 core

in vec2 uv;

out vec4 out_color;

uniform sampler2D spriteTexture;

void main() {

	vec4 color = texture(spriteTexture, uv);

    out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	out_color = color;
}

