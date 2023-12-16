Texture2D DownsampledDepth : register(t0);
SamplerState BilinearSampler : register(s0);
Texture2D<float3> HalfResolutionBuffer : register(t1);
SamplerState PointSampler : register(s1);
Texture2D FullResDepth : register(t2);

RWTexture2D<float3> PostUpsampledBuffer : register(u0);

static const float upsampleDepthThreshold = 0.0001;

[numthreads(8, 8, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID)
{
    // Get nearest half-res pixels
    float2 halfResDimensions;
    HalfResolutionBuffer.GetDimensions(halfResDimensions.x, halfResDimensions.y);
    float2 halfResUV = dispatchThreadID / 2.0f / halfResDimensions;
    
    float halfResPixelReds[4] = (float[4]) HalfResolutionBuffer.GatherRed(PointSampler, halfResUV).rgba;
    float halfResPixelGreens[4] = (float[4]) HalfResolutionBuffer.GatherGreen(PointSampler, halfResUV).rgba;
    float halfResPixelBlues[4] = (float[4]) HalfResolutionBuffer.GatherBlue(PointSampler, halfResUV).rgba;
            
    float2 fullResDimensions;
    PostUpsampledBuffer.GetDimensions(fullResDimensions.x, fullResDimensions.y);
    
    // Get current pixel fullResDepth (from fullres)
    float fullResDepth = FullResDepth.Load(float3(dispatchThreadID, 0)).r;

    // Get half-resolution depths for each halfResPixel
    float4 halfPixelDepths = DownsampledDepth.Gather(PointSampler, halfResUV);
    
    float diffs[4];
    diffs[0] = abs(halfPixelDepths.x - fullResDepth);
    diffs[1] = abs(halfPixelDepths.y - fullResDepth);
    diffs[2] = abs(halfPixelDepths.z - fullResDepth);
    diffs[3] = abs(halfPixelDepths.w - fullResDepth);
    
    float dmin = min(min(diffs[0], diffs[1]), min(diffs[2], diffs[3]));
    float dmax = max(max(diffs[0], diffs[1]), max(diffs[2], diffs[3]));
    
    // If under threshold we suppose that we're on a fullResDepth edge which means that we can just use bilinear sampler
    float diffd = dmax - dmin;
    float avg = dot(float4(diffs[0], diffs[1], diffs[2], diffs[3]), 0.25f.xxxx);
    bool d_edge = (diffd / avg) < upsampleDepthThreshold;
    
    [branch]
    if (d_edge)
    {
        PostUpsampledBuffer[dispatchThreadID] = HalfResolutionBuffer.SampleLevel(BilinearSampler, halfResUV, 0);
        return;
    }
    
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        if (dmin == diffs[i])
        {
            PostUpsampledBuffer[dispatchThreadID] = float3(halfResPixelReds[i], halfResPixelGreens[i], halfResPixelBlues[i]);
            return;
        }
    }
    
    PostUpsampledBuffer[dispatchThreadID] = HalfResolutionBuffer[dispatchThreadID * 0.5f];
}