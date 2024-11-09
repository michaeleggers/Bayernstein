#version 460 core

layout (location = 0) in vec3  in_pos;
layout (location = 1) in vec2  in_uv;
layout (location = 2) in vec2  in_uvLightmap;
layout (location = 3) in vec3  in_bc;
layout (location = 4) in vec3  in_normal;
layout (location = 5) in vec4  in_color;
layout (location = 6) in uvec4 in_blendindices;
layout (location = 7) in vec4  in_blendweights;

layout (location = 0) out vec2 out_uv;

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


void main() {
    gl_Position = vec4( quad[ gl_VertexID ], 1.0f );
    out_uv = uvs[ gl_VertexID ];
}


