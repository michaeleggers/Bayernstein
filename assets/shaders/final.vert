#version 410

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in vec2 in_UV;

out vec3 Color;
out vec2 UV;

uniform mat4 u_ModelMat;
uniform mat4 u_ViewProjMat;

void main()
{
	// vec4 pos = u_ViewProjMat * u_ModelMat * vec4(in_Pos, 1.0f);
	gl_Position = vec4(in_Pos, 1.0);
	
	Color = in_Color;
	UV = in_UV;
}

