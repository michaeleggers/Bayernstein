#version 460 core

uniform sampler2D main3dSceneTexture;    // GL_TEXTURE0
uniform sampler2D screenspace2dTexture;  // GL_TEXTURE1

layout (location = 0) in vec2  in_uv;

out vec4 out_Color;

void main() {

    vec4 main3dColor = texture( main3dSceneTexture, in_uv );
    vec4 screen2dColor = texture( screenspace2dTexture, in_uv );

    vec3 compositeColor = mix( main3dColor.rgb, screen2dColor.rgb, screen2dColor.a );
    float alpha = screen2dColor.a + main3dColor.a * (1.0f - screen2dColor.a);

    //out_Color = vec4( main3dColor.rgb + screen2dColor.rgb * screen2dColor.a, 1.0f );
    //out_Color = screen2dColor;
    out_Color = vec4( compositeColor, 1.0f );

}

