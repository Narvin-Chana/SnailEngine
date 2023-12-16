#include "../ScreenSpaceDef.hlsli"

cbuffer PositionInfo
{
    matrix screenToNormMatrix;
    Rect screenSpaceCorners;
}

float2 GetVertexCoord(uint vertexId, Rect rect)
{
    const float2 vertexScreenSpace[] =
    {
        rect.topLeft, // topleft
        rect.botLeft, // bottomleft
        rect.botRight, // bottomright
        rect.topLeft, // topleft
        rect.botRight, // bottomright
        rect.topRight // topright
    };
    
    return vertexScreenSpace[vertexId];
}

Rect GetRectBounds(float2 pos, float2 size)
{
    Rect rect;
    rect.topLeft = pos;
    
    rect.topRight = rect.topLeft;
    rect.topRight.x += size.x;
    
    rect.botLeft = rect.topLeft;
    rect.botLeft.y += size.y;
    
    rect.botRight = pos + size;
    
    return rect;
}

Rect TransformRect(Rect corners, matrix transformMat)
{
    Rect rec;
    rec.topLeft = mul(float4(corners.topLeft, 1, 1), transformMat).xy;
    rec.topRight = mul(float4(corners.topRight, 1, 1), transformMat).xy;
    rec.botLeft = mul(float4(corners.botLeft, 1, 1), transformMat).xy;
    rec.botRight = mul(float4(corners.botRight, 1, 1), transformMat).xy;
    return rec;
}

Rect GetNormalizedCorners() {
    return TransformRect(screenSpaceCorners, screenToNormMatrix);
}