
cbuffer ConstantBuffer : register(b0)
{
    float4x4 WVP;
};

Texture2D tex0 : register(t0);
SamplerState samLinear : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};



PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0f), WVP);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = tex0.Sample(samLinear, input.uv);
    //return float4(1, 0, 0, 1);
    //return float4(input.uv, 0, 1);
    return texColor;
}