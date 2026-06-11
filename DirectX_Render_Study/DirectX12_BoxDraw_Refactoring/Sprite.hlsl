// ================================
// Constant Buffer (b0)
// ================================
cbuffer ConstantBuffer : register(b0)
{
    float4x4 WVP;
    float4 color;
};

// ================================
// Resources
// ================================
Texture2D tex0 : register(t0);
SamplerState samLinear : register(s0);

// ================================
// Vertex Input
// ================================
struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// ================================
// Vertex Shader
// ================================
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // WVP •ÏŠ· (Orthographic Projection)
    output.position = mul(float4(input.position, 1.0f), WVP);
    output.uv = input.uv;

    return output;
}

// ================================
// Pixel Shader
// ================================
float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = tex0.Sample(samLinear, input.uv);
    return texColor * color;
}
