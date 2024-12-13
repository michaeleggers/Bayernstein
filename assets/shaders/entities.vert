#version 460 core

layout (location = 0) in vec3  in_pos;
layout (location = 1) in vec2  in_uv;
layout (location = 2) in vec2  in_uvLightmap;
layout (location = 3) in vec3  in_bc;
layout (location = 4) in vec3  in_normal;
layout (location = 5) in vec4  in_color;
layout (location = 6) in uvec4 in_blendindices;
layout (location = 7) in vec4  in_blendweights;


// layout (std140, binding = 0) uniform ViewProjMatrices {
//     mat4 view;
//     mat4 proj;
// };

// layout (std140, binding = 4) uniform TransformMatrix {
//     mat4 transform;
// };

// layout (std140, binding = 5) uniform PoseMatrices {
//     mat4 palette[96];
// };

const uint SHADER_WIREFRAME_ON_MESH = 0x00000001 << 0;
const uint SHADER_LINEMODE          = 0x00000001 << 1;
const uint SHADER_ANIMATED          = 0x00000001 << 2;
const uint SHADER_IS_TEXTURED		= 0x00000001 << 3;

layout (std140) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Settings {
    uvec4 bitFields;
};

layout (std140) uniform Palette {
    mat4 palette[96];
};


uniform mat4 model;

out vec2 TexCoord;
out vec3 BaryCentricCoords;
out vec3 Normal;

void main() {
    vec4 v = vec4(in_pos, 1.0);
    
    mat4 skinMat = (
       palette[in_blendindices.x]*in_blendweights.x
     + palette[in_blendindices.y]*in_blendweights.y 
     + palette[in_blendindices.z]*in_blendweights.y 
     + palette[in_blendindices.w]*in_blendweights.w);

    uint shaderBits0 = bitFields.x;
    if ( (shaderBits0 & SHADER_ANIMATED) == SHADER_ANIMATED ) {
        gl_Position = proj * view * model * skinMat * v;
    }
    else {
        gl_Position = proj * view * model * v;
    }
        
    TexCoord = in_uv;
    BaryCentricCoords = in_bc;
    Normal = in_normal;
}

