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

layout (std140) uniform Settings {
    uint drawWireframe;
};

layout (std140) uniform Palette {
    mat4 palette[96];
};

uniform mat4 model;
uniform vec3 viewPos;

out vec4 Color;

void main() {
    vec4 v = vec4(in_pos, 1.0);    
    gl_Position = proj * view * model * v;
  
    Color = in_color;
}

