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

struct PointLightData
{
    float4 position; // xyz: world pos, w: range
    float4 color;    // rgb: color, w: intensity
};

// ================================
// Light Buffer (b1) - Frame-wide Constant Buffer (512 bytes)
// ================================
cbuffer LightBuffer : register(b1)
{
    float4x4 LightViewProj; // 16 floats (0..15)
    float4   cameraPos;     // 4 floats  (16..19)  xyz: camera world pos
    float4   lightDir;      // 4 floats  (20..23)  xyz: light direction, w: intensity
    float4   lightColor;    // 4 floats  (24..27)  rgb: light color
    float4   ambientColor;  // 4 floats  (28..31)  rgb: ambient color, a: specular power
    float4   shadowParams;  // 4 floats  (32..35)  x: bias, y: darkness, z: mapSize, w: enabled
    float4   lightCounts;   // 4 floats  (36..39)  x: active point light count
    float4   reserved[6];   // 24 floats (40..63) -> 64 floats (256 bytes)

    PointLightData pointLights[8]; // 8 * 8 floats = 64 floats (256 bytes, total 512 bytes)
};

// ================================
// Material Buffer (b2) - Material Constants
// ================================
cbuffer MaterialBuffer : register(b2)
{
    float4   matBaseColor;     // 4 floats (0..3)   Base Color / Tint
    float2   matUvTiling;      // 2 floats (4..5)   UV Tiling
    float2   matUvOffset;      // 2 floats (6..7)   UV Offset
    float    matRoughness;     // 1 float  (8)      Roughness (0..1)
    float    matMetallic;      // 1 float  (9)      Metallic (0..1)
    float2   matPadding;       // 2 floats (10..11) 16-byte alignment
    float4   matCustomParams[2];// 8 floats (12..19) Custom shader parameters
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

    // UV Transform: combine Object uvScale/uvOffset with Material matUvTiling/matUvOffset
    output.uv = (input.uv * uvScale * matUvTiling) + uvOffset + matUvOffset;

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
    float4 texColor = tex0.Sample(samLinear, input.uv) * matBaseColor;
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

    // 2. Blinn-Phong Specular Highlight (modulated by roughness / metallic)
    float NdotH = saturate(dot(N, H));
    // Roughness inversely affects specPower (0.0 -> power 128, 1.0 -> power 4)
    float roughnessFactor = clamp(matRoughness, 0.01f, 1.0f);
    float specPower = max(ambientColor.a * (1.0f / (roughnessFactor * roughnessFactor)), 1.0f);
    float specular = pow(NdotH, specPower) * (NdotL > 0.0f ? 1.0f : 0.0f);

    // 3. Shadow Calculation
    float shadowFactor = CalculateShadow(input.shadowPos, N, L);

    // 4. Directional Light contribution
    // Direct light (diffuse & specular) is attenuated by shadow; ambient is unaffected
    float3 lightContribution    = lightColor.rgb * (diffuse * lightDir.w * shadowFactor);
    float3 ambientContribution  = ambientColor.rgb;
    float3 specularContribution = lightColor.rgb * (specular * 0.35f * shadowFactor);

    // 5. Point Lights (Smooth distance attenuation + Lambert diffuse + Blinn-Phong specular)
    float3 pointLightContribution = float3(0.0f, 0.0f, 0.0f);
    float3 pointSpecularContribution = float3(0.0f, 0.0f, 0.0f);
    int numPointLights = min(int(lightCounts.x), 8);

    [loop]
    for (int i = 0; i < numPointLights; ++i)
    {
        float3 lightPos   = pointLights[i].position.xyz;
        float  range      = pointLights[i].position.w;
        float3 pColor     = pointLights[i].color.rgb;
        float  pIntensity = pointLights[i].color.w;

        float3 toLight = lightPos - input.worldPos;
        float  dist    = length(toLight);

        if (dist < range && dist > 0.001f)
        {
            float3 pL = toLight / dist; // Direction towards point light source
            float3 pH = normalize(pL + V);

            // Smooth windowed distance attenuation: drops smoothly to 0 at dist == range
            float normDist   = dist / range;
            float attenWindow = saturate(1.0f - normDist * normDist);
            float smoothAtten = (attenWindow * attenWindow) / (dist * dist + 1.0f);

            // Diffuse
            float pNdotL  = saturate(dot(N, pL));
            float pDiffuse = pNdotL * (pIntensity * 10.0f) * smoothAtten;

            // Specular
            float pNdotH   = saturate(dot(N, pH));
            float pSpecular = pow(pNdotH, specPower) * (pNdotL > 0.0f ? 1.0f : 0.0f) * (pIntensity * 10.0f) * smoothAtten * 0.35f;

            pointLightContribution    += pColor * pDiffuse;
            pointSpecularContribution += pColor * pSpecular;
        }
    }

    float3 finalColor = texColor.rgb * (lightContribution + ambientContribution + pointLightContribution) + specularContribution + pointSpecularContribution;

    return float4(finalColor, texColor.a);
}
