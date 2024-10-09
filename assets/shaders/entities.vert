#version 410

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 bc;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec4 color;
layout (location = 5) in uvec4 blendindices;
layout (location = 6) in vec4 blendweights;


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
    vec4 v = vec4(pos, 1.0);
    
    mat4 skinMat = (
        palette[blendindices.x]*blendweights.x
     + palette[blendindices.y]*blendweights.y 
     + palette[blendindices.z]*blendweights.y 
     + palette[blendindices.w]*blendweights.w);

    uint shaderBits0 = bitFields.x;
    if ( (shaderBits0 & SHADER_ANIMATED) == SHADER_ANIMATED ) {
        gl_Position = proj * view * model * skinMat * v;
    }
    else {
        gl_Position = proj * view * model * v;
    }
        
    TexCoord = uv;
    BaryCentricCoords = bc;
    Normal = normal;
}
