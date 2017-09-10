#version 430 core

in vec4 position;
in vec4 normal;
in vec2 UV;

layout( location = 0 ) out vec4 outColour;
layout( location = 1 ) out vec4 outNormal;
layout( location = 2 ) out vec4 outPosition;
layout( location = 3 ) out vec4 outDepth;

void main()
{
    outColour = vec4(1.0);//texture(diffuse, vec2(UV.x, -UV.y));
                //outColour.a = 1.0;
    outNormal = normal;
    outPosition = position;
    outDepth = vec4(gl_FragCoord.z / gl_FragCoord.w);
    outDepth.a = 1.0;
}
