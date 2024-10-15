#version 460 core

layout (location = 0) in vec3  in_pos;
layout (location = 1) in vec2  in_uv;
layout (location = 2) in vec3  in_bc;
layout (location = 3) in vec3  in_normal;
layout (location = 4) in vec4  in_color;
layout (location = 5) in uvec4 in_blendindices;
layout (location = 6) in vec4  in_blendweights;

layout (location = 0) out vec2 out_uv;
layout (location = 1) out vec4 out_color;

vec3 quad[6] = vec3[6](
    // lower left tri
    vec3(1.0f, -1.0f, 0.0f),
    vec3(1.0f, 1.0f, 0.0f),
    vec3(-1.0f, -1.0f, 0.0f),
    
    // upper right tri
    vec3(-1.0f, -1.0f, 0.0f),
    vec3(1.0f, 1.0f, 0.0f),
    vec3(-1.0f, 1.0f, 0.0f)
);

vec2 uvs[6] = vec2[6](
    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f),
    vec2(0.0f, 0.0f),

    vec2(0.0f, 0.0f),
    vec2(1.0f, 1.0f),
    vec2(0.0f, 1.0f)
);

layout (std140) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Settings {
    uvec4 bitFields;
};

layout (std140) uniform screenspaceUBO {
    vec4 color;
    vec4 size; // x = size, yzw = free (unused for now)
};

void main() {
    gl_Position = proj * vec4( in_pos, 1.0f );
    out_uv = in_uv; 
    out_color = color;
}

