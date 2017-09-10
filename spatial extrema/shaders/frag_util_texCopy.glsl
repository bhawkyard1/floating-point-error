#version 430 core

layout( location = 0 ) out vec4 fragColour;

in vec4 normal;
in vec4 position;
in vec2 UV;

uniform sampler2D u_tex;

void main()
{
    fragColour = texture( u_tex, UV );
    //fragColour = vec4(1.0,0.0,1.0,1.0);
}
