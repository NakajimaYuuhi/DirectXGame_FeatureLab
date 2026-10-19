#pragma once

#include "Shader.h"
#include <unordered_map>
#include <memory>
#include <string>

class ShaderManager
{
public:
    static ShaderManager& GetInstance()
    {
        static ShaderManager instance;
        return instance;
    }

    std::shared_ptr<Shader> GetShader(const std::wstring& filePath, const std::string& entryPoint, const std::string& target);
    std::shared_ptr<Shader> GetShaderFromCSO(const std::wstring& csoFilePath);

    // Recompile all cached HLSL shaders (returns false if any shader failed to compile)
    bool ReloadAllShaders(std::string* outErrorLog = nullptr);

    void Clear() { m_shaders.clear(); }

private:
    ShaderManager() = default;
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    std::unordered_map<std::wstring, std::shared_ptr<Shader>> m_shaders;
};
