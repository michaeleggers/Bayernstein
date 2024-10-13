#version 460 core

out vec4 out_color;

uniform sampler2D glyphsTexture;

layout (std140) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Settings {
    uvec4 bitFields;
};

layout (location = 0) in vec2 in_uv;

void main() {

    vec4 color = texture( glyphsTexture, in_uv );
    out_color = vec4(color.a, 0.0f, 0.0f, 1.0f);
}

