#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"

class ForwardRenderPass : public IRenderPass {
public:
    ForwardRenderPass(RenderTexture* pDestTex = nullptr);
    virtual ~ForwardRenderPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override;

    // Layer Filter (-1: All Layers, 0: Opaque, 1: Transparent, 2: Debug, 3: UI)
    int GetLayerFilter() const { return m_layerFilter; }
    void SetLayerFilter(int filter) { m_layerFilter = filter; }

    // Clear render target at beginning of pass
    bool IsClearEnabled() const { return m_clearEnabled; }
    void SetClearEnabled(bool clear) { m_clearEnabled = clear; }

private:
    RenderTexture* m_pDestTex = nullptr;
    int m_layerFilter = -1;
    bool m_clearEnabled = true;
};
