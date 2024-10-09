#version 410

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 bc;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec4 color;
layout (location = 5) in uvec4 blendindices;
layout (location = 6) in vec4 blendweights;


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
    vec4 v = vec4(pos, 1.0);    
    gl_Position = proj * view * v;
    
    TexCoord = uv;
    BaryCentricCoords = bc;
    Normal = view * vec4(normal, 0.0f);
    Normal = view * vec4(normal, 0.0f);
    Color = color;
    ViewPosWorldSpace = vec4(viewPos, 1.0f);
    Pos = view * vec4(pos, 1.0);
    ViewMat = view;
}

