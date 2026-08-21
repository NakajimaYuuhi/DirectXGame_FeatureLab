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

    bool CompileFromFile(const std::wstring& filePath, const std::string& entryPoint, const std::string& target);
    bool LoadFromCSO(const std::wstring& csoFilePath);

    ID3DBlob* GetBlob() const { return m_blob.Get(); }
    D3D12_SHADER_BYTECODE GetBytecode() const;

private:
    ComPtr<ID3DBlob> m_blob;
};
