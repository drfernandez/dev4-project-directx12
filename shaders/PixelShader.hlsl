
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 nrm : NORMAL;
};

float4 main(PS_IN input) : SV_TARGET
{
    return float4(0.5f, 0.25f, 0.75f, 0);
}