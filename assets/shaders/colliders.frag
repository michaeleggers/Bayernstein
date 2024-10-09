#version 410

in vec4 Color;

uniform vec4 uDebugColor;

out vec4 out_Color;



void main() {

    out_Color = uDebugColor;

}
