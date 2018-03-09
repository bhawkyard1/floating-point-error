#version 430 core

#define shadowbuffer 0

#define NUM_CASCADES 3
#define MAX_LIGHTS 512
#define MAX_SHADOW_SOURCES 16

#define FOG_DIVISOR 256.0

#import module_math
#import module_noise

in vec2 UV;

uniform float u_cascades[NUM_CASCADES + 1];

uniform sampler2D u_diffuse;
uniform sampler2D u_normal;
uniform sampler2D u_position;
uniform sampler3D u_shadowDepths;
uniform sampler2D u_linearDepth;

layout (location = 0) out vec4 fragColour;

struct light
{
    vec4 pos;
    vec4 dir;
    vec4 col;
    int type;
};
layout( std140 ) uniform lightBuffer
{
    light buf [MAX_LIGHTS];
} u_lbuf;

uniform mat4 u_shadowMatrices[ MAX_SHADOW_SOURCES ][ NUM_CASCADES ];

uniform int u_lbufLen;

uniform vec4 u_camPos;

//UVs per-pixel
uniform vec2 u_pixstep;

float shadowSample(float depth, vec2 smpl, ivec2 offset, int index)
{
    vec2 uv = smpl + offset * u_pixstep;
    if(depth > texture( u_shadowDepths, vec3( uv.x, uv.y, index ) ).r)
        return 1.0;
    return 0.0;
}

vec3 basicLight(vec4 fragPos, vec3 fragNormal, int lightIndex)
{
    vec4 lp = u_lbuf.buf[ lightIndex ].pos;
    float d = distanceSquared(fragPos.xyz, lp.xyz);

    //Light cutoff threshold
    if(u_lbuf.buf[ lightIndex ].col.a / d < 0.001)
        return vec3(0.0);

    vec4 lv = lp - fragPos;
    lv = normalize(lv);
    float mul = dot( lv, vec4(fragNormal.xyz, 0.0) );
    mul = clamp(mul, 0.0, 1.0);

    vec3 base = u_lbuf.buf[ lightIndex ].col.xyz;
    d = distance(lp, fragPos);
    d *= d;
    return vec3(mul);//(mul * u_lbuf.buf[ lightIndex ].col.a * base) / (1.0 + d);
}

void main()
{
    fragColour = vec4(texture(u_diffuse, UV));
    vec4 fragPos = texture(u_position, UV);
    vec3 fragNorm = texture(u_normal, UV).xyz;
    float fragDepth = texture(u_linearDepth, UV).r;

    fragColour.xyz = fragNorm.xyz;

#if shadowbuffer == 0
    if(fragColour.a == 0.0)
        discard;
#endif

    //Lights
    for(int i = 0; i < u_lbufLen; ++i)
    {
        fragColour.xyz = basicLight(fragPos, fragNorm, i);
    }
    fragColour.xyz = vec3( u_lbuf.buf[0].col.rgb );
    /*
        int cascadeIndex = -1;

        if(fragDepth > u_cascades[0] && fragDepth < u_cascades[1])
            cascadeIndex = 0;
        else if(fragDepth < u_cascades[2])
            cascadeIndex = 1;
        else if(fragDepth < u_cascades[3])
            cascadeIndex = 2;

        if(cascadeIndex != -1)
        {
            vec4 su_position = u_shadowMatrices[cascadeIndex] * vec4(fragPos.xyz, 1.0);

            float depth = su_position.z - bias;

            float shadow = shadowSample(depth, su_position.xy, ivec2(0, 0), cascadeIndex);
            shadow += shadowSample(depth, su_position.xy, ivec2(0, 1), cascadeIndex);
            shadow += shadowSample(depth, su_position.xy, ivec2(1, 0), cascadeIndex);
            shadow += shadowSample(depth, su_position.xy, ivec2(0, -1), cascadeIndex);
            shadow += shadowSample(depth, su_position.xy, ivec2(-1, 0), cascadeIndex);

            shadow /= 5.0;
            mul -= shadow;
        }
*/
}
