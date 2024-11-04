#version 460 core

layout (location = 0) in vec3  in_pos;
layout (location = 1) in vec2  in_uv;
layout (location = 2) in vec2  in_uvLightmap;
layout (location = 3) in vec3  in_bc;
layout (location = 4) in vec3  in_normal;
layout (location = 5) in vec4  in_color;
layout (location = 6) in uvec4 in_blendindices;
layout (location = 7) in vec4  in_blendweights;


layout (location = 0) out vec4 out_color;

layout (std140) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Settings {
    uvec4 bitFields;
};

layout (std140) uniform shapesUB {
    vec4 color;
    vec4 scale; // x = scale (unused for now), yzw = free (unused for now)
};

void main() {
    gl_Position = proj * vec4( in_pos, 1.0f );
    out_color = color;
}

