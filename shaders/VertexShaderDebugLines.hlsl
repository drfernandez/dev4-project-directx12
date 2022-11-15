
struct VS_IN
{
    float4 pos : POSITION;
    float4 col : COLOR;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

struct SCENE
{
    matrix view;
    matrix projection;
    float4 cameraPosition;
};

ConstantBuffer<SCENE> SceneData : register(b1, space0);


VS_OUT main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0;
    float4 p = float4(input.pos.xyz, 1);
    p = mul(SceneData.view, p);
    p = mul(SceneData.projection, p);
    
    output.pos = p;
    output.col = input.col;    
    
    return output;
}