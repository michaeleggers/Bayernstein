#version 460 core

uniform sampler2D main3dSceneTexture;    // GL_TEXTURE0
uniform sampler2D firstPersonViewTexture; // GL_TEXTURE1
uniform sampler2D screenspace2dTexture;  // GL_TEXTURE2
uniform sampler2D consoleTexture;		 // GL_TEXTURE3

layout (location = 0) in vec2  in_uv;

out vec4 out_Color;

void main() {

    vec4 main3dColor = texture( main3dSceneTexture, in_uv );
    vec4 firstPersonViewColor = texture( firstPersonViewTexture, in_uv);
    vec4 screen2dColor = texture( screenspace2dTexture, in_uv );
    vec4 consoleColor = texture ( consoleTexture, in_uv );

    out_Color = mix(main3dColor, firstPersonViewColor, firstPersonViewColor.a);
    out_Color = mix(out_Color, screen2dColor, screen2dColor.a);
    out_Color = mix(out_Color, consoleColor, consoleColor.a);
}

