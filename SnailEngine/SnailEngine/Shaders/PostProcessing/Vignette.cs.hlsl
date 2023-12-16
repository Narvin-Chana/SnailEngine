RWTexture2D<float4> UAV : register(u0);

cbuffer VignetteParameters
{
    float4 color;
    float intensity;
    float radiusDivisionFactor;
};

[numthreads(32, 32, 1)]
void main(uint2 threadID : SV_DispatchThreadID)
{
    // Get center of the image
    float width, height;
    UAV.GetDimensions(width, height);
    float2 center = float2(width / 2.0, height / 2.0);

    // Distance from the current pixel to the center
    float distance = length(threadID - center);

    // Define the radius of the vignette effect and the intensity
    float radius = length(center) / radiusDivisionFactor;

    // Calculate the vignette factor based on the distance and radius
    float vignetteFactor = 1.0 - saturate(distance / radius);
    float4 vignetteColor = saturate(UAV[threadID] * vignetteFactor * color * intensity);

    UAV[threadID] = vignetteColor;
}
