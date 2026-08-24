// ================================
// Constant Buffer (b0)
// ================================
cbuffer ConstantBuffer : register(b0)
{
    float4x4 WVP;
    float2 uvOffset;
    float2 uvScale;
};

// ================================
// Resources
// ================================
Texture2D tex0 : register(t0);
SamplerState samLinear : register(s0);

// ボーン用 SRV（t1）
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);

// ================================
// Vertex Input
// ================================
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;

    uint4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

// ================================
// Pixel Input
// ================================
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// ================================
// Vertex Shader (Triangle.hlslと共通のスキニング処理)
// ================================
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // --- ボーン変形 ---
    float4 localPos = float4(input.position, 1.0f);

    float4 skinnedPos =
    mul(localPos, g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
    mul(localPos, g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
    mul(localPos, g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
    mul(localPos, g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;

    // WVP 変換
    output.position = mul(skinnedPos, WVP);
    output.uv = (input.uv * uvScale) + uvOffset;

    return output;
}

// ================================
// Pixel Shader (ワイヤーフレーム風)
// ================================
float4 PSMain(PSInput input) : SV_TARGET
{
    // UV座標を使って、擬似的な細かい網目（ワイヤーフレーム風）を作る
    // ※本来のラスタライザによるワイヤーフレームとは異なりますが、サイバーな見た目になります。
    
    float gridTiling = 20.0f; // 網目の細かさ
    float2 uv = frac(input.uv * gridTiling); 
    
    float edgeThickness = 0.1f; // 線の太さ
    
    // UVの端に近い部分を線（1.0）とする
    float isEdgeX = step(uv.x, edgeThickness) + step(1.0 - edgeThickness, uv.x);
    float isEdgeY = step(uv.y, edgeThickness) + step(1.0 - edgeThickness, uv.y);
    float isEdge = saturate(isEdgeX + isEdgeY);
    
    // 線の色（明るいネオングリーン）
    float4 wireColor = float4(0.0f, 1.0f, 0.2f, 1.0f);
    
    // ベースの色（暗い緑、または透明に近い色）
    float4 bgColor = float4(0.0f, 0.1f, 0.0f, 0.8f);
    
    // 線と背景を合成
    return lerp(bgColor, wireColor, isEdge);
}
