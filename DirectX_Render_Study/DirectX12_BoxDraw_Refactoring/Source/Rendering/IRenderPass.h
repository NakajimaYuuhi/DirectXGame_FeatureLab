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

    // Pass enable / disable flag
    bool IsEnabled() const { return m_isEnabled; }
    void SetEnabled(bool enabled) { m_isEnabled = enabled; }

protected:
    bool m_isEnabled = true;
};
