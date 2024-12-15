#version 460 core

const vec2 quad_pos[6] = vec2[6](
    vec2(0.0f, 0.0f),
    vec2(0.0f, -1.0f),
    vec2(1.0f, -1.0f),

    vec2(1.0f, -1.0f),
    vec2(1.0f, 0.0f),
    vec2(0.0f, 0.0f)
);

const vec2 quad_uv[6] = vec2[6](
    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f),
    vec2(1.0f, 0.0f),

    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f),
    vec2(0.0f, 1.0f)
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
    vec2 size;
    vec2 scale;
    vec2 uv_topLeft;
    vec2 uv_bottomRight;
};

out vec2 uv;

void main() {
    vec2 v = quad_pos[ gl_VertexID ];
    v = scale * size * v;
    v = v + pos;

    vec4 clipSpaceV = proj * view * vec4(v, 0.0f, 1.0f);

    uv = quad_uv[ gl_VertexID ];
    //uv.x = uv_topLeft.x + uv.x * uv_bottomRight.x;
    //uv.y = uv_topLeft.x + uv.y * uv_bottomRight.y;

    uv = uv_topLeft + uv * uv_bottomRight;

    gl_Position = clipSpaceV;
}

