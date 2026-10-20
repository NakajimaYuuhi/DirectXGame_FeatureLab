// ==============================================================
// GBuffer.hlsl - Deferred Geometry Buffer Generation (MRT)
// Target0: Albedo (RGB) + Specular Intensity (A)
// Target1: World Normal (RGB) + Roughness (A)
// Target2: Metallic (R) + AO (G) + Emissive (B) + Extra (A)
// ==============================================================

cbuffer ObjectBuffer : register(b0)
{
    float4x4 WVP;
    float4x4 World;
    float2   uvOffset;
    float2   uvScale;
};

cbuffer LightBuffer : register(b1)
{
    float4x4 LightViewProj;
    float4   cameraPos;
    float4   lightDir;
    float4   lightColor;
    float4   ambientColor;
    float4   shadowParams;
    float4   lightCounts;
    float4   reserved[6];
};

cbuffer MaterialBuffer : register(b2)
{
    float4   matBaseColor;
    float2   matUvTiling;
    float2   matUvOffset;
    float    matRoughness;
    float    matMetallic;
    float2   matPadding;
    float4   matCustomParams[2];
};

Texture2D tex0 : register(t0);
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);

SamplerState g_Sampler : register(s0);

struct VSInput
{
    float3 position     : POSITION;
    float3 normal       : NORMAL;
    float2 uv           : TEXCOORD;
    uint4  blendIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHT;
};

struct VSOutput
{
    float4 pos       : SV_POSITION;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD0;
    float3 worldPos  : POSITION0;
};

struct PSOutput
{
    float4 gAlbedo   : SV_Target0; // RGB: Albedo, A: Specular/Flags
    float4 gNormal   : SV_Target1; // RGB: World Normal, A: Roughness
    float4 gMaterial : SV_Target2; // R: Metallic, G: AO, B: Emissive, A: Extra
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float3 pos = input.position;
    float3 norm = input.normal;

    float totalWeight = input.blendWeights.x + input.blendWeights.y + input.blendWeights.z + input.blendWeights.w;
    if (totalWeight > 0.001f)
    {
        float4x4 skinMat = g_BoneMatrices[input.blendIndices.x] * input.blendWeights.x
                         + g_BoneMatrices[input.blendIndices.y] * input.blendWeights.y
                         + g_BoneMatrices[input.blendIndices.z] * input.blendWeights.z
                         + g_BoneMatrices[input.blendIndices.w] * input.blendWeights.w;

        pos = mul(skinMat, float4(pos, 1.0f)).xyz;
        norm = mul((float3x3)skinMat, norm);
    }

    output.pos = mul(WVP, float4(pos, 1.0f));
    output.worldPos = mul(World, float4(pos, 1.0f)).xyz;
    output.normal = normalize(mul((float3x3)World, norm));
    output.uv = input.uv * uvScale + uvOffset;

    return output;
}

PSOutput PSMain(VSOutput input)
{
    PSOutput output;

    float2 finalUV = input.uv * matUvTiling + matUvOffset;
    float4 texColor = tex0.Sample(g_Sampler, finalUV);
    float4 albedo = texColor * matBaseColor;

    if (albedo.a < 0.05f)
        discard;

    // Target0: Albedo (RGB) + Specular / Flags (A)
    output.gAlbedo = float4(albedo.rgb, 1.0f);

    // Target1: World Normal (RGB) + Roughness (A)
    float3 normal = normalize(input.normal);
    output.gNormal = float4(normal, matRoughness);

    // Target2: Metallic (R) + AO (G) + Emissive (B)
    output.gMaterial = float4(matMetallic, 1.0f, 0.0f, 1.0f);

    return output;
}
