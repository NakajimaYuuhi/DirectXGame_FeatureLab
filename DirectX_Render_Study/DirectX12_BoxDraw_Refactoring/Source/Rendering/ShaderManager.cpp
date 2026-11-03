#include "ShaderManager.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <windows.h>

namespace fs = std::filesystem;

std::shared_ptr<Shader> ShaderManager::GetShader(const std::wstring& filePath, const std::string& entryPoint, const std::string& target)
{
    std::wstring key = filePath + L":" + std::wstring(entryPoint.begin(), entryPoint.end());

    auto it = m_shaders.find(key);
    if (it != m_shaders.end())
    {
        return it->second;
    }

    std::shared_ptr<Shader> newShader = std::make_shared<Shader>();
    std::string err;
    if (newShader->CompileFromFile(filePath, entryPoint, target, &err))
    {
        m_shaders[key] = newShader;
        return newShader;
    }
    else
    {
        m_lastErrorLog = err;
    }

    return nullptr;
}

std::shared_ptr<Shader> ShaderManager::GetShaderFromCSO(const std::wstring& csoFilePath)
{
    auto it = m_shaders.find(csoFilePath);
    if (it != m_shaders.end())
    {
        return it->second;
    }

    std::shared_ptr<Shader> newShader = std::make_shared<Shader>();
    if (newShader->LoadFromCSO(csoFilePath))
    {
        m_shaders[csoFilePath] = newShader;
        return newShader;
    }

    return nullptr;
}

bool ShaderManager::ReloadAllShaders(std::string* outErrorLog)
{
    bool allSucceeded = true;
    std::string combinedErrors;
    for (auto& kv : m_shaders)
    {
        if (kv.second && kv.second->IsCompiledFromFile())
        {
            std::string err;
            if (!kv.second->Recompile(&err))
            {
                allSucceeded = false;
                combinedErrors += err + "\n";
            }
        }
    }

    if (allSucceeded)
    {
        m_lastErrorLog.clear();
    }
    else
    {
        m_lastErrorLog = combinedErrors;
        if (outErrorLog) *outErrorLog = combinedErrors;
    }
    return allSucceeded;
}

std::vector<std::string> ShaderManager::GetAvailableShaderFiles() const
{
    std::vector<std::string> result;
    std::error_code ec;
    if (fs::exists("Assets/Shader", ec))
    {
        for (const auto& entry : fs::recursive_directory_iterator("Assets/Shader", ec))
        {
            if (entry.is_regular_file(ec))
            {
                std::string ext = entry.path().extension().string();
                if (ext == ".hlsl")
                {
                    std::string p = entry.path().generic_string();
                    result.push_back(p);
                }
            }
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

bool ShaderManager::CreateShaderFromTemplate(const std::string& filePath, ShaderTemplateType templateType, std::string* outError)
{
    std::error_code ec;
    fs::path p(filePath);
    if (p.has_parent_path())
    {
        fs::create_directories(p.parent_path(), ec);
    }

    if (fs::exists(p, ec))
    {
        if (outError) *outError = "File already exists: " + filePath;
        return false;
    }

    std::ofstream ofs(filePath);
    if (!ofs.is_open())
    {
        if (outError) *outError = "Failed to open file for writing: " + filePath;
        return false;
    }

    if (templateType == ShaderTemplateType::LitShadowMesh)
    {
        ofs <<
R"(// ==============================================================
// Custom Lit & Shadow Mesh Shader (.hlsl)
// Edit in VS Code and press Ctrl+S to auto hot-reload!
// Use @Param(index, "Label", min, max) to expose sliders in Inspector
// ==============================================================
// @Param(0, "Shadow PCF Spread", 0.0, 4.0)
// @Param(1, "Toon Step Threshold", 0.0, 1.0)
// @Param(2, "Rim Light Intensity", 0.0, 3.0)
// @Color(1, "Shadow Tint Color")

cbuffer ObjectBuffer : register(b0)
{
    float4x4 WVP;
    float4x4 World;
    float2   uvOffset;
    float2   uvScale;
};

struct PointLightData
{
    float4 position;
    float4 color;
};

cbuffer LightBuffer : register(b1)
{
    float4x4 LightViewProj;
    float4   cameraPos;
    float4   lightDir;
    float4   lightColor;
    float4   ambientColor;
    float4   shadowParams; // x: bias, y: darkness, z: mapSize, w: enabled
    float4   lightCounts;
    float4   reserved[6];
    PointLightData pointLights[8];
};

cbuffer MaterialBuffer : register(b2)
{
    float4   matBaseColor;
    float2   matUvTiling;
    float2   matUvOffset;
    float    matRoughness;
    float    matMetallic;
    float2   matPadding;
    float4   matCustomParams[2]; // [0].xyz = @Param(0..2), [0].w = editor highlight, [1] = @Color(1)
};

Texture2D tex0 : register(t0);
StructuredBuffer<float4x4> g_BoneMatrices : register(t1);
Texture2D shadowMap : register(t2);

SamplerState samLinear : register(s0);
SamplerState samShadow : register(s1);

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
    float4 shadowPos   : TEXCOORD1;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 localPos = float4(input.position, 1.0f);
    float3 localNormal = input.normal;

    float totalWeight = input.boneWeights.x + input.boneWeights.y + input.boneWeights.z + input.boneWeights.w;
    if (totalWeight > 0.001f)
    {
        localPos =
            mul(localPos, g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
            mul(localPos, g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
            mul(localPos, g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
            mul(localPos, g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;
        localNormal =
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
            mul(localNormal, (float3x3)g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;
    }

    output.position = mul(localPos, WVP);
    output.worldPos = mul(localPos, World).xyz;
    output.worldNormal = normalize(mul(float4(localNormal, 0.0f), World).xyz);
    output.shadowPos = mul(float4(output.worldPos, 1.0f), LightViewProj);
    output.uv = (input.uv * uvScale * matUvTiling) + uvOffset + matUvOffset;
    return output;
}

float CalculateCustomShadow(float4 shadowPos, float3 N, float3 L)
{
    if (shadowParams.w < 0.5f) return 1.0f;

    float3 projCoords = shadowPos.xyz / shadowPos.w;
    float2 uv = float2(projCoords.x * 0.5f + 0.5f, -projCoords.y * 0.5f + 0.5f);
    float currentDepth = projCoords.z;

    if (uv.x < 0.001f || uv.x > 0.999f || uv.y < 0.001f || uv.y > 0.999f || currentDepth > 1.0f || currentDepth < 0.0f)
        return 1.0f;

    float cosTheta = saturate(dot(N, L));
    float bias = max(shadowParams.x * (1.0f - cosTheta), shadowParams.x * 0.25f);
    float spread = max(0.25f, 1.0f + matCustomParams[0].x);
    float texelSize = (1.0f / shadowParams.z) * spread;

    float shadow = 0.0f;
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = shadowMap.Sample(samShadow, uv + float2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? shadowParams.y : 1.0f;
        }
    }
    return shadow / 9.0f;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = tex0.Sample(samLinear, input.uv) * matBaseColor;
    if (texColor.a < 0.05f) discard;

    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-lightDir.xyz);
    float3 V = normalize(cameraPos.xyz - input.worldPos);

    float halfLambert = saturate(dot(N, L) * 0.5f + 0.5f);
    float toonStep = matCustomParams[0].y;
    float diffuse = (toonStep > 0.01f) ? smoothstep(toonStep - 0.05f, toonStep + 0.05f, halfLambert) : (halfLambert * halfLambert);

    float shadowFactor = CalculateCustomShadow(input.shadowPos, N, L);
    float3 shadowTint = lerp(float3(1.0f, 1.0f, 1.0f), matCustomParams[1].rgb, (1.0f - shadowFactor) * matCustomParams[1].a);

    float rim = pow(1.0f - saturate(dot(N, V)), 3.0f) * matCustomParams[0].z;
    float3 finalColor = texColor.rgb * (lightColor.rgb * diffuse * lightDir.w * shadowFactor * shadowTint + ambientColor.rgb) + rim * lightColor.rgb;

    if (matCustomParams[0].w > 0.001f)
    {
        finalColor = lerp(finalColor, float3(1.0f, 0.85f, 0.2f), matCustomParams[0].w);
    }
    return float4(finalColor, texColor.a);
}
)";
    }
    else if (templateType == ShaderTemplateType::ShadowDepth)
    {
        ofs <<
R"(// ==============================================================
// Custom Shadow Map Depth Pass Shader (.hlsl)
// Attach this shader to ShadowMapPass in Render Pipeline Inspector
// ==============================================================

cbuffer ShadowTransformBuffer : register(b0)
{
    float4x4 LightWVP;
};

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

    if (totalWeight > 0.001f)
    {
        localPos =
            mul(localPos, g_BoneMatrices[input.boneIndices.x]) * input.boneWeights.x +
            mul(localPos, g_BoneMatrices[input.boneIndices.y]) * input.boneWeights.y +
            mul(localPos, g_BoneMatrices[input.boneIndices.z]) * input.boneWeights.z +
            mul(localPos, g_BoneMatrices[input.boneIndices.w]) * input.boneWeights.w;
    }

    output.position = mul(localPos, LightWVP);
    return output;
}
)";
    }
    else if (templateType == ShaderTemplateType::PostProcess)
    {
        ofs <<
R"(// ==============================================================
// Custom Post-Process Shader (.hlsl)
// Attach this shader to any PostProcessPass in Render Pipeline Inspector
// ==============================================================

cbuffer PostProcessBuffer : register(b0)
{
    float  g_Threshold;
    float  g_Knee;
    float  g_Intensity;
    float  g_Spread;
    float2 g_BlurDir;
    float  g_BloomEnabled;
    float  g_EffectType;
    float  g_ChromaticAberration;
    float  g_OutlineIntensity;
    float  g_OutlineThreshold;
    float  g_OutlineWidth;
    float  g_ScreenW;
    float  g_ScreenH;
    float  g_ToneMapType;
    float  g_Exposure;
    float3 g_OutlineColor;
    float  g_PreserveAlpha;
    float3 g_FadeColor;
    float  g_Fade;
};

Texture2D g_InputTex : register(t0);
Texture2D g_BloomTex : register(t1);
SamplerState g_LinearClamp : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

PSInput VSMain(uint vertexID : SV_VertexID)
{
    PSInput output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

float4 PSPassThrough(PSInput input) : SV_TARGET
{
    float4 col = g_InputTex.Sample(g_LinearClamp, input.uv);
    col.rgb = lerp(col.rgb, g_FadeColor, saturate(g_Fade));
    return col;
}

float4 PSBrightPass(PSInput input) : SV_TARGET
{
    float3 c = g_InputTex.Sample(g_LinearClamp, input.uv).rgb;
    float br = max(max(c.r, c.g), c.b);
    return float4(c * saturate(br - g_Threshold), 1.0f);
}

float4 PSBlurPass(PSInput input) : SV_TARGET
{
    return g_InputTex.Sample(g_LinearClamp, input.uv);
}

float4 PSComposite(PSInput input) : SV_TARGET
{
    float4 baseCol = g_InputTex.Sample(g_LinearClamp, input.uv);
    float3 bloom = g_BloomTex.Sample(g_LinearClamp, input.uv).rgb * g_Intensity;
    return float4(baseCol.rgb + bloom, baseCol.a);
}
)";
    }
    else if (templateType == ShaderTemplateType::Compute)
    {
        ofs <<
R"(// ==============================================================
// Custom Compute Shader (.hlsl)
// Attach this shader to SampleComputePass in Render Pipeline Inspector
// ==============================================================

cbuffer ComputeConstants : register(b0)
{
    float  g_Time;
    float3 g_Padding;
};

RWTexture2D<float4> g_OutputTex : register(u0);

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint width, height;
    g_OutputTex.GetDimensions(width, height);
    if (dispatchThreadID.x >= width || dispatchThreadID.y >= height) return;

    float2 uv = float2(dispatchThreadID.xy) / float2(width, height);
    float3 col = 0.5f + 0.5f * cos(g_Time + uv.xyx + float3(0.0f, 2.0f, 4.0f));
    g_OutputTex[dispatchThreadID.xy] = float4(col, 1.0f);
}
)";
    }

    ofs.close();
    return true;
}

bool ShaderManager::OpenInVSCode(const std::string& filePath, int line) const
{
    std::error_code ec;
    fs::path absPath = fs::absolute(filePath, ec);
    std::string targetPath = ec ? filePath : absPath.string();

    std::string cmd = "cmd.exe /C code -g \"" + targetPath + ":" + std::to_string((std::max)(1, line)) + "\"";

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi{};

    std::vector<char> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back('\0');

    if (CreateProcessA(nullptr, cmdBuf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    HINSTANCE res = ShellExecuteA(nullptr, "open", targetPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<intptr_t>(res) > 32;
}
