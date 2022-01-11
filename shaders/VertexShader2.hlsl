#pragma pack_matrix(row_major)

#define MAX_MESH_PER_DRAW 1024
#define MAX_LIGHT_PER_DRAW 1024
struct LIGHT
{
    float4 position;
    float4 direction;
    float4 attributes;
    float4 color;
};

struct ATTRIBUTES
{
	float3 Kd;			// diffuse reflectivity
	float d;			// dissolve (transparency) 
	float3 Ks;			// specular reflectivity
	float Ns;			// specular exponent
	float3 Ka;			// ambient reflectivity
	float sharpness;	// local reflection map sharpness
	float3 Tf;			// transmission filter
	float Ni;			// optical density (index of refraction)
	float3 Ke;			// emissive reflectivity
	uint illum;			// illumination model
};

struct SHADER_DATA
{
    float4 lightDirection, lightColor, lightCount;
	float4 ambient, camPos;
	matrix view, projection;
    matrix matricies[MAX_MESH_PER_DRAW];
    ATTRIBUTES materials[MAX_MESH_PER_DRAW];
    LIGHT lights[MAX_LIGHT_PER_DRAW];
};

//StructuredBuffer<SHADER_DATA> SceneData : register(t0);
ByteAddressBuffer SceneData : register(t0);

//[[vk::push_constant]]
cbuffer MESH_INDEX
{
	uint mesh_ID;
    uint material_ID;
    uint hasColorTexture;
};

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
	float3 world_pos : WORLD;
};

VS_OUT main(VS_IN input, uint instanceID : SV_InstanceID)
{
	VS_OUT output = (VS_OUT)0;

	output.pos = float4(input.pos, 1);

	//output.pos = mul(output.pos, SceneData[0].matricies[mesh_ID + instanceID]);
	output.world_pos = output.pos.xyz;
	//output.pos = mul(output.pos, SceneData[0].view);
	//output.pos = mul(output.pos, SceneData[0].projection);
	output.uv = input.uvw.xy;
	//output.nrm = mul(input.nrm, (float3x3)SceneData[0].matricies[mesh_ID + instanceID]);

	return output;
}