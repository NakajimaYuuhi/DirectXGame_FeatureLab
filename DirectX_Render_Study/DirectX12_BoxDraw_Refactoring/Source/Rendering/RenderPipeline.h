#pragma once
#include <vector>
#include <memory>
#include <string>
#include "IRenderPass.h"
#include "RenderContext.h"
#include "RenderBlackboard.h"

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

    // Build the 10-step commercial multi-layer game pipeline
    void SetupCommercialMultiLayerPipeline(ID3D12Device* pDevice, RenderTexture* pDefaultSceneTex);

    // Build the simple 4-pass legacy default pipeline
    void SetupSimpleDefaultPipeline(ID3D12Device* pDevice, RenderTexture* pDefaultSceneTex);

    // Save / Load pipeline configuration to/from JSON file
    bool SaveToJson(const std::string& filePath) const;
    bool LoadFromJson(const std::string& filePath, ID3D12Device* pDevice, RenderTexture* pDefaultSceneTex);

    // Accessors for passes
    size_t GetPassCount() const { return m_passes.size(); }
    const std::vector<std::unique_ptr<IRenderPass>>& GetPasses() const { return m_passes; }
    std::vector<std::unique_ptr<IRenderPass>>& GetPasses() { return m_passes; }

    // Blackboard for shared textures
    RenderBlackboard& GetBlackboard() { return m_blackboard; }
    const RenderBlackboard& GetBlackboard() const { return m_blackboard; }

    // Pin a specific Blackboard texture to the Viewport for debugging/inspection
    void SetPreviewTextureName(const std::string& name) { m_previewTextureName = name; }
    const std::string& GetPreviewTextureName() const { return m_previewTextureName; }

private:
    std::vector<std::unique_ptr<IRenderPass>> m_passes;
    RenderBlackboard m_blackboard;
    std::string m_previewTextureName;
};
