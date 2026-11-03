#pragma once
#include <d3d12.h>
#include <string>
#include "RenderContext.h"

class IRenderPass {
public:
    virtual ~IRenderPass() = default;

    // Initialize pass (PSO creation, resources)
    virtual void Init(ID3D12Device* pDevice) = 0;

    // Record commands to command list
    virtual void Execute(const RenderContext& ctx) = 0;

    // Pass name for debugging and profiling
    virtual std::string GetName() const = 0;

    // Recompile pass-specific shaders / PSOs on hot-reload (optional override)
    virtual bool ReloadShaders(ID3D12Device* pDevice, std::string* outError = nullptr) { return true; }

    // Pass enable / disable flag
    bool IsEnabled() const { return m_isEnabled; }
    void SetEnabled(bool enabled) { m_isEnabled = enabled; }

    // Resource Binding (Input / Output Texture Names in Blackboard)
    const std::string& GetInputResourceName() const { return m_inputResourceName; }
    void SetInputResourceName(const std::string& name) { m_inputResourceName = name; }

    const std::string& GetOutputResourceName() const { return m_outputResourceName; }
    void SetOutputResourceName(const std::string& name) { m_outputResourceName = name; }

protected:
    bool m_isEnabled = true;
    std::string m_inputResourceName;
    std::string m_outputResourceName;
};
