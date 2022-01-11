
struct VS_IN
{
    float3 pos : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

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

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 nrm : NORMAL;
};

cbuffer SCENE : register(b0)
{
    matrix view;
    matrix projection;
};

struct MODEL_DATA
{
    matrix world;
    ATTRIBUTES attribs;
};

StructuredBuffer<MODEL_DATA> SceneData : register(t0);

VS_OUT main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0;
    
    float4 p = float4(input.pos, 1.0f);
    float2 u = input.uvw.xy;
    float3 n = normalize(input.nrm);
    
    //output.pos = float4(p.x, p.y - 0.75f, 0.0f, 1.0f);
    p = mul(SceneData[0].world, p);
    p = mul(view, p);
    p = mul(projection, p);
    
    output.pos = p;
    output.uv = u;
    output.nrm = mul(SceneData[0].world, float4(n, 0.0f)).xyz;
    
    return output;
}