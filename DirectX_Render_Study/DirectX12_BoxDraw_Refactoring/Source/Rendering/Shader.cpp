#include "Shader.h"
#include <d3dcompiler.h>
#include <fstream>
#include <vector>

bool Shader::CompileFromFile(const std::wstring& filePath, const std::string& entryPoint, const std::string& target)
{
    ComPtr<ID3DBlob> errorBlob;
    
    HRESULT hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(),
        target.c_str(),
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &m_blob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        return false;
    }
    return true;
}

bool Shader::LoadFromCSO(const std::wstring& csoFilePath)
{
    std::ifstream file(csoFilePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        return false;
    }

    HRESULT hr = D3DCreateBlob(size, &m_blob);
    if (FAILED(hr))
    {
        return false;
    }

    memcpy(m_blob->GetBufferPointer(), buffer.data(), size);
    return true;
}

D3D12_SHADER_BYTECODE Shader::GetBytecode() const
{
    D3D12_SHADER_BYTECODE bytecode = {};
    if (m_blob)
    {
        bytecode.pShaderBytecode = m_blob->GetBufferPointer();
        bytecode.BytecodeLength = m_blob->GetBufferSize();
    }
    return bytecode;
}
