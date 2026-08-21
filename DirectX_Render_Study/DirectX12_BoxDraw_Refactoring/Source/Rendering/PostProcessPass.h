#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"

class PostProcessPass : public IRenderPass {
public:
    PostProcessPass(RenderTexture* pSourceTex);
    virtual ~PostProcessPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return "PostProcess Pass"; }

private:
    RenderTexture* m_pSourceTex = nullptr;
    
    ID3D12PipelineState* m_pPipelineState = nullptr;
    ID3D12RootSignature* m_pRootSignature = nullptr;
};
