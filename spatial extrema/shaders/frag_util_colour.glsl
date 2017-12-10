#version 410 core

//Writes out a single colour.

layout( location = 0 ) out vec4 fragColour;

in vec4 normal;
in vec4 position;
in vec2 UV;

uniform vec4 u_colour;

void main()
{
    fragColour = vec4(1.0,0.0,0.0,1.0);
}
