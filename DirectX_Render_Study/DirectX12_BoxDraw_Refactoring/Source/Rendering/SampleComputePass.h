#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"
#include <wrl/client.h>
#include <memory>

class SampleComputePass : public IRenderPass {
public:
    SampleComputePass();
    virtual ~SampleComputePass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return "Compute Shader Pass"; }

    RenderTexture* GetOutputTexture() const { return m_pOutputTexture.get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetOutputSRV() const {
        return m_pOutputTexture ? m_pOutputTexture->GetSRV() : D3D12_GPU_DESCRIPTOR_HANDLE{};
    }

    bool IsPreviewEnabled() const { return m_previewInViewport; }
    void SetPreviewEnabled(bool enabled) { m_previewInViewport = enabled; }
    float GetSpeed() const { return m_speed; }
    void SetSpeed(float speed) { m_speed = speed; }

private:
    std::unique_ptr<RenderTexture> m_pOutputTexture;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPipelineState;

    float m_totalTime = 0.0f;
    float m_speed = 1.0f;
    bool m_previewInViewport = false;
};
