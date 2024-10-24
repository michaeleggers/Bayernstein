#version 460 core

out vec4 out_color;

layout (std140) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Settings {
    uvec4 bitFields;
};

layout (location = 0) in vec4 in_color;

void main() {

    out_color = in_color;

}

