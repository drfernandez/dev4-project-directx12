
struct ATTRIBUTES
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    uint illum; // illumination model
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 nrm : NORMAL;
};

cbuffer MESH_DATA : register(b0, space0)
{
    uint mesh_id;
    uint material_id;
};
cbuffer SCENE : register(b1, space0)
{
    matrix view;
    matrix projection;
};
StructuredBuffer<ATTRIBUTES> AttributesData : register(t0, space0);
StructuredBuffer<matrix> InstanceData : register(t1, space0);

float4 main(PS_IN input) : SV_TARGET
{
    float3 lightDir = normalize(float3(-1.0f, -1.0f, 2.0f));
    float4 lightColor = float4(0.9f, 0.9f, 1.0f, 1.0f);    
    float lightRatio = saturate(dot(-lightDir, input.nrm));
    
    ATTRIBUTES material = AttributesData[material_id];
    
    return saturate(lightColor * lightRatio * float4(material.Kd, material.d));
}