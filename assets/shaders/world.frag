
#version 460 core

in vec2 TexCoord;
in vec3 BaryCentricCoords;
in vec4 Normal;
in vec4 Color;
in vec4 ViewPosWorldSpace;
in vec4 Pos;
in mat4 ViewMat;
in vec2 uv;
in vec2 uvLightmap;

out vec4 out_Color;

layout(binding = 0) uniform sampler2D diffuseTexture;  // Slot 0
layout(binding = 1) uniform sampler2D lightmapTexture; // Slot 1

layout (std140) uniform Settings {
    uint shaderSettingBits;
};


const uint SHADER_WIREFRAME_ON_MESH = 0x00000001 << 0;
const uint SHADER_LINEMODE          = 0x00000001 << 1;
const uint SHADER_ANIMATED          = 0x00000001 << 2;
const uint SHADER_IS_TEXTURED       = 0x00000001 << 3;
const uint SHADER_USE_LIGHTMAP      = 0x00000001 << 4;
const uint SHADER_LIGHTMAP_ONLY     = 0x00000001 << 5;

// Scaling factor used during PNG generation
const float gamma = 2.2;
const float scaling_factor = 1.6953125; // Replace with the printed scaling factor from the Python code

float edgeFactor(){
    vec3 d = fwidth(BaryCentricCoords);
    vec3 a3 = smoothstep(vec3(0.0), d, BaryCentricCoords);
    return min(min(a3.x, a3.y), a3.z);
}

void main() {

// TODO: Wireframe rendering (at the moment turned off, the stuff
//       below is legacy code.
//

/*
    vec4 lightColor = vec4(1.0, .9, 0.3, 1.0);
    vec4 lightPos = ViewMat * ViewPosWorldSpace;
    //vec4 lightPos = ViewMat * vec4(0.0f, -200.0f, 10.0f, 1.0f);

    vec3 fragToLight = normalize(lightPos.xyz - Pos.xyz);
    // abs to light both sides of the polygon
    float lightContribution = abs(dot(fragToLight, Normal.xyz));

    vec4 wireframe = vec4(0.0);
    if ( (shaderSettingBits & SHADER_WIREFRAME_ON_MESH) == SHADER_WIREFRAME_ON_MESH) {
        wireframe = vec4(mix(vec3(1.0), vec3(0.0), edgeFactor()), 1.0);
        wireframe.a = 0.2;
        wireframe.rgb *= wireframe.a;
    }

    vec3 finalColor;
    if ( (shaderSettingBits & SHADER_LINEMODE) == SHADER_LINEMODE) {
        finalColor = Color.rgb;
    } else {        
        vec3 ambient = 0.3*Color.rgb;
        finalColor = clamp(ambient + Color.rgb*lightColor.rgb*lightContribution, 0.0f, 1.0f);
    }
*/

    vec4 diffuseColor = texture( diffuseTexture, uv );

    vec4 lightmapColor = vec4(1.0f);
    if ( ((shaderSettingBits & SHADER_USE_LIGHTMAP) == SHADER_USE_LIGHTMAP) ||
         ((shaderSettingBits & SHADER_LIGHTMAP_ONLY) == SHADER_LIGHTMAP_ONLY) ) 
    {
        lightmapColor = texture( lightmapTexture, uvLightmap );

        // Reverse gamma correction
        vec3 linearLightmap = pow(lightmapColor.rgb, vec3(gamma));

        // Reverse logarithmic compression
        vec3 hdrLightmap = exp2(log2(1.0 + linearLightmap) * scaling_factor) - 1.0;

                // Rescale back to the HDR range using the scaling factor
        lightmapColor.rgb = hdrLightmap * scaling_factor;


    }
    
    vec4 finalOutputColor = vec4( diffuseColor.rgb * lightmapColor.rgb, 1.0f );
    //finalOutputColor.rgb = pow(finalOutputColor.rgb, vec3(1.0f/1.2f));
    if ( (shaderSettingBits & SHADER_LIGHTMAP_ONLY) == SHADER_LIGHTMAP_ONLY ) {
        finalOutputColor = vec4(lightmapColor.rgb, 1.0f);
    }
    
    out_Color = finalOutputColor;
}
