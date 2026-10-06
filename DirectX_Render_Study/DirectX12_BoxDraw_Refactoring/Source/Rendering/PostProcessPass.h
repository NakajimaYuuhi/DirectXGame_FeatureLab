#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"
#include <wrl/client.h>
#include <memory>

class PostProcessPass : public IRenderPass {
public:
    PostProcessPass(RenderTexture* pSourceTex);
    virtual ~PostProcessPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return "PostProcess Pass (Bloom)"; }

    void SetSourceTexture(RenderTexture* pSourceTex) { m_pSourceTex = pSourceTex; }
    RenderTexture* GetSourceTexture() const { return m_pSourceTex; }

    RenderTexture* GetFinalTexture() const { return m_pFinalTex.get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetFinalSRV() const { return m_pFinalTex ? m_pFinalTex->GetSRV() : D3D12_GPU_DESCRIPTOR_HANDLE{}; }

private:
    RenderTexture* m_pSourceTex = nullptr;

    // ブルーム作業用バッファ (1/2 解像度)
    std::unique_ptr<RenderTexture> m_pBrightTex;
    std::unique_ptr<RenderTexture> m_pBlurTexTemp;

    // 最終合成結果出力用バッファ (フル解像度 / Viewport表示用)
    std::unique_ptr<RenderTexture> m_pFinalTex;

    // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;

    // パイプラインステート (PSO)
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPassThroughPSO;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pBrightPSO;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pBlurPSO;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pCompositePSO;
};