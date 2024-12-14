#version 460 core

const vec2 quad_pos[6] = vec2[6](
    vec2(0.0f, 0.0f),
    vec2(0.0f, -1.0f),
    vec2(1.0f, -1.0f),

    vec2(1.0f, -1.0f),
    vec2(1.0f, 0.0f),
    vec2(0.0f, 0.0f)
);

layout (std140) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Settings {
	uint settings;
};

layout (std140) uniform SpriteData {
    vec2 pos;
	vec2 scale;
	vec2 uv_topLeft;
    vec2 uv_bottomRight;
};

out vec2 uv;

void main() {
    vec2 v = quad_pos[ gl_VertexID ];
    v = scale * v;
    v = v + pos;

	uv = vec2(0.0f, 0.0f);

    gl_Position = vec4( v, 0.0f, 1.0f );
}

