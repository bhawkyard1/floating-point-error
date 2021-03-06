#version 430 core

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

out vec4 normal;
out vec4 position;
out vec2 UV;

uniform mat4 M;

void main()
{
    gl_Position = M * inPosition;

    normal = M * vec4(inNormal.xyz, 0.0);
    normal.w = 1.0;
    position = M * inPosition;
    UV = inUV;
}
