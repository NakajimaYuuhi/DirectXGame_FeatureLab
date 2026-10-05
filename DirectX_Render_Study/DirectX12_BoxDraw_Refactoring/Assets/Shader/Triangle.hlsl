// ==============================================================
// Triangle.hlsl - Mesh Rendering with Directional Lighting
// Half-Lambert Diffuse + Blinn-Phong Specular + Skinned Mesh Support
// ==============================================================

// ================================
// Constant Buffer (b0) - 52 floats
// ================================
cbuffer ObjectBuffer : register(b0)
{
    float4x4 WVP;         // 16 floats (0..15)
    float4x4 World;       // 16 floats (16..31)
    float2 uvOffset;      // 2 floats  (32..33)
    float2 uvScale;       // 2 floats  (34..35)
    float4 cameraPos;     // 4 floats  (36..39)  xyz: camera world pos
    float4 lightDir;      // 4 floats  (40..43)  xyz: light direction, w: intensity
    float4 lightColor;    // 4 floats  (44..47)  rgb: light color
    float4 ambientColor;  // 4 floats  (48..51)  rgb: ambient color, a: specular power
};

// ================================
// Resources
// ================================
Texture2D tex0 : register(t0);
SamplerState samLinear : register(s0);

// Bone Matrices StructuredBuffer (t1)
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);

// ================================
// Vertex Input / Output
// ================================
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
    float4 position    : SV_POSITION;
    float3 worldPos    : POSITION_WORLD;
    float3 worldNormal : NORMAL_WORLD;
    float2 uv          : TEXCOORD;
};

// ================================
// Vertex Shader (Skinning & Transform)
// ================================
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    float4 localPos = float4(input.position, 1.0f);
    float3 localNormal = input.normal;

    // Check if bone weights are active
    float totalWeight = input.boneWeights.x + input.boneWeights.y + input.boneWeights.z + input.boneWeights.w;
    
    float4 skinnedPos = localPos;
    float3 skinnedNormal = localNormal;

    if (totalWeight > 0.001f)
    {
        skinnedPos =
            mul(localPos, g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
            mul(localPos, g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
            mul(localPos, g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
            mul(localPos, g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;

        skinnedNormal =
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;
    }

    // Clip space position
    output.position = mul(skinnedPos, WVP);

    // World position and normal for lighting
    output.worldPos = mul(skinnedPos, World).xyz;
    output.worldNormal = normalize(mul(float4(skinnedNormal, 0.0f), World).xyz);

    // UV Transform
    output.uv = (input.uv * uvScale) + uvOffset;

    return output;
}

// ================================
// Pixel Shader (Half-Lambert + Blinn-Phong)
// ================================
float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = tex0.Sample(samLinear, input.uv);
    if (texColor.a < 0.05f)
    {
        discard;
    }

    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-lightDir.xyz); // Direction towards light source
    float3 V = normalize(cameraPos.xyz - input.worldPos);
    float3 H = normalize(L + V);

    // 1. Half-Lambert Diffuse Lighting
    // (dot(N, L) * 0.5 + 0.5)^2 softens shadow falloff and prevents black crushing
    float NdotL = dot(N, L);
    float halfLambert = saturate(NdotL * 0.5f + 0.5f);
    float diffuse = halfLambert * halfLambert;

    // 2. Blinn-Phong Specular Highlight
    float NdotH = saturate(dot(N, H));
    float specPower = max(ambientColor.a, 1.0f);
    float specular = pow(NdotH, specPower) * (NdotL > 0.0f ? 1.0f : 0.0f);

    // 3. Combine lighting terms
    float3 lightContribution   = lightColor.rgb * (diffuse * lightDir.w);
    float3 ambientContribution = ambientColor.rgb;
    float3 specularContribution = lightColor.rgb * (specular * 0.35f);

    float3 finalColor = texColor.rgb * (lightContribution + ambientContribution) + specularContribution;

    return float4(finalColor, texColor.a);
}