// ==============================================================
// DeferredLighting.hlsl - Deferred Lighting Pass
// Evaluates Directional / Point lights & Shadows from GBuffer
// ==============================================================

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

// Fullscreen triangle vertex shader
VSOutput VSMain(uint vertexID : SV_VertexID)
{
    VSOutput output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

struct PointLightData
{
    float4 position; // xyz: pos, w: range
    float4 color;    // rgb: color, w: intensity
};

// 768-byte aligned Constant Buffer Data (Matches LightBufferData in C++)
cbuffer LightBuffer : register(b0)
{
    float4x4 LightViewProj;
    float4   cameraPos;
    float4   lightDir;
    float4   lightColor;
    float4   ambientColor;
    float4   shadowParams;        // x: bias, y: darkness, z: mapSize, w: shadowMode (0..6)
    float4   lightCounts;         // x: pointLightCount, y: blobCasterCount, z: visualizeCascades, w: projectiveGroundOnly
    float4   shadowExtraParams;   // x: normalBias, y: pcssLightSize, z: pcfKernelMode, w: selfShadowStrength
    float4   cascadeSplits;       // x: split0Dist, y: split1Dist, z: split2Dist, w: orthoSize
    float4   blobCasters[4];      // 16 floats

    PointLightData pointLights[8];

    float4x4 LightViewProjCascade1;
    float4x4 LightViewProjCascade2;
    float4   csmPadding[8];
};

// GBuffer inputs
Texture2D g_AlbedoTex   : register(t0);
Texture2D g_NormalTex   : register(t1);
Texture2D g_MaterialTex : register(t2);
Texture2D g_WorldPosTex : register(t3);

// Shadow Map inputs (Cascades 0..2)
Texture2D shadowMap0 : register(t4);
Texture2D shadowMap1 : register(t5);
Texture2D shadowMap2 : register(t6);

SamplerState g_Sampler : register(s0);
SamplerState samShadow : register(s1);

// Helper: Convert light clip-space position to UV and depth
bool ProjectToShadowUV(float4 clipPos, out float2 outUV, out float outDepth)
{
    float3 projCoords = clipPos.xyz / clipPos.w;
    outUV = float2(projCoords.x * 0.5f + 0.5f, -projCoords.y * 0.5f + 0.5f);
    outDepth = projCoords.z;
    return (outUV.x >= 0.002f && outUV.x <= 0.998f &&
            outUV.y >= 0.002f && outUV.y <= 0.998f &&
            outDepth >= 0.0f && outDepth <= 1.0f);
}

// Multi-cascade shadow evaluation
float CalculateShadow(float3 worldPos, float3 normal, float3 L)
{
    int mode = int(shadowParams.w + 0.5f);
    if (mode <= 0) return 1.0f; // Shadow disabled

    // Normal offset bias
    float normalBias = max(shadowExtraParams.x, 0.0005f);
    float3 biasedWorldPos = worldPos + normal * normalBias;

    int cascadeCount = (mode == 5) ? 3 : 1; // Mode 5 is CSM

    for (int c = 0; c < cascadeCount; ++c)
    {
        float4x4 vp = (c == 0) ? LightViewProj : ((c == 1) ? LightViewProjCascade1 : LightViewProjCascade2);
        float4 clipPos = mul(vp, float4(biasedWorldPos, 1.0f));

        float2 shadowUV;
        float shadowDepth;
        if (ProjectToShadowUV(clipPos, shadowUV, shadowDepth))
        {
            float bias = max(shadowParams.x * (1.0f - dot(normal, L)), 0.0002f);
            float currentDepth = shadowDepth - bias;

            float shadowSum = 0.0f;
            float2 texelSize = float2(1.0f / 2048.0f, 1.0f / 2048.0f);

            [unroll]
            for (int x = -1; x <= 1; ++x)
            {
                [unroll]
                for (int y = -1; y <= 1; ++y)
                {
                    float2 offset = float2(x, y) * texelSize;
                    float mapDepth = 1.0f;
                    if (c == 1) mapDepth = shadowMap1.SampleLevel(samShadow, shadowUV + offset, 0).r;
                    else if (c == 2) mapDepth = shadowMap2.SampleLevel(samShadow, shadowUV + offset, 0).r;
                    else mapDepth = shadowMap0.SampleLevel(samShadow, shadowUV + offset, 0).r;

                    shadowSum += (currentDepth > mapDepth) ? shadowParams.y : 1.0f;
                }
            }
            return shadowSum / 9.0f;
        }
    }

    return 1.0f;
}

float4 PSMain(VSOutput input) : SV_Target
{
    float4 posData = g_WorldPosTex.Sample(g_Sampler, input.uv);

    // If no geometry was drawn to this pixel, keep background clear
    if (posData.w < 0.5f)
    {
        discard;
    }

    float4 albedoData = g_AlbedoTex.Sample(g_Sampler, input.uv);
    float4 normalData = g_NormalTex.Sample(g_Sampler, input.uv);
    float4 matData    = g_MaterialTex.Sample(g_Sampler, input.uv);

    float3 albedo    = albedoData.rgb;
    float3 normal    = normalize(normalData.xyz);
    float  roughness = normalData.w;
    float  metallic  = matData.r;
    float  ao        = matData.g;
    float3 worldPos  = posData.xyz;

    float3 N = normal;
    float3 L = normalize(-lightDir.xyz);
    float3 V = normalize(cameraPos.xyz - worldPos);

    // 1. Directional Light (Lambert)
    float NdotL = max(dot(N, L), 0.0f);

    // 2. Shadow Factor
    float shadowFactor = CalculateShadow(worldPos, N, L);

    // 3. Specular (Blinn-Phong / PBR Roughness-based)
    float3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0f);
    float specPower = lerp(8.0f, 128.0f, (1.0f - roughness));
    float specTerm = pow(NdotH, specPower) * (1.0f - roughness);
    float3 specular = specTerm * lightColor.rgb * lerp(0.04f, albedo, metallic);

    // 4. Directional contribution
    float3 diffuse = albedo * lightColor.rgb * NdotL * shadowFactor;
    float3 ambient = albedo * ambientColor.rgb * ao;

    // 5. Point Lights (up to 8)
    float3 pointLightSum = float3(0.0f, 0.0f, 0.0f);
    int numPointLights = min(int(lightCounts.x + 0.5f), 8);
    for (int p = 0; p < numPointLights; ++p)
    {
        float3 pPos = pointLights[p].position.xyz;
        float  pRange = max(pointLights[p].position.w, 0.001f);
        float3 pCol = pointLights[p].color.rgb * pointLights[p].color.w;

        float3 pToLight = pPos - worldPos;
        float  pDist = length(pToLight);
        if (pDist < pRange)
        {
            float3 pL = pToLight / pDist;
            float  pNdotL = max(dot(N, pL), 0.0f);
            float  att = saturate(1.0f - (pDist / pRange));
            att *= att;

            pointLightSum += albedo * pCol * pNdotL * att;
        }
    }

    float3 finalColor = ambient + diffuse + specular * shadowFactor + pointLightSum;

    return float4(finalColor, 1.0f);
}
