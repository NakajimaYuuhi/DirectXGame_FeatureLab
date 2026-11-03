// ==============================================================
// Triangle.hlsl - Multi-Technique Shadow Mapping & Stylized Lighting
// Supports:
//   0: Off
//   1: Projective Shadow (ìäâeÉVÉÉÉhÉE - ê[ìxî‰ärÇ»Çµ)
//   2: Hard Depth Shadow (ê[ìxÉVÉÉÉhÉE - 1-tapê[ìxî‰är)
//   3: PCF Soft Shadow   (PCFÉVÉÉÉhÉE - 3x3 / 5x5 / 16-tap Poisson Disk)
//   4: PCSS              (Contact-Hardening Soft Shadow - ê⁄ínçdâªÅEãóó£É{ÉP)
//   5: CSM               (3-Cascade Shadow Maps Near/Mid/Far + ï™äÑâ¬éãâª)
//   6: Blob Shadow       (ä€âe - î‰ärópä»à’ìäâe)
// ==============================================================
// @Param(0, "Shadow PCF Spread", -0.8, 3.0)
// @Param(1, "Toon Shadow Step", 0.0, 1.0)
// @Param(2, "Rim Light Power", 0.0, 4.0)
// @Color(1, "Shadow Tint Color")

// ================================
// Constant Buffer (b0) - 36 floats
// ================================
cbuffer ObjectBuffer : register(b0)
{
    float4x4 WVP;         // 16 floats (0..15)
    float4x4 World;       // 16 floats (16..31) (World[3][3] == 2.0 -> Ground/Field receiver)
    float2   uvOffset;    // 2 floats  (32..33)
    float2   uvScale;     // 2 floats  (34..35)
};

struct PointLightData
{
    float4 position; // xyz: world pos, w: range
    float4 color;    // rgb: color, w: intensity
};

// ================================
// Light Buffer (b1) - Frame-wide Constant Buffer (768 bytes)
// ================================
cbuffer LightBuffer : register(b1)
{
    float4x4 LightViewProj;          // 16 floats (0..15)  Cascade 0 / Single LightViewProj
    float4   cameraPos;              // 4 floats  (16..19) xyz: camera world pos
    float4   lightDir;               // 4 floats  (20..23) xyz: light direction, w: intensity
    float4   lightColor;             // 4 floats  (24..27) rgb: light color
    float4   ambientColor;           // 4 floats  (28..31) rgb: ambient color, a: specular power
    float4   shadowParams;           // 4 floats  (32..35) x: bias, y: darkness, z: mapSize, w: shadowMode (0..6)
    float4   lightCounts;            // 4 floats  (36..39) x: pointLightCount, y: blobCasterCount, z: visualizeCascades, w: projectiveGroundOnly
    float4   shadowExtraParams;      // 4 floats  (40..43) x: normalBias, y: pcssLightSize, z: pcfKernelMode, w: selfShadowStrength
    float4   cascadeSplits;          // 4 floats  (44..47) x: split0Dist, y: split1Dist, z: split2Dist, w: orthoSize
    float4   blobCasters[4];         // 16 floats (48..63) xyz: caster world pos, w: radius

    PointLightData pointLights[8];   // 64 floats (64..127)

    float4x4 LightViewProjCascade1;  // 16 floats (128..143) Cascade 1 (Mid)
    float4x4 LightViewProjCascade2;  // 16 floats (144..159) Cascade 2 (Far)
    float4   csmPadding[8];          // 32 floats (160..191)
};

// ================================
// Material Buffer (b2) - Material Constants
// ================================
cbuffer MaterialBuffer : register(b2)
{
    float4   matBaseColor;      // 4 floats (0..3)   Base Color / Tint
    float2   matUvTiling;       // 2 floats (4..5)   UV Tiling
    float2   matUvOffset;       // 2 floats (6..7)   UV Offset
    float    matRoughness;      // 1 float  (8)      Roughness (0..1)
    float    matMetallic;       // 1 float  (9)      Metallic (0..1)
    float2   matPadding;        // 2 floats (10..11) 16-byte alignment
    float4   matCustomParams[2];// 8 floats (12..19) Custom shader parameters
};

// ================================
// Resources
// ================================
Texture2D tex0       : register(t0);
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);
Texture2D shadowMap  : register(t2); // Cascade 0 (Near) / Primary Shadow Map
Texture2D shadowMap1 : register(t3); // Cascade 1 (Mid)
Texture2D shadowMap2 : register(t4); // Cascade 2 (Far)

SamplerState samLinear : register(s0);
SamplerState samShadow : register(s1);

// 16-tap Poisson Disk samples for high-quality PCF & PCSS
static const float2 g_PoissonDisk[16] =
{
    float2(-0.94201624f, -0.39906216f),
    float2( 0.94558609f, -0.76890725f),
    float2(-0.09418410f, -0.92938870f),
    float2( 0.34495938f,  0.29387760f),
    float2(-0.91588581f,  0.45771432f),
    float2(-0.81544232f, -0.87912464f),
    float2(-0.38277543f,  0.27676845f),
    float2( 0.97484398f,  0.75648379f),
    float2( 0.44323325f, -0.97511554f),
    float2( 0.53742981f, -0.47373420f),
    float2(-0.26496911f, -0.41893023f),
    float2( 0.79197514f,  0.19090188f),
    float2(-0.24188840f,  0.99706507f),
    float2(-0.81409955f,  0.91437590f),
    float2( 0.19984126f,  0.78641367f),
    float2( 0.14383161f, -0.14100790f)
};

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
    float2 uv          : TEXCOORD0;
    float4 shadowPos   : TEXCOORD1; // Light clip space position (with Normal Offset Bias)
    float  isGround    : TEXCOORD2; // 1.0 if Ground/Field, 0.0 if Character/Model
};

// ================================
// Vertex Shader (Skinning & Transform)
// ================================
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    float4 localPos = float4(input.position, 1.0f);
    float3 localNormal = input.normal;

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

    // Extract receiver type encoded in World[3][3] before world transform
    output.isGround = (World[3][3] > 1.5f) ? 1.0f : 0.0f;

    // World position and normal for lighting
    output.worldPos = mul(skinnedPos, World).xyz;
    output.worldNormal = normalize(mul(float4(skinnedNormal, 0.0f), World).xyz);

    // Normal Offset Shadow Bias: shifts shadow lookup point slightly along surface normal
    // to prevent shadow acne on curved 3D models without causing Peter Panning!
    float3 L_dir = normalize(-lightDir.xyz);
    float  cosTheta = saturate(dot(output.worldNormal, L_dir));
    float  sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
    float3 biasedWorldPos = output.worldPos + output.worldNormal * (shadowExtraParams.x * (0.35f + 0.65f * sinTheta));

    output.shadowPos = mul(float4(biasedWorldPos, 1.0f), LightViewProj);

    // UV Transform: combine Object uvScale/uvOffset with Material matUvTiling/matUvOffset
    output.uv = (input.uv * uvScale * matUvTiling) + uvOffset + matUvOffset;

    return output;
}

// Helper: Sample specific cascade texture (0, 1, or 2)
float SampleCascadeDepth(int cascadeIdx, float2 uv)
{
    if (cascadeIdx == 1) return shadowMap1.SampleLevel(samShadow, uv, 0).r;
    if (cascadeIdx == 2) return shadowMap2.SampleLevel(samShadow, uv, 0).r;
    return shadowMap.SampleLevel(samShadow, uv, 0).r;
}

// Helper: Convert light clip-space position to UV and NDC depth
bool ProjectToShadowUV(float4 clipPos, out float2 outUV, out float outDepth)
{
    float3 projCoords = clipPos.xyz / clipPos.w;
    outUV = float2(projCoords.x * 0.5f + 0.5f, -projCoords.y * 0.5f + 0.5f);
    outDepth = projCoords.z;
    return (outUV.x >= 0.002f && outUV.x <= 0.998f &&
            outUV.y >= 0.002f && outUV.y <= 0.998f &&
            outDepth >= 0.0f && outDepth <= 1.0f);
}

// ================================
// Multi-Mode Shadow Calculation
// ================================
float CalculateShadow(PSInput input, float3 N, float3 L, out int outCascadeIdx)
{
    outCascadeIdx = 0;
    int mode = int(shadowParams.w + 0.5f);
    if (mode <= 0) return 1.0f; // 0: Shadow Off

    // --------------------------------------------------------
    // Mode 6: Blob Shadow (ä€âe - î‰ärópä»à’ìäâe)
    // --------------------------------------------------------
    if (mode == 6)
    {
        int numCasters = min(int(lightCounts.y + 0.5f), 4);
        float minBlobFactor = 1.0f;
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            if (i < numCasters)
            {
                float3 casterPos = blobCasters[i].xyz;
                float  radius    = max(blobCasters[i].w, 0.2f);
                float  dy        = input.worldPos.y - casterPos.y;
                // Project onto ground/surfaces near or below the caster's feet
                if (dy <= 0.25f && dy >= -4.0f)
                {
                    float distXZ = length(input.worldPos.xz - casterPos.xz);
                    if (distXZ < radius)
                    {
                        float normD = distXZ / radius;
                        float heightFade = saturate(1.0f - abs(dy) * 0.25f);
                        float blob = smoothstep(0.15f, 1.0f, normD);
                        float factor = lerp(shadowParams.y, 1.0f, lerp(1.0f, blob, heightFade));
                        minBlobFactor = min(minBlobFactor, factor);
                    }
                }
            }
        }
        return minBlobFactor;
    }

    float2 uv;
    float currentDepth;
    bool insideFrustum = ProjectToShadowUV(input.shadowPos, uv, currentDepth);

    // Adaptive slope depth bias
    float cosTheta = saturate(dot(N, L));
    float bias = max(shadowParams.x * (1.0f - cosTheta), shadowParams.x * 0.25f);
    float pcfSpread = max(0.2f, 1.0f + matCustomParams[0].x);
    float baseTexelSize = (1.0f / max(shadowParams.z, 1.0f)) * pcfSpread;

    // --------------------------------------------------------
    // Mode 1: Projective Shadow (ìäâeÉVÉÉÉhÉE - ê[ìxî‰ärÇ»Çµ)
    // --------------------------------------------------------
    if (mode == 1)
    {
        if (!insideFrustum) return 1.0f;
        // If "Ground Only Receiver" is enabled, only project onto Ground (Field)
        if (lightCounts.w > 0.5f && input.isGround < 0.5f)
        {
            return 1.0f;
        }
        // Pure silhouette projection: checks if any caster was written to the shadow map (< 0.999)
        // WITHOUT comparing receiver depth vs caster depth!
        float projectedDepth = shadowMap.SampleLevel(samShadow, uv, 0).r;
        return (projectedDepth < 0.999f) ? shadowParams.y : 1.0f;
    }

    // --------------------------------------------------------
    // Mode 2: Hard Depth Shadow (ê[ìxÉVÉÉÉhÉE - 1-tapê[ìxî‰är)
    // --------------------------------------------------------
    if (mode == 2)
    {
        if (!insideFrustum) return 1.0f;
        float closestDepth = shadowMap.SampleLevel(samShadow, uv, 0).r;
        return (currentDepth - bias > closestDepth) ? shadowParams.y : 1.0f;
    }

    // --------------------------------------------------------
    // Mode 3: PCF Soft Shadow (3x3 / 5x5 / 16-tap Poisson Disk)
    // --------------------------------------------------------
    if (mode == 3)
    {
        if (!insideFrustum) return 1.0f;
        int kernelMode = int(shadowExtraParams.z + 0.5f);

        if (kernelMode == 0)
        {
            // 3x3 Box PCF (9 taps)
            float shadow = 0.0f;
            [unroll]
            for (int x = -1; x <= 1; ++x)
            {
                [unroll]
                for (int y = -1; y <= 1; ++y)
                {
                    float pcfDepth = shadowMap.SampleLevel(samShadow, uv + float2(x, y) * baseTexelSize, 0).r;
                    shadow += (currentDepth - bias > pcfDepth) ? shadowParams.y : 1.0f;
                }
            }
            return shadow / 9.0f;
        }
        else if (kernelMode == 1)
        {
            // 5x5 Box PCF (25 taps)
            float shadow = 0.0f;
            [unroll]
            for (int x = -2; x <= 2; ++x)
            {
                [unroll]
                for (int y = -2; y <= 2; ++y)
                {
                    float pcfDepth = shadowMap.SampleLevel(samShadow, uv + float2(x, y) * (baseTexelSize * 0.75f), 0).r;
                    shadow += (currentDepth - bias > pcfDepth) ? shadowParams.y : 1.0f;
                }
            }
            return shadow / 25.0f;
        }
        else
        {
            // 16-tap Rotated Poisson Disk PCF
            float noiseAngle = frac(52.9829189f * frac(dot(input.position.xy, float2(0.06711056f, 0.00583715f)))) * 6.2831853f;
            float s = sin(noiseAngle);
            float c = cos(noiseAngle);
            float2x2 rotMat = float2x2(c, -s, s, c);

            float radius = baseTexelSize * 2.2f;
            float shadow = 0.0f;
            [unroll]
            for (int i = 0; i < 16; ++i)
            {
                float2 offset = mul(g_PoissonDisk[i], rotMat) * radius;
                float pcfDepth = shadowMap.SampleLevel(samShadow, uv + offset, 0).r;
                shadow += (currentDepth - bias > pcfDepth) ? shadowParams.y : 1.0f;
            }
            return shadow / 16.0f;
        }
    }

    // --------------------------------------------------------
    // Mode 4: PCSS (Percentage-Closer Soft Shadows - ê⁄ínçdâªÉ\ÉtÉgÉVÉÉÉhÉE)
    // --------------------------------------------------------
    if (mode == 4)
    {
        if (!insideFrustum) return 1.0f;

        float noiseAngle = frac(52.9829189f * frac(dot(input.position.xy, float2(0.06711056f, 0.00583715f)))) * 6.2831853f;
        float s = sin(noiseAngle);
        float c = cos(noiseAngle);
        float2x2 rotMat = float2x2(c, -s, s, c);

        // Step 1: Blocker Search
        float lightSize = max(shadowExtraParams.y, 0.1f);
        float searchRadius = (1.0f / max(shadowParams.z, 1.0f)) * lightSize * 8.0f;
        float blockerSum = 0.0f;
        int   blockerCount = 0;

        [unroll]
        for (int i = 0; i < 16; ++i)
        {
            float2 sampleUV = uv + mul(g_PoissonDisk[i], rotMat) * searchRadius;
            float sampleDepth = shadowMap.SampleLevel(samShadow, sampleUV, 0).r;
            if (sampleDepth < currentDepth - bias)
            {
                blockerSum += sampleDepth;
                blockerCount++;
            }
        }

        // No blockers in neighborhood -> fully lit!
        if (blockerCount == 0) return 1.0f;

        // Step 2: Penumbra Size Estimation (contact = sharp, far from blocker = soft)
        float avgBlockerDepth = blockerSum / float(blockerCount);
        float depthDiff = max(0.0f, currentDepth - avgBlockerDepth);
        // Scale depth difference (in [0,1] ortho depth range ~80m) into texel filter radius
        float penumbraWidth = clamp(depthDiff * 180.0f * lightSize, 0.6f, 16.0f);
        float filterRadius = penumbraWidth * baseTexelSize;

        // Step 3: Variable-Radius Poisson PCF
        float shadow = 0.0f;
        [unroll]
        for (int j = 0; j < 16; ++j)
        {
            float2 offset = mul(g_PoissonDisk[j], rotMat) * filterRadius;
            float pcfDepth = shadowMap.SampleLevel(samShadow, uv + offset, 0).r;
            shadow += (currentDepth - bias > pcfDepth) ? shadowParams.y : 1.0f;
        }
        return shadow / 16.0f;
    }

    // --------------------------------------------------------
    // Mode 5: CSM (3-Cascade Shadow Maps Near / Mid / Far)
    // --------------------------------------------------------
    if (mode == 5)
    {
        float3 L_dir = normalize(-lightDir.xyz);
        float  sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
        float3 biasedWorldPos = input.worldPos + N * (shadowExtraParams.x * (0.35f + 0.65f * sinTheta));

        float2 cUV = uv;
        float  cDepth = currentDepth;
        int    cascadeIdx = -1;

        // Check Cascade 0 (Near - High Res)
        float4 posC0 = mul(float4(biasedWorldPos, 1.0f), LightViewProj);
        float2 uv0; float d0;
        if (ProjectToShadowUV(posC0, uv0, d0) &&
            uv0.x > 0.03f && uv0.x < 0.97f && uv0.y > 0.03f && uv0.y < 0.97f)
        {
            cascadeIdx = 0;
            cUV = uv0;
            cDepth = d0;
        }
        else
        {
            // Check Cascade 1 (Mid)
            float4 posC1 = mul(float4(biasedWorldPos, 1.0f), LightViewProjCascade1);
            float2 uv1; float d1;
            if (ProjectToShadowUV(posC1, uv1, d1) &&
                uv1.x > 0.03f && uv1.x < 0.97f && uv1.y > 0.03f && uv1.y < 0.97f)
            {
                cascadeIdx = 1;
                cUV = uv1;
                cDepth = d1;
            }
            else
            {
                // Check Cascade 2 (Far)
                float4 posC2 = mul(float4(biasedWorldPos, 1.0f), LightViewProjCascade2);
                float2 uv2; float d2;
                if (ProjectToShadowUV(posC2, uv2, d2))
                {
                    cascadeIdx = 2;
                    cUV = uv2;
                    cDepth = d2;
                }
            }
        }

        if (cascadeIdx < 0)
        {
            outCascadeIdx = 2;
            return 1.0f;
        }

        outCascadeIdx = cascadeIdx;
        float cascadeBiasScale = (cascadeIdx == 0) ? 0.75f : ((cascadeIdx == 1) ? 1.25f : 2.2f);
        float cBias = bias * cascadeBiasScale;

        float shadow = 0.0f;
        [unroll]
        for (int i = 0; i < 16; ++i)
        {
            float2 offset = g_PoissonDisk[i] * (baseTexelSize * 1.6f);
            float pcfDepth = SampleCascadeDepth(cascadeIdx, cUV + offset);
            shadow += (cDepth - cBias > pcfDepth) ? shadowParams.y : 1.0f;
        }
        return shadow / 16.0f;
    }

    return 1.0f;
}

// ================================
// Pixel Shader (Half-Lambert + Blinn-Phong + Multi-Mode Shadow)
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

    // 1. Half-Lambert Diffuse Lighting (with optional Toon Step via @Param(1))
    float NdotL = dot(N, L);
    float halfLambert = saturate(NdotL * 0.5f + 0.5f);
    float toonThreshold = matCustomParams[0].y;
    float diffuse = (toonThreshold > 0.001f)
        ? smoothstep(toonThreshold - 0.04f, toonThreshold + 0.04f, halfLambert)
        : (halfLambert * halfLambert);

    // 2. Blinn-Phong Specular Highlight (modulated by roughness / metallic)
    float NdotH = saturate(dot(N, H));
    float roughnessFactor = clamp(matRoughness, 0.01f, 1.0f);
    float specPower = max(ambientColor.a * (1.0f / (roughnessFactor * roughnessFactor)), 1.0f);
    float specular = pow(NdotH, specPower) * (NdotL > 0.0f ? 1.0f : 0.0f);

    // 3. Multi-Mode Shadow Calculation & Self-Shadow Contrast Enhancement on 3D Models
    int cascadeIdx = 0;
    float shadowFactor = CalculateShadow(input, N, L, cascadeIdx);

    // Compute normalized shadow occlusion [0 = lit, 1 = fully shadowed]
    float shadowOcclusion = saturate((1.0f - shadowFactor) / max(1.0f - shadowParams.y, 0.001f));
    float3 shadowTint = lerp(float3(1.0f, 1.0f, 1.0f), matCustomParams[1].rgb, shadowOcclusion * matCustomParams[1].a);

    // Attenuate ambient in cast shadow regions according to selfShadowStrength so 3D models
    // exhibit clear, high-contrast self-shadowing (óéÇøâe) and model-to-model shadows
    float ambientShadowMult = lerp(1.0f, shadowParams.y, shadowOcclusion * shadowExtraParams.w);

    // 4. Directional Light contribution
    float3 lightContribution    = lightColor.rgb * (diffuse * lightDir.w * shadowFactor) * shadowTint;
    float3 ambientContribution  = ambientColor.rgb * ambientShadowMult * shadowTint;
    float3 specularContribution = lightColor.rgb * (specular * 0.35f * (1.0f - shadowOcclusion));
    float  rimFactor            = pow(1.0f - saturate(dot(N, V)), 3.0f) * matCustomParams[0].z;
    float3 rimContribution      = lightColor.rgb * rimFactor;

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
            float3 pL = toLight / dist;
            float3 pH = normalize(pL + V);

            float normDist    = dist / range;
            float attenWindow = saturate(1.0f - normDist * normDist);
            float smoothAtten = (attenWindow * attenWindow) / (dist * dist + 1.0f);

            float pNdotL  = saturate(dot(N, pL));
            float pDiffuse = pNdotL * (pIntensity * 10.0f) * smoothAtten;

            float pNdotH    = saturate(dot(N, pH));
            float pSpecular = pow(pNdotH, specPower) * (pNdotL > 0.0f ? 1.0f : 0.0f) * (pIntensity * 10.0f) * smoothAtten * 0.35f;

            pointLightContribution    += pColor * pDiffuse;
            pointSpecularContribution += pColor * pSpecular;
        }
    }

    float3 finalColor = texColor.rgb * (lightContribution + ambientContribution + pointLightContribution) + specularContribution + pointSpecularContribution + rimContribution;

    // Optional CSM Cascade Split Visualization Overlay (Red = Near, Green = Mid, Blue = Far)
    if (int(shadowParams.w + 0.5f) == 5 && lightCounts.z > 0.5f)
    {
        float3 cascadeColor = (cascadeIdx == 0) ? float3(1.0f, 0.25f, 0.25f)
                            : (cascadeIdx == 1) ? float3(0.25f, 1.0f, 0.25f)
                                                : float3(0.3f, 0.5f, 1.0f);
        finalColor = lerp(finalColor, finalColor * cascadeColor + cascadeColor * 0.18f, 0.45f);
    }

    // Mesh Highlight (Editor selection effect): matCustomParams[0].w is highlight intensity
    if (matCustomParams[0].w > 0.001f)
    {
        float3 highlightColor = float3(1.0f, 0.85f, 0.2f);
        finalColor = lerp(finalColor, highlightColor, matCustomParams[0].w);
    }

    return float4(finalColor, texColor.a);
}
