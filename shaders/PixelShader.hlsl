#include "../shaders/Lighting.hlsli"

struct PS_IN
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

TextureCube skybox : register(t3, space1);
Texture2D color_texture[] : register(t0, space1);
Texture2D normal_texture[] : register(t0, space2);
SamplerState filter : register(s0, space0);

float4 main(PS_IN input) : SV_TARGET
{
    ATTRIBUTES material = AttributesData[MeshData.material_id];
    float4 texture_color = color_texture[MeshData.texture_c_id].Sample(filter, input.uv);

    if (MeshData.has_texture_c)
    {
        material.Kd = texture_color.rgb;
        material.d = texture_color.a;
    }
    
    SURFACE surface = { input.wpos.xyz, normalize(input.nrm) };
    
    float4 luminance = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    uint numLights = uint(SceneData.cameraPosition.w);
    for (uint i = 0; i < numLights; i++)
    {
        LIGHT light = LightData[i];
        luminance += CalculateLight(material, light, surface);
        specular += CalculateSpecular(material, light, surface, SceneData.cameraPosition.xyz);
    }
    
    float4 diffuse = float4(material.Kd, material.d);
    float4 ambient = float4(material.Ka, 0.0f);
    float4 emissive = float4(material.Ke, 0.0f);
    float4 result = float4(luminance.xyz + ambient.xyz, 1.0f);
    
    return saturate((result * diffuse) + specular + emissive);
}