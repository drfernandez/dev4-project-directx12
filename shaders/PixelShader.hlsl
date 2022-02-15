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
    uint has_texture;
    uint texture_c_id;
    uint texture_n_id;
    uint texture_s_id;
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
Texture2D specular_texture[] : register(t0, space3);
SamplerState filter : register(s0, space0);

float4 main(PS_IN input) : SV_TARGET
{
    ATTRIBUTES material = AttributesData[MeshData.material_id];
    SURFACE surface = { input.wpos.xyz, normalize(input.nrm) };
    float HasSpecular = 1.0f;

    if (MeshData.has_texture & COLOR_FLAG)
    {
        float4 texture_color = color_texture[MeshData.texture_c_id].Sample(filter, input.uv);
        material.Kd = texture_color.rgb;
        material.d = texture_color.a;
    }

    if (MeshData.has_texture & NORMAL_FLAG)
    {
        float3 normal_color = normal_texture[MeshData.texture_n_id].Sample(filter, input.uv).xyz;
        float3 viewDirection = normalize(SceneData.cameraPosition.xyz - input.wpos);
        normal_color = normalize(normal_color * 2.0f - 1.0f);
        surface.normal = PerturbNormal(surface.normal, viewDirection, input.uv, normal_color);
    }

    if (MeshData.has_texture & SPECULAR_FLAG)
    {
        HasSpecular = specular_texture[MeshData.texture_s_id].Sample(filter, input.uv).x;
    }
    
    float4 luminance = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    uint numLights = uint(SceneData.cameraPosition.w);
    for (uint i = 0; i < numLights; i++)
    {
        LIGHT light = LightData[i];
        luminance += CalculateLight(material, light, surface);
        specular += CalculateSpecular(material, light, surface, SceneData.cameraPosition.xyz) * HasSpecular;
    }
    
    float4 diffuse = float4(material.Kd, material.d);
    float4 ambient = float4(material.Ka, 0.0f) * 0.25f;
    float4 emissive = float4(material.Ke, 0.0f);
    float4 result = float4(luminance.xyz + ambient.xyz, 1.0f);
    
    return saturate((result * diffuse) + specular + emissive);
}