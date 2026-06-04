// ================================
// Constant Buffer (b0)
// ================================
cbuffer ConstantBuffer : register(b0)
{
    float4x4 WVP;
};

// ================================
// Resources
// ================================
Texture2D tex0 : register(t0);
SamplerState samLinear : register(s0);

// ボーン行列 SRV（t1）
//StructuredBuffer<float4x4> g_BoneMatrices : register(t1);
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);

// ================================
// Vertex Input
// ================================
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;

    uint4 boneIndices : BLENDINDICES; // DX11/12 の標準セマンティクス
    float4 boneWeights : BLENDWEIGHT;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;


};


// ================================
// Vertex Shader (Skinning)
// ================================
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    //output.position = mul(float4(input.position, 1.0f), WVP);
    
    // --- ボーン変形 ---
    float4 localPos = float4(input.position, 1.0f);

    float4 skinnedPos =
    mul(localPos, g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
    mul(localPos, g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
    mul(localPos, g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
    mul(localPos, g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;

    //float4 skinnedPos =
    //mul(g_BoneMatrices[input.boneIndices.x], localPos) * input.boneWeights.x +
    //mul(g_BoneMatrices[input.boneIndices.y], localPos) * input.boneWeights.y +
    //mul(g_BoneMatrices[input.boneIndices.z], localPos) * input.boneWeights.z +
    //mul(g_BoneMatrices[input.boneIndices.w], localPos) * input.boneWeights.w;
    
    // 【テスト用】StructuredBufferの値を無視して、強制的にIdentityでスキニングする
    float4x4 identityMatrix = float4x4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
    );

    // 順番は mul(行列, ベクトル) で合わせます
    //float4 skinnedPos =
    //mul(identityMatrix, localPos) * input.boneWeights.x +
    //mul(identityMatrix, localPos) * input.boneWeights.y +
    //mul(identityMatrix, localPos) * input.boneWeights.z +
    //mul(identityMatrix, localPos) * input.boneWeights.w;
    
    //float4 skinnedPos = mul(identityMatrix, localPos);
    

    
    // WVP 変換
    //output.position = mul(localPos, WVP);
    output.position = mul(skinnedPos, WVP);
    
    
    
    
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