#version 460 core

uniform sampler2D main3dSceneTexture;    // GL_TEXTURE0
uniform sampler2D screenspace2dTexture;  // GL_TEXTURE1

layout (location = 0) in vec2  in_uv;

out vec4 out_Color;

void main() {

    vec4 main3dColor = texture( main3dSceneTexture, in_uv );
    vec4 screen2dColor = texture( screenspace2dTexture, in_uv );

    out_Color = vec4( main3dColor.rgb + screen2dColor.rgb, 1.0 );
}

