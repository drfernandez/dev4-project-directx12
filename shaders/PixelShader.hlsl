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
    uint texture_id;
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

Texture2D diffuse[] : register(t0, space1);
SamplerState filter : register(s0, space0);

float4 main(PS_IN input) : SV_TARGET
{
    float4 texture_color = diffuse[MeshData.texture_id].Sample(filter, input.uv);
    ATTRIBUTES material = AttributesData[MeshData.material_id];
    material.Kd = texture_color.rgb;
    material.d = texture_color.a;
    
    SURFACE surface = (SURFACE) 0;
    surface.position = input.wpos.xyz;
    surface.normal = normalize(input.nrm);
    
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