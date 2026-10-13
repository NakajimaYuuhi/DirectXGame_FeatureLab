#include "GPUParticlePass.h"
#include "DX12Manager.h"

GPUParticlePass::GPUParticlePass(RenderTexture* pRenderTarget)
    : m_pRenderTarget(pRenderTarget)
{
}

void GPUParticlePass::Init(ID3D12Device* pDevice)
{
    m_particleSystem.Init(pDevice);
}

void GPUParticlePass::Execute(const RenderContext& ctx)
{
    m_particleSystem.SetEnabled(m_isEnabled);
    if (!m_isEnabled) return;

    // 1. Simulation via Compute Shader (UAV dispatch)
    m_particleSystem.UpdateAndSimulate(ctx.cmdList, ctx.deltaTime);

    // 2. Set Render Target & Depth Stencil
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = ctx.backBufferRTV;
    if (m_pRenderTarget)
    {
        m_pRenderTarget->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        rtvHandle = m_pRenderTarget->GetRTV();
    }

    ctx.cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &ctx.mainDSV);

    // Set Viewport & Scissor
    D3D12_VIEWPORT viewport = {
        0.0f, 0.0f,
        static_cast<float>(ctx.screenWidth),
        static_cast<float>(ctx.screenHeight),
        0.0f, 1.0f
    };
    D3D12_RECT scissorRect = {
        0, 0,
        static_cast<LONG>(ctx.screenWidth),
        static_cast<LONG>(ctx.screenHeight)
    };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 3. Render Particles with Additive Blending
    DirectX::XMMATRIX view = DX12Manager::GetInstance().GetView();
    DirectX::XMMATRIX proj = DX12Manager::GetInstance().GetProj();

    m_particleSystem.Render(ctx.cmdList, view, proj);
}
