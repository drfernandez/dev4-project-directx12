
struct VS_IN
{
    float3 pos : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 nrm : NORMAL;
};

cbuffer VP : register(b0)
{
    matrix view;
    matrix projection;
};

cbuffer WORLD : register(b1)
{
    matrix world;
}

VS_OUT main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0;
    
    float4 p = float4(input.pos, 1.0f);
    float2 u = input.uvw.xy;
    float3 n = input.nrm;
    
    output.pos = p;
    output.uv = u;
    output.nrm = n;
    
    return output;
}