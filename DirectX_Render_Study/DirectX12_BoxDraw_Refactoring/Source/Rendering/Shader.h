#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <string>

using Microsoft::WRL::ComPtr;

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

private:
    ComPtr<ID3DBlob> m_blob;
    std::wstring m_filePath;
    std::string m_entryPoint;
    std::string m_target;
};
