
struct VS_IN
{
    float3 pos : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

struct ATTRIBUTES
{
    float3  Kd; // diffuse reflectivity
    float   d; // dissolve (transparency) 
    float3  Ks; // specular reflectivity
    float   Ns; // specular exponent
    float3  Ka; // ambient reflectivity
    float   sharpness; // local reflection map sharpness
    float3  Tf; // transmission filter
    float   Ni; // optical density (index of refraction)
    float3  Ke; // emissive reflectivity
    uint    illum; // illumination model
};

struct VS_OUT
{
    float4 pos : SV_Position;
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

VS_OUT main(VS_IN input, uint id : SV_InstanceID)
{    
    float4 p = float4(input.pos, 1.0f);
    float2 u = input.uvw.xy;
    float3 n = normalize(input.nrm);
    
    matrix w = InstanceData[mesh_id + id];
    p = mul(w, p);
    p = mul(view, p);
    p = mul(projection, p);
    n = mul(w, float4(n, 0.0f)).xyz;
    
    VS_OUT output = { p, u, n };
    
    return output;
}