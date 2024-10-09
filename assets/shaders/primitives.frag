#version 410

in vec2 TexCoord;
in vec3 BaryCentricCoords;
in vec4 Normal;
in vec4 Color;
in vec4 ViewPosWorldSpace;
in vec4 Pos;
in mat4 ViewMat;

out vec4 out_Color;

layout (std140) uniform Settings {
    uint drawWireframe;
};

float edgeFactor(){
    vec3 d = fwidth(BaryCentricCoords);
    vec3 a3 = smoothstep(vec3(0.0), d, BaryCentricCoords);
    return min(min(a3.x, a3.y), a3.z);
}

const uint SHADER_WIREFRAME_ON_MESH = 0x00000001 << 0;
const uint SHADER_LINEMODE          = 0x00000001 << 1;
const uint SHADER_ANIMATED          = 0x00000001 << 2;

void main() {

    vec4 lightColor = vec4(1.0, .9, 0.3, 1.0);
    vec4 lightPos = ViewMat * ViewPosWorldSpace;
    //vec4 lightPos = ViewMat * vec4(0.0f, -200.0f, 10.0f, 1.0f);

    vec3 fragToLight = normalize(lightPos.xyz - Pos.xyz);
    // abs to light both sides of the polygon
    float lightContribution = abs(dot(fragToLight, Normal.xyz));

    vec4 wireframe = vec4(0.0);
    if ( (drawWireframe & SHADER_WIREFRAME_ON_MESH) == SHADER_WIREFRAME_ON_MESH) {
        wireframe = vec4(mix(vec3(1.0), vec3(0.0), edgeFactor()), 1.0);
        wireframe.a = 0.2;
        wireframe.rgb *= wireframe.a;
    }

    vec3 finalColor;
    if ( (drawWireframe & SHADER_LINEMODE) == SHADER_LINEMODE) {
        finalColor = Color.rgb;
    } else {        
        vec3 ambient = 0.3*Color.rgb;
        finalColor = clamp(ambient + Color.rgb*lightColor.rgb*lightContribution, 0.0f, 1.0f);
    }
    
    out_Color = vec4(Color.rgb + wireframe.rgb, 1.0);  
    out_Color = vec4(finalColor, 1.0);  
    //out_Color = vec4(0.5*Normal.xyz + 0.5f, 1.0f);

}
