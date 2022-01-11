
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 nrm : NORMAL;
};

float4 main(PS_IN input) : SV_TARGET
{
    float3 lightDir = normalize(float3(-1.0f, -1.0f, 2.0f));
    float4 lightColor = float4(0.9f, 0.9f, 1.0f, 1.0f);
    
    float lightRatio = saturate(dot(-lightDir, input.nrm));
    
    return saturate(lightColor * lightRatio);
}