#pragma once

#include "IRenderPass.h"
#include "GPUParticleSystem.h"
#include "RenderTexture.h"
#include <memory>

class GPUParticlePass : public IRenderPass
{
public:
    GPUParticlePass(RenderTexture* pRenderTarget = nullptr);
    virtual ~GPUParticlePass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return "GPU Particle Pass"; }

    void SetRenderTarget(RenderTexture* pRenderTarget) { m_pRenderTarget = pRenderTarget; }
    RenderTexture* GetRenderTarget() const { return m_pRenderTarget; }

    GPUParticleSystem& GetParticleSystem() { return m_particleSystem; }
    const GPUParticleSystem& GetParticleSystem() const { return m_particleSystem; }

private:
    RenderTexture* m_pRenderTarget = nullptr;
    GPUParticleSystem m_particleSystem;
};
