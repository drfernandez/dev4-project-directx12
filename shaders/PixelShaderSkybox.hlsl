#include "../shaders/Lighting.hlsli"

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 uv : TEXCOORD;
};

struct MESH_DATA
{
    uint mesh_id;
    uint material_id;
    uint has_texture_c;
    uint has_texture_n;
    uint texture_c_id;
    uint texture_n_id;
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

TextureCube skybox : register(t3, space0);
Texture2D color_texture[] : register(t0, space1);
Texture2D normal_texture[] : register(t0, space2);
SamplerState filter : register(s0, space0);

float4 main(PS_IN input) : SV_TARGET
{
    return skybox.Sample(filter, input.uv);
}