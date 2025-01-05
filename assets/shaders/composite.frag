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

    vec3 gameColor = main3dColor.rgb + firstPersonViewColor.rgb;

    vec3 blendedColor3d2d = screen2dColor.rgb + (1.0f - screen2dColor.a) * (firstPersonViewColor.rgb + main3dColor.rgb);
    vec3 blendedColor = consoleColor.rgb + (1.0f - consoleColor.a) * blendedColor3d2d.rgb;

    float blendedAlpha = screen2dColor.a  + (1.0f - screen2dColor.a) * (1.0f - consoleColor.a)
					    * firstPersonViewColor.a * main3dColor.a;

    //out_Color = vec4( main3dColor.rgb + (1.0f - main3dColor.a) * firstPersonViewColor.rgb, firstPersonViewColor.a);
    //out_Color = firstPersonViewColor;
    out_Color = mix(main3dColor, firstPersonViewColor, firstPersonViewColor.a);
    //out_Color = main3dColor + firstPersonViewColor; // + firstPersonViewColor * firstPersonViewColor.a;
}

