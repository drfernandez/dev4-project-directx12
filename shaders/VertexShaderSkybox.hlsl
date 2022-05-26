#include "../shaders/Lighting.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float3 uv : TEXCOORD;
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
    float3 u = input.pos;
    matrix w = InstanceData[MeshData.mesh_id + id];
    
    p = mul(w, p);
    p = mul(SceneData.view, p);
    p = mul(SceneData.projection, p);
    
    VS_OUT output = { p.xyww, u };
    return output;
}