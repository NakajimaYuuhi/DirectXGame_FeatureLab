#include "ShaderManager.h"

std::shared_ptr<Shader> ShaderManager::GetShader(const std::wstring& filePath, const std::string& entryPoint, const std::string& target)
{
    std::wstring key = filePath + L":" + std::wstring(entryPoint.begin(), entryPoint.end());

    auto it = m_shaders.find(key);
    if (it != m_shaders.end())
    {
        return it->second;
    }

    std::shared_ptr<Shader> newShader = std::make_shared<Shader>();
    if (newShader->CompileFromFile(filePath, entryPoint, target))
    {
        m_shaders[key] = newShader;
        return newShader;
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
