#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"
#include <wrl.h>
#include <string>

using Microsoft::WRL::ComPtr;

class DeferredLightingPass : public IRenderPass {
public:
    DeferredLightingPass(RenderTexture* pDestTex = nullptr);
    virtual ~DeferredLightingPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override;
    virtual bool ReloadShaders(ID3D12Device* pDevice, std::string* outError = nullptr) override;

    void SetCustomName(const std::string& name) { m_customName = name; }
    const std::string& GetCustomName() const { return m_customName; }

    void SetClearColorEnabled(bool enable) { m_clearColor = enable; }
    bool IsClearColorEnabled() const { return m_clearColor; }

private:
    void BuildRootSignatureAndPSO(ID3D12Device* pDevice);

private:
    RenderTexture* m_pDestTex = nullptr;
    bool m_clearColor = false;
    std::string m_customName;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;
};
