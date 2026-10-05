#include "ForwardRenderPass.h"
#include "ObjectManager.h"
#include "DX12Manager.h"

ForwardRenderPass::ForwardRenderPass(RenderTexture* pDestTex)
    : m_pDestTex(pDestTex)
{
}

void ForwardRenderPass::Init(ID3D12Device* pDevice)
{
}

void ForwardRenderPass::Execute(const RenderContext& ctx)
{
    // 0. Bind main SRV descriptor heap
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    // 1. Render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = ctx.backBufferRTV;

    if (m_pDestTex) {
        m_pDestTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        rtvHandle = m_pDestTex->GetRTV();
    }

    // 2. Set render target & depth stencil
    ctx.cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &ctx.mainDSV);

    // 3. Clear render target (use texture's optimized clear color if available to avoid #820 warning)
    const float defaultClearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };
    const float* clearColor = m_pDestTex ? m_pDestTex->GetClearColor() : defaultClearColor;
    ctx.cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    ctx.cmdList->ClearDepthStencilView(ctx.mainDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 4. Set viewport & scissor
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 5. Draw objects
    ObjectManager::GetInstance().Draw(ctx.sceneID);
}

std::string ForwardRenderPass::GetName() const
{
    return "Forward Render Pass";
}