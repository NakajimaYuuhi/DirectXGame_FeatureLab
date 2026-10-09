#pragma once
#include <vector>
#include <memory>
#include "IRenderPass.h"
#include "RenderContext.h"

class RenderPipeline {
public:
    RenderPipeline() = default;
    ~RenderPipeline() = default;

    // Add pass to end of pipeline
    void AddPass(std::unique_ptr<IRenderPass> pass);

    // Initialize all registered passes
    void Init(ID3D12Device* pDevice);

    // Execute active passes in order
    void Execute(const RenderContext& ctx);

    // Clear passes
    void ClearPasses();

    // Accessors for passes
    const std::vector<std::unique_ptr<IRenderPass>>& GetPasses() const { return m_passes; }
    std::vector<std::unique_ptr<IRenderPass>>& GetPasses() { return m_passes; }

private:
    std::vector<std::unique_ptr<IRenderPass>> m_passes;
};
