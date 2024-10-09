#version 410

in vec3 Color;
in vec2 UV;

out vec4 out_Color;

uniform sampler2D u_Texture;
uniform uint      u_FrameNo;
uniform ivec2     u_FrameWidthHeight;


void main()
{
	vec4 textureColor = texture(u_Texture, UV);
	//out_Color = vec4(Color, 1.0);	
	out_Color = textureColor;
}
