struct PS_CUBEMAP_IN
{
    float4 position : SV_Position;
	float3 worldPosition : POSITION;
	float3 normal : NORMAL;
    bool edgeVertex : PSIZE;
};

cbuffer param
{ 
	matrix matWorldViewProj;
	matrix matWorld;
	matrix invMatWorld;
}

float3 GetNormal(float3 A, float3 B, float3 C) { 
    float3 AB = normalize(B - A);
    float3 AC = normalize(C - A); 
    return normalize(cross(AB,AC)); 
}

[maxvertexcount(12)] 
void main(inout TriangleStream<PS_CUBEMAP_IN> Out, triangleadj PS_CUBEMAP_IN input[6])
{
    PS_CUBEMAP_IN pos[6];
    pos[0] = input[0];
    pos[1] = input[1];
    pos[2] = input[2];

    pos[3].position = lerp(pos[0].position, pos[1].position, 0.5f);
    pos[3].worldPosition = lerp(pos[0].worldPosition, pos[1].worldPosition, 0.5f);
    pos[3].normal = lerp(pos[0].normal, pos[1].normal, 0.5f);
    pos[3].edgeVertex = false;
    
    pos[4].position = lerp(pos[1].position, pos[2].position, 0.5f);
    pos[4].worldPosition = lerp(pos[1].worldPosition, pos[2].worldPosition, 0.5f);
    pos[4].normal = lerp(pos[1].normal, pos[2].normal, 0.5f);
    pos[4].edgeVertex = false;

    pos[5].position = lerp(pos[2].position, pos[0].position, 0.5f);
    pos[5].worldPosition = lerp(pos[2].worldPosition, pos[0].worldPosition, 0.5f);
    pos[5].normal = lerp(pos[2].normal, pos[0].normal, 0.5f);
    pos[5].edgeVertex = false;

    int indexes[12] = {
        0, 3, 5,
        3, 1, 4,
        4, 2, 5,
        3, 4, 5,
    };

    PS_CUBEMAP_IN vertex;
    for(uint i = 0; i < 4; i++)
    {
        vertex = pos[indexes[(i * 3)]];
        Out.Append(vertex);

        vertex = pos[indexes[(i * 3) + 1]];
        Out.Append(vertex);

        vertex = pos[indexes[(i * 3) + 2]];
        Out.Append(vertex);

        Out.RestartStrip();
    }
}