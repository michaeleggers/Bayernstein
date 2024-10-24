#version 460 core

uniform sampler2D main3dSceneTexture;    // GL_TEXTURE0
uniform sampler2D screenspace2dTexture;  // GL_TEXTURE1

layout (location = 0) in vec2  in_uv;

out vec4 out_Color;

void main() {

    vec4 main3dColor = texture( main3dSceneTexture, in_uv );
    vec4 screen2dColor = texture( screenspace2dTexture, in_uv );

    vec3 blendedColor = screen2dColor.rgb + (1.0f - screen2dColor.a) * main3dColor.rgb;
    float blendedAlpha = screen2dColor.a  + (1.0f - screen2dColor.a) * main3dColor.a;

    out_Color = vec4( blendedColor, blendedAlpha );
}

