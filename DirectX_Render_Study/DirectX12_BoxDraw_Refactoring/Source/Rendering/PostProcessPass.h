#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"
#include <wrl/client.h>
#include <memory>
#include <string>

class PostProcessPass : public IRenderPass {
public:
    struct Settings {
        bool useCustomSettings = false;

        // Bloom
        bool bloomEnabled = false;
        float bloomThreshold = 0.8f;
        float bloomIntensity = 1.2f;
        float bloomSpread = 1.0f;

        // Outline (Sobel Edge)
        bool outlineEnabled = false;
        float outlineIntensity = 1.0f;
        float outlineThreshold = 0.1f;
        float outlineWidth = 1.5f;
        float outlineColor[3] = { 0.05f, 0.9f, 1.0f };
        bool preserveAlpha = false;

        // Chromatic Aberration
        bool chromaticAberrationEnabled = false;
        float chromaticAberrationIntensity = 0.005f;

        // Color Filter (0: None, 1: Grayscale, 2: Sepia, 3: Invert, 4: Vignette)
        int effectType = 0;

        // Tonemapping (0: None/Linear, 1: Reinhard, 2: ACES Filmic)
        int toneMapType = 0;
        float exposure = 1.0f;

        // Screen Fade (0.0 = normal, 1.0 = full fadeColor)
        float fade = 0.0f;
        float fadeColor[3] = { 0.0f, 0.0f, 0.0f };

        // Whether this pass copies to BackBuffer when Viewport is hidden
        bool outputToBackBuffer = true;
    };

    PostProcessPass(RenderTexture* pSourceTex = nullptr);
    virtual ~PostProcessPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override;

    void SetCustomName(const std::string& name) { m_customName = name; }
    const std::string& GetCustomName() const { return m_customName; }

    Settings& GetSettings() { return m_settings; }
    const Settings& GetSettings() const { return m_settings; }
    void SetSettings(const Settings& settings) { m_settings = settings; }

    void SetSourceTexture(RenderTexture* pSourceTex) { m_pSourceTex = pSourceTex; }
    RenderTexture* GetSourceTexture() const { return m_pSourceTex; }

    RenderTexture* GetFinalTexture() const { return m_pFinalTex.get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetFinalSRV() const { return m_pFinalTex ? m_pFinalTex->GetSRV() : D3D12_GPU_DESCRIPTOR_HANDLE{}; }

private:
    RenderTexture* m_pSourceTex = nullptr;
    std::string m_customName;
    Settings m_settings;

    // Half-resolution buffers for Bloom
    std::unique_ptr<RenderTexture> m_pBrightTex;
    std::unique_ptr<RenderTexture> m_pBlurTexTemp;

    // Full-resolution output buffer
    std::unique_ptr<RenderTexture> m_pFinalTex;

    // Root Signature
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;

    // Pipeline State Objects (PSOs)
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPassThroughPSO;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pBrightPSO;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pBlurPSO;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pCompositePSO;
};