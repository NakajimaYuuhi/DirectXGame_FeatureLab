#pragma once
#include <vector>
#include <memory>
#include "IRenderPass.h"
#include "RenderContext.h"

class RenderPipeline {
public:
    RenderPipeline() = default;
    ~RenderPipeline() = default;

    // Add pass to end of pipeline (optional device for immediate initialization)
    void AddPass(std::unique_ptr<IRenderPass> pass, ID3D12Device* pDevice = nullptr);

    // Initialize all registered passes
    void Init(ID3D12Device* pDevice);

    // Execute active passes in order
    void Execute(const RenderContext& ctx);

    // Move pass from one index to another (reorder)
    bool MovePass(size_t fromIndex, size_t toIndex);

    // Remove pass at specified index
    bool RemovePass(size_t index);

    // Clear passes
    void ClearPasses();

    // Accessors for passes
    size_t GetPassCount() const { return m_passes.size(); }
    const std::vector<std::unique_ptr<IRenderPass>>& GetPasses() const { return m_passes; }
    std::vector<std::unique_ptr<IRenderPass>>& GetPasses() { return m_passes; }

private:
    std::vector<std::unique_ptr<IRenderPass>> m_passes;
};
