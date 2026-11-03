#pragma once

#include "Shader.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

enum class ShaderTemplateType
{
    LitShadowMesh = 0, // Standard forward mesh shader with ShadowMap & @Param annotations
    ShadowDepth,       // Depth-only shadow caster shader
    PostProcess,       // Fullscreen Post-Process shader
    Compute            // RWTexture2D Compute shader
};

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

    // Scan Assets/Shader for all .hlsl files
    std::vector<std::string> GetAvailableShaderFiles() const;

    // Create a new .hlsl file from a built-in template
    bool CreateShaderFromTemplate(const std::string& filePath, ShaderTemplateType templateType, std::string* outError = nullptr);

    // Launch VS Code (or OS default editor) at the specified file and line
    bool OpenInVSCode(const std::string& filePath, int line = 1) const;

    // Last compilation error log for UI display
    const std::string& GetLastErrorLog() const { return m_lastErrorLog; }
    void SetLastErrorLog(const std::string& err) { m_lastErrorLog = err; }
    void ClearLastErrorLog() { m_lastErrorLog.clear(); }

    const std::unordered_map<std::wstring, std::shared_ptr<Shader>>& GetLoadedShaders() const { return m_shaders; }

    void Clear() { m_shaders.clear(); }

private:
    ShaderManager() = default;
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    std::unordered_map<std::wstring, std::shared_ptr<Shader>> m_shaders;
    std::string m_lastErrorLog;
};
