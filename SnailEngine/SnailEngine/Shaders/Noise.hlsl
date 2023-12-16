#ifndef __NOISE_HLSL__
#define __NOISE_HLSL__

// Pseudo-random number generators sourced from http://obge.paradice-insight.us/wiki/Includes_%28Effects%29 & others
float rand_1_05(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}

float2 rand_2_10(in float2 uv)
{
    float noiseX = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    float noiseY = sqrt(1 - noiseX * noiseX);
    return float2(noiseX, noiseY);
}

float2 rand_2_0004(in float2 uv)
{
    float noiseX = (frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453));
    float noiseY = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return float2(noiseX, noiseY) * 0.004;
}

uint murmurHash12(uint2 src)
{
    const uint M = 0x5bd1e995u;
    uint h = 1190494759u;
    src *= M;
    src ^= src >> 24u;
    src *= M;
    h *= M;
    h ^= src.x;
    h *= M;
    h ^= src.y;
    h ^= h >> 13u;
    h *= M;
    h ^= h >> 15u;
    return h;
}

uint2 murmurHash22(uint2 src)
{
    const uint M = 0x5bd1e995u;
    uint2 h = uint2(1190494759u, 2147483647u);
    src *= M;
    src ^= src >> 24u;
    src *= M;
    h *= M;
    h ^= src.x;
    h *= M;
    h ^= src.y;
    h ^= h >> 13u;
    h *= M;
    h ^= h >> 15u;
    return h;
}

uint4 murmurHash42(uint2 src)
{
    const uint M = 0x5bd1e995u;
    uint4 h = uint4(1190494759u, 2147483647u, 3559788179u, 179424673u);
    src *= M;
    src ^= src >> 24u;
    src *= M;
    h *= M;
    h ^= src.x;
    h *= M;
    h ^= src.y;
    h ^= h >> 13u;
    h *= M;
    h ^= h >> 15u;
    return h;
}

float4 Hash42(float2 src)
{
    uint4 h = murmurHash42(asuint(src));
    return asfloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

float2 Hash22(float2 src)
{
    uint2 h = murmurHash22(asuint(src));
    return asfloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

float Hash12(float2 src)
{
    uint h = murmurHash12(asuint(src));
    return asfloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

float noise12(float2 p)
{
    float2 i = floor(p);

    float2 f = frac(p);
    float2 u = smoothstep(0.0.xx, 1.0.xx, f);

    float val = lerp(lerp(Hash12(i + float2(0.0, 0.0)),
                        Hash12(i + float2(1.0, 0.0)), u.x),
                   lerp(Hash12(i + float2(0.0, 1.0)),
                        Hash12(i + float2(1.0, 1.0)), u.x), u.y);
    return val * 2.0 - 1.0;
}

#endif