
#version 460 core


in vec2 TexCoord;
in vec3 BaryCentricCoords;
in vec3 Normal;

out vec4 out_Color;

const uint SHADER_WIREFRAME_ON_MESH = 0x00000001 << 0;
const uint SHADER_LINEMODE          = 0x00000001 << 1;
const uint SHADER_ANIMATED          = 0x00000001 << 2;
const uint SHADER_IS_TEXTURED		= 0x00000001 << 3;

layout (std140) uniform Settings {
    uvec4 bitFields;
};

uniform sampler2D colorTex;

float edgeFactor(){
    vec3 d = fwidth(BaryCentricCoords);
    vec3 a3 = smoothstep(vec3(0.0), d, BaryCentricCoords);
    return min(min(a3.x, a3.y), a3.z);
}

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main() {
    
    uint shaderBits0 = bitFields.x;
    vec4 wireframe = vec4(0.0);
    if ( (shaderBits0 & SHADER_WIREFRAME_ON_MESH) == SHADER_WIREFRAME_ON_MESH ) {
        wireframe = vec4(mix(vec3(1.0), vec3(0.0), edgeFactor()), 1.0);
        wireframe.a = 0.2;
        wireframe.rgb *= wireframe.a;
    }

    vec3 normalColor = 0.5*Normal + 0.5;
    vec4 finalColor = vec4(normalColor, 1.0f);
    if ( (shaderBits0 & SHADER_IS_TEXTURED) == SHADER_IS_TEXTURED ) {
        finalColor = texture(colorTex, TexCoord);
    }
    
    out_Color = vec4(finalColor.rgb + wireframe.rgb, 1.0f);
}

