// ==============================================================
// ShadowMap.hlsl - Directional Light Shadow Depth Rendering
// Supports both static meshes and skinned meshes
// ==============================================================

cbuffer ShadowObjectBuffer : register(b0)
{
    float4x4 LightWVP; // LightViewProj * World (16 floats)
};

// Bone Matrices StructuredBuffer (t1)
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);

struct VSInput
{
    float3 position    : POSITION;
    float3 normal      : NORMAL;
    float2 uv          : TEXCOORD;
    uint4  boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 localPos = float4(input.position, 1.0f);

    float totalWeight = input.boneWeights.x + input.boneWeights.y + input.boneWeights.z + input.boneWeights.w;
    float4 skinnedPos = localPos;

    if (totalWeight > 0.001f)
    {
        skinnedPos =
            mul(localPos, g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
            mul(localPos, g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
            mul(localPos, g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
            mul(localPos, g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;
    }

    output.position = mul(skinnedPos, LightWVP);
    return output;
}
