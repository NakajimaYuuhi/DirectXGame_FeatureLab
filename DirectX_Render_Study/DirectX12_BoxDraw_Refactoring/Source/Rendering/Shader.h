#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

// Metadata parsed from HLSL comments (e.g. // @Param(0, "Shadow Softness", 0.0, 4.0))
// Maps directly to MaterialBuffer::matCustomParams[0..1] (indices 0..7)
struct ShaderParamAnnotation
{
    int paramIndex = 0;       // 0..7 (0..3 = matCustomParams[0].xyzw, 4..7 = matCustomParams[1].xyzw)
    std::string label;        // Display name in Inspector
    float minVal = 0.0f;
    float maxVal = 1.0f;
    bool isColor = false;     // If true, uses ColorEdit4 on matCustomParams[0] (idx=0) or [1] (idx=4)
};

// Reflection info extracted via D3DReflect
struct ReflectedVariable
{
    std::string name;
    UINT offset = 0;
    UINT size = 0;
};

struct ReflectedCBuffer
{
    std::string name;
    UINT bindPoint = 0;
    UINT size = 0;
    std::vector<ReflectedVariable> variables;
};

struct ShaderReflectionData
{
    bool isValid = false;
    UINT instructionCount = 0;
    std::vector<ReflectedCBuffer> constantBuffers;
    std::vector<std::string> boundResources;
};

class Shader
{
public:
    Shader() = default;
    ~Shader() = default;

    bool CompileFromFile(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, std::string* outError = nullptr);
    bool LoadFromCSO(const std::wstring& csoFilePath);

    // Recompile from the original file path/entryPoint/target; keeps old blob on failure
    bool Recompile(std::string* outError = nullptr);

    ID3DBlob* GetBlob() const { return m_blob.Get(); }
    D3D12_SHADER_BYTECODE GetBytecode() const;

    const std::wstring& GetFilePath() const { return m_filePath; }
    const std::string& GetEntryPoint() const { return m_entryPoint; }
    const std::string& GetTarget() const { return m_target; }
    bool IsCompiledFromFile() const { return !m_filePath.empty() && !m_entryPoint.empty(); }

    const ShaderReflectionData& GetReflection() const { return m_reflection; }
    const std::vector<ShaderParamAnnotation>& GetAnnotations() const { return m_annotations; }

private:
    void ExtractReflection();
    void ParseAnnotationsFromFile(const std::wstring& filePath);

    ComPtr<ID3DBlob> m_blob;
    std::wstring m_filePath;
    std::string m_entryPoint;
    std::string m_target;

    ShaderReflectionData m_reflection;
    std::vector<ShaderParamAnnotation> m_annotations;
};
