#include "../shaders/Lighting.hlsli"

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
    float3 wpos : WORLDPOS;
};

struct MESH_DATA
{
    uint mesh_id;
    uint material_id;
    uint has_texture;
    uint diffuse_id;
    uint normal_id;
    uint specular_id;
};

struct SCENE
{
    matrix view;
    matrix projection;
    float4 cameraPosition;
};

ConstantBuffer<MESH_DATA> MeshData : register(b0, space0);
ConstantBuffer<SCENE> SceneData : register(b1, space0);
StructuredBuffer<ATTRIBUTES> AttributesData : register(t0, space0);
StructuredBuffer<matrix> InstanceData : register(t1, space0);
StructuredBuffer<LIGHT> LightData : register(t2, space0);

VS_OUT main(VS_IN input, uint id : SV_InstanceID)
{    
    float4 p = float4(input.pos, 1.0f);
    float2 u = input.uvw.xy;
    float3 n = input.nrm;
    float3 wp = float3(0, 0, 0);
    matrix w = InstanceData[MeshData.mesh_id + id];
    
    p = mul(w, p);
    wp = p.xyz;
    p = mul(SceneData.view, p);
    p = mul(SceneData.projection, p);
    n = mul(w, float4(n, 0.0f)).xyz;
    
    VS_OUT output = { p, u, n, wp };    
    return output;
}