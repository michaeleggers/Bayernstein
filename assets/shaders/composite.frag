#version 460 core

uniform sampler2D main3dSceneTexture;
uniform sampler2D screenspace2dTexture;

layout (location = 0) in vec2  in_uv;

out vec4 out_Color;

void main() {

    vec4 main3dColor = texture( main3dSceneTexture, in_uv );
    vec4 screen2dColor = texture( screenspace2dTexture, in_uv );

    out_Color = vec4( main3dColor + screen2dColor );
}

