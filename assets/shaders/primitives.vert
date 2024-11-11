#version 460 core

layout (location = 0) in vec3  in_pos;
layout (location = 1) in vec2  in_uv;
layout (location = 2) in vec2  in_uvLightmap;
layout (location = 3) in vec3  in_bc;
layout (location = 4) in vec3  in_normal;
layout (location = 5) in vec4  in_color;
layout (location = 6) in uvec4 in_blendindices;
layout (location = 7) in vec4  in_blendweights;


layout (std140) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Palette {
    mat4 palette[96];
};

uniform mat4 model;
uniform vec3 viewPos;

out vec2 TexCoord;
out vec3 BaryCentricCoords;
out vec4 Normal;
out vec4 Color;
out vec4 ViewPosWorldSpace;
out vec4 Pos;
out mat4 ViewMat;

void main() {
    vec4 v = vec4(in_pos, 1.0);    
    gl_Position = proj * view * v;
    
    TexCoord = in_uv;
    BaryCentricCoords = in_bc;
    Normal = view * vec4(in_normal, 0.0f);
    Normal = view * vec4(in_normal, 0.0f);
    Color = in_color;
    ViewPosWorldSpace = vec4(viewPos, 1.0f);
    Pos = view * vec4(in_pos, 1.0);
    ViewMat = view;
}

