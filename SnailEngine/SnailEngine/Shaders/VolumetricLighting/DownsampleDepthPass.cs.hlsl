Texture2D fullResDepth : register(t0);
SamplerState fullResDepthSampler : register(s0);

RWTexture2D<float> downsampledDepth : register(u0);

[numthreads(8, 8, 1)]
void main(uint2 threadID : SV_DispatchThreadID)
{
    float2 dims;
    fullResDepth.GetDimensions(dims.x, dims.y);
    
    float4 depths = fullResDepth.Gather(fullResDepthSampler, float2(threadID * 2 / dims));

    GroupMemoryBarrierWithGroupSync();
    
    // Checkerboard downsample pattern as described in https://c0de517e.blogspot.com/2016/02/downsampled-effects-with-depth-aware.html
    downsampledDepth[threadID] = (threadID.x % 2 + (threadID.y % 2)) == 0 ? min(min(depths.x, depths.y), min(depths.z, depths.w)) : max(max(depths.x, depths.y), max(depths.z, depths.w));
}