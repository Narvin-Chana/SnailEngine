cbuffer TransformMatrixes : register(b0)
{
    matrix matViewProj;
}

struct VSInput
{
    float3 position : POSITION;
    matrix modelMat : MODEL_MATRIX;
};

float4 main(VSInput input) : SV_Position
{
    return mul(float4(input.position, 1), mul(input.modelMat, matViewProj));
}
