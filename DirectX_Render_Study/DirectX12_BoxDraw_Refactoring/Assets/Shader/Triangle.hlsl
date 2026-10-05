// ==============================================================
// Triangle.hlsl - Mesh Rendering with Directional Lighting & PCF Shadow
// Half-Lambert Diffuse + Blinn-Phong Specular + 3x3 PCF Soft Shadow
// ==============================================================

// ================================
// Constant Buffer (b0) - 36 floats
// ================================
cbuffer ObjectBuffer : register(b0)
{
    float4x4 WVP;         // 16 floats (0..15)
    float4x4 World;       // 16 floats (16..31)
    float2   uvOffset;    // 2 floats  (32..33)
    float2   uvScale;     // 2 floats  (34..35)
};

// ================================
// Light Buffer (b1) - Frame-wide Constant Buffer
// ================================
cbuffer LightBuffer : register(b1)
{
    float4x4 LightViewProj; // 16 floats (0..15)
    float4   cameraPos;     // 4 floats  (16..19)  xyz: camera world pos
    float4   lightDir;      // 4 floats  (20..23)  xyz: light direction, w: intensity
    float4   lightColor;    // 4 floats  (24..27)  rgb: light color
    float4   ambientColor;  // 4 floats  (28..31)  rgb: ambient color, a: specular power
    float4   shadowParams;  // 4 floats  (32..35)  x: bias, y: darkness, z: mapSize, w: enabled
};

// ================================
// Resources
// ================================
Texture2D tex0 : register(t0);
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);
Texture2D shadowMap : register(t2);

SamplerState samLinear : register(s0);
SamplerState samShadow : register(s1);

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
    float4 shadowPos   : TEXCOORD1; // Light clip space position
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

    // Light projection for shadow mapping
    output.shadowPos = mul(float4(output.worldPos, 1.0f), LightViewProj);

    // UV Transform
    output.uv = (input.uv * uvScale) + uvOffset;

    return output;
}

// ================================
// 3x3 PCF Soft Shadow Calculation
// ================================
float CalculateShadow(float4 shadowPos, float3 N, float3 L)
{
    if (shadowParams.w < 0.5f) return 1.0f; // Shadow disabled

    // Perspective divide to NDC
    float3 projCoords = shadowPos.xyz / shadowPos.w;

    // Transform from [-1, 1] to [0, 1] UV space (DirectX Y is inverted)
    float2 uv = float2(projCoords.x * 0.5f + 0.5f, -projCoords.y * 0.5f + 0.5f);
    float currentDepth = projCoords.z;

    // Outside light frustum -> no shadow
    if (uv.x < 0.001f || uv.x > 0.999f || uv.y < 0.001f || uv.y > 0.999f || currentDepth > 1.0f || currentDepth < 0.0f)
    {
        return 1.0f;
    }

    // Adaptive slope bias based on surface angle to light
    float cosTheta = saturate(dot(N, L));
    float bias = max(shadowParams.x * (1.0f - cosTheta), shadowParams.x * 0.25f);

    float texelSize = 1.0f / shadowParams.z;
    float shadow = 0.0f;

    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = shadowMap.Sample(samShadow, uv + float2(x, y) * texelSize).r;
            // If receiver depth > shadow map depth + bias, surface is in shadow
            shadow += (currentDepth - bias > pcfDepth) ? shadowParams.y : 1.0f;
        }
    }
    shadow /= 9.0f;

    return shadow;
}

// ================================
// Pixel Shader (Half-Lambert + Blinn-Phong + PCF Shadow)
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
    float NdotL = dot(N, L);
    float halfLambert = saturate(NdotL * 0.5f + 0.5f);
    float diffuse = halfLambert * halfLambert;

    // 2. Blinn-Phong Specular Highlight
    float NdotH = saturate(dot(N, H));
    float specPower = max(ambientColor.a, 1.0f);
    float specular = pow(NdotH, specPower) * (NdotL > 0.0f ? 1.0f : 0.0f);

    // 3. Shadow Calculation
    float shadowFactor = CalculateShadow(input.shadowPos, N, L);

    // 4. Combine lighting terms
    // Direct light (diffuse & specular) is attenuated by shadow; ambient is unaffected
    float3 lightContribution    = lightColor.rgb * (diffuse * lightDir.w * shadowFactor);
    float3 ambientContribution  = ambientColor.rgb;
    float3 specularContribution = lightColor.rgb * (specular * 0.35f * shadowFactor);

    float3 finalColor = texColor.rgb * (lightContribution + ambientContribution) + specularContribution;

    return float4(finalColor, texColor.a);
}