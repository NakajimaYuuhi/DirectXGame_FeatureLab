#pragma once

#include "IRenderPass.h"
#include "GPUParticleSystem.h"
#include "RenderTexture.h"
#include <memory>
#include <string>

class GPUParticlePass : public IRenderPass
{
public:
    GPUParticlePass(RenderTexture* pRenderTarget = nullptr);
    virtual ~GPUParticlePass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return m_customName.empty() ? "GPU Particle Pass" : m_customName; }

    void SetCustomName(const std::string& name) { m_customName = name; }
    const std::string& GetCustomName() const { return m_customName; }

    void SetRenderTarget(RenderTexture* pRenderTarget) { m_pRenderTarget = pRenderTarget; }
    RenderTexture* GetRenderTarget() const { return m_pRenderTarget; }

    void SetClearColorEnabled(bool enabled) { m_clearColorEnabled = enabled; }
    bool IsClearColorEnabled() const { return m_clearColorEnabled; }

    GPUParticleSystem& GetParticleSystem() { return m_particleSystem; }
    const GPUParticleSystem& GetParticleSystem() const { return m_particleSystem; }

private:
    RenderTexture* m_pRenderTarget = nullptr;
    std::string m_customName;
    bool m_clearColorEnabled = false;
    GPUParticleSystem m_particleSystem;
};
