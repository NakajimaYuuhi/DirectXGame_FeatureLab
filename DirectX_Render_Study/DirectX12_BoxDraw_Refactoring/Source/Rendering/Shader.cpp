#include "Shader.h"
#include <d3dcompiler.h>
#include <d3d12shader.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

bool Shader::CompileFromFile(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, std::string* outError)
{
    m_filePath = filePath;
    m_entryPoint = entryPoint;
    m_target = target;

    ComPtr<ID3DBlob> newBlob;
    ComPtr<ID3DBlob> errorBlob;
    
    HRESULT hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(),
        target.c_str(),
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &newBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        if (errorBlob && errorBlob->GetBufferPointer())
        {
            std::string errStr(static_cast<const char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
            OutputDebugStringA(errStr.c_str());
            if (outError) *outError += errStr;
        }
        else if (outError)
        {
            std::string pathStr(filePath.begin(), filePath.end());
            *outError += "Failed to compile shader: " + pathStr + " (" + entryPoint + ")\n";
        }
        return false;
    }

    m_blob = newBlob;
    ExtractReflection();
    ParseAnnotationsFromFile(filePath);
    return true;
}

bool Shader::Recompile(std::string* outError)
{
    if (!IsCompiledFromFile())
    {
        return true;
    }
    return CompileFromFile(m_filePath, m_entryPoint, m_target, outError);
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
    ExtractReflection();
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

void Shader::ExtractReflection()
{
    m_reflection = ShaderReflectionData{};
    if (!m_blob) return;

    ComPtr<ID3D12ShaderReflection> reflector;
    HRESULT hr = D3DReflect(m_blob->GetBufferPointer(), m_blob->GetBufferSize(), IID_PPV_ARGS(&reflector));
    if (FAILED(hr) || !reflector) return;

    D3D12_SHADER_DESC shaderDesc{};
    if (FAILED(reflector->GetDesc(&shaderDesc))) return;

    m_reflection.isValid = true;
    m_reflection.instructionCount = shaderDesc.InstructionCount;

    for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
    {
        ID3D12ShaderReflectionConstantBuffer* cb = reflector->GetConstantBufferByIndex(i);
        if (!cb) continue;

        D3D12_SHADER_BUFFER_DESC cbDesc{};
        if (FAILED(cb->GetDesc(&cbDesc))) continue;

        ReflectedCBuffer refCB;
        refCB.name = cbDesc.Name ? cbDesc.Name : "CBuffer";
        refCB.size = cbDesc.Size;
        refCB.bindPoint = i;

        for (UINT v = 0; v < cbDesc.Variables; ++v)
        {
            ID3D12ShaderReflectionVariable* var = cb->GetVariableByIndex(v);
            if (!var) continue;
            D3D12_SHADER_VARIABLE_DESC varDesc{};
            if (SUCCEEDED(var->GetDesc(&varDesc)))
            {
                ReflectedVariable rv;
                rv.name = varDesc.Name ? varDesc.Name : "var";
                rv.offset = varDesc.StartOffset;
                rv.size = varDesc.Size;
                refCB.variables.push_back(rv);
            }
        }
        m_reflection.constantBuffers.push_back(refCB);
    }

    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
        if (SUCCEEDED(reflector->GetResourceBindingDesc(i, &bindDesc)))
        {
            std::string resName = bindDesc.Name ? bindDesc.Name : "Resource";
            std::string regPrefix = "r";
            if (bindDesc.Type == D3D_SIT_CBUFFER)
            {
                regPrefix = "b";
                for (auto& cb : m_reflection.constantBuffers)
                {
                    if (cb.name == resName) cb.bindPoint = bindDesc.BindPoint;
                }
            }
            else if (bindDesc.Type == D3D_SIT_TEXTURE || bindDesc.Type == D3D_SIT_STRUCTURED)
            {
                regPrefix = "t";
            }
            else if (bindDesc.Type == D3D_SIT_UAV_RWTYPED || bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
            {
                regPrefix = "u";
            }
            else if (bindDesc.Type == D3D_SIT_SAMPLER)
            {
                regPrefix = "s";
            }
            m_reflection.boundResources.push_back(resName + " (" + regPrefix + std::to_string(bindDesc.BindPoint) + ")");
        }
    }
}

void Shader::ParseAnnotationsFromFile(const std::wstring& filePath)
{
    m_annotations.clear();
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return;

    std::string line;
    while (std::getline(ifs, line))
    {
        // Format 1: // @Param(index, "Label", min, max)
        auto posParam = line.find("@Param(");
        if (posParam != std::string::npos)
        {
            size_t start = posParam + 7;
            size_t end = line.find(')', start);
            if (end != std::string::npos)
            {
                std::string args = line.substr(start, end - start);
                size_t q1 = args.find('"');
                size_t q2 = (q1 != std::string::npos) ? args.find('"', q1 + 1) : std::string::npos;
                if (q1 != std::string::npos && q2 != std::string::npos)
                {
                    std::string idxPart = args.substr(0, q1);
                    std::string label = args.substr(q1 + 1, q2 - q1 - 1);
                    std::string rest = args.substr(q2 + 1);

                    idxPart.erase(std::remove(idxPart.begin(), idxPart.end(), ','), idxPart.end());
                    std::replace(rest.begin(), rest.end(), ',', ' ');

                    int idx = 0;
                    float minV = 0.0f, maxV = 1.0f;
                    try { idx = std::stoi(idxPart); } catch (...) {}
                    std::stringstream ss(rest);
                    ss >> minV >> maxV;

                    if (idx >= 0 && idx < 8)
                    {
                        ShaderParamAnnotation ann;
                        ann.paramIndex = idx;
                        ann.label = label;
                        ann.minVal = minV;
                        ann.maxVal = maxV;
                        ann.isColor = false;
                        m_annotations.push_back(ann);
                    }
                }
            }
        }

        // Format 2: // @Color(vecIndex, "Label")  where vecIndex is 0 or 1
        auto posColor = line.find("@Color(");
        if (posColor != std::string::npos)
        {
            size_t start = posColor + 7;
            size_t end = line.find(')', start);
            if (end != std::string::npos)
            {
                std::string args = line.substr(start, end - start);
                size_t q1 = args.find('"');
                size_t q2 = (q1 != std::string::npos) ? args.find('"', q1 + 1) : std::string::npos;
                if (q1 != std::string::npos && q2 != std::string::npos)
                {
                    std::string idxPart = args.substr(0, q1);
                    std::string label = args.substr(q1 + 1, q2 - q1 - 1);
                    idxPart.erase(std::remove(idxPart.begin(), idxPart.end(), ','), idxPart.end());
                    int vecIdx = 0;
                    try { vecIdx = std::stoi(idxPart); } catch (...) {}
                    if (vecIdx == 0 || vecIdx == 1)
                    {
                        ShaderParamAnnotation ann;
                        ann.paramIndex = vecIdx * 4;
                        ann.label = label;
                        ann.isColor = true;
                        m_annotations.push_back(ann);
                    }
                }
            }
        }
    }
}
