#ifndef __SCREEN_SPACE_DEF_HLSL__
#define __SCREEN_SPACE_DEF_HLSL__

struct Rect
{
    float2 topLeft; // topleft
    float2 topRight; // topleft
    float2 botLeft; // bottomleft
    float2 botRight; // bottomright
};

static const float2 uvScreenSpace[] =
{
    float2(0, 1), // topleft
    float2(0, 0), // bottomleft
    float2(1, 0), // bottomright
    float2(0, 1), // topleft
    float2(1, 0), // bottomright
    float2(1, 1) // topright
};

static const float2 vertexScreenSpace[] =
{
    float2(-1, -1), // topleft
    float2(-1, 1), // bottomleft
    float2(1, 1), // bottomright
    float2(-1, -1), // topleft
    float2(1, 1), // bottomright
    float2(1, -1) // topright
};

#endif