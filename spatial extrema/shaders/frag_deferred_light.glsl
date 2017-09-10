#version 430 core

#define shadowbuffer 0
#define NUM_CASCADES 3

#define FOG_DIVISOR 256.0

#import module_math
#import module_noise

in vec2 UV;

uniform float u_cascades[NUM_CASCADES + 1];

uniform sampler2D u_diffuse;
uniform sampler2D u_normal;
uniform sampler2D u_position;
uniform sampler2D u_shadowDepths[NUM_CASCADES];
uniform sampler2D u_linearDepth;
uniform sampler2D u_rayDir;

layout (location = 0) out vec4 fragColour;

struct light
{
  vec4 pos;
  vec3 col;
  float lum;
};
layout( std140 ) uniform lightBuffer
{
  light buf [512];
} lbuf;
uniform int u_lbufLen;

uniform vec3 u_lightDir;
uniform vec3 u_lightCol;
uniform float u_lightInts;

uniform mat4 u_shadowMatrix[NUM_CASCADES];

uniform vec4 u_camPos;

//UVs per-pixel
uniform vec2 u_pixstep;

vec3 getEyeRay()
{
  return texture(u_rayDir, UV).xyz;
}

float shadowSample(float depth, vec2 smpl, ivec2 offset, int index)
{
  if(depth > texture(u_shadowDepths[index], smpl + offset * u_pixstep).r)
    return 1.0;
  return 0.0;
}

vec3 basicLight(vec4 fragPos, vec3 fragNormal, int lightIndex)
{
  vec4 lp = lbuf.buf[ lightIndex ].pos;
  float d = distanceSquared(fragPos.xyz, lp.xyz);

  //Light cutoff threshold
  if(lbuf.buf[ lightIndex ].lum / d < 0.001)
    return vec3(0.0);

  vec4 lv = lp - fragPos;
  lv = normalize(lv);
  float mul = dot( lv, vec4(fragNormal.xyz, 0.0) );
  mul = clamp(mul, 0.0, 1.0);

  vec3 base = lbuf.buf[ lightIndex ].col.xyz;
  d = distance(lp, fragPos);
  d *= d;
  return (mul * lbuf.buf[ lightIndex ].lum * base) / (1.0 + d);
}

void main()
{
    fragColour = vec4(1.0,1.0,0.0,1.0);
    return;
  fragColour = vec4(texture(u_diffuse, UV));
  vec4 fragPos = texture(u_position, UV);
  vec3 fragNorm = texture(u_normal, UV).xyz;
  float fragDepth = texture(u_linearDepth, UV).r;

#if shadowbuffer == 0
  if(fragColour.a == 0.0)
    discard;
#endif
  float lightmul = clamp(dot(fragNorm, u_lightDir), 0.0, 1.0);
  float moonmul = clamp(dot(fragNorm, -u_lightDir), 0.0, 1.0);
  float mul = lightmul + moonmul;
  mul = clamp(mul, 0.0, 1.0);
  float bias = 0.005 * tan( acos( mul ) );
  bias = clamp( bias, 0.0, 0.001);

  lightmul = clamp(dot(fragNorm, u_lightDir), 0.0, 1.0) * u_lightInts;
  mul = lightmul + moonmul;

  int cascadeIndex = -1;

  if(fragDepth > u_cascades[0] && fragDepth < u_cascades[1])
    cascadeIndex = 0;
  else if(fragDepth < u_cascades[2])
    cascadeIndex = 1;
  else if(fragDepth < u_cascades[3])
    cascadeIndex = 2;

  if(cascadeIndex != -1)
  {
    vec4 su_position = u_shadowMatrix[cascadeIndex] * vec4(fragPos.xyz, 1.0);

    float depth = su_position.z - bias;

    float shadow = shadowSample(depth, su_position.xy, ivec2(0, 0), cascadeIndex);
    shadow += shadowSample(depth, su_position.xy, ivec2(0, 1), cascadeIndex);
    shadow += shadowSample(depth, su_position.xy, ivec2(1, 0), cascadeIndex);
    shadow += shadowSample(depth, su_position.xy, ivec2(0, -1), cascadeIndex);
    shadow += shadowSample(depth, su_position.xy, ivec2(-1, 0), cascadeIndex);

    shadow /= 5.0;
    mul -= shadow;
  }

#if shadowbuffer == 0

  mul = max(mul, 0.35 / (lightmul + moonmul + 1.0));
  fragColour.xyz *= mul;
  fragColour.xyz *= u_lightCol;

  for(int i = 0; i < u_lbufLen; ++i)
  {
    fragColour.xyz += basicLight(fragPos, fragNorm, i);
  }
#endif

  //Fog
  float a = clamp(texture(u_linearDepth, UV).r / FOG_DIVISOR, 0.0, 1.0);
  fragColour.xyz = mix(fragColour.xyz, 0.8 * clamp(u_lightInts, 0.0, 1.0) * u_lightCol, a);
  //fragColour.xyz = vec3(texture(u_linearDepth, UV).r);

#if shadowbuffer == 1
  fragColour.xyz = vec3(texture(u_shadowDepths[0], UV).r);
#endif

  //fragColour.xyz = vec3(fragPos.xyz) / 16.0;

  fragColour.a = 1.0;
}
