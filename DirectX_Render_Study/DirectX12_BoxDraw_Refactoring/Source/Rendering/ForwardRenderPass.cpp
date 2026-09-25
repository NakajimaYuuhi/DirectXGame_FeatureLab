#include "ForwardRenderPass.h"
#include "ObjectManager.h" // ObjectManager??g????
ForwardRenderPass::ForwardRenderPass(RenderTexture* pDestTex)
    : m_pDestTex(pDestTex)
{
}

void ForwardRenderPass::Init(ID3D12Device* pDevice)
{
}

void ForwardRenderPass::Execute(const RenderContext& ctx)
{
    // 1. ?`?????????
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = ctx.backBufferRTV; // ?f?t?H???g????o??

    // ?I?t?X?N???[???p?e?N?X?`?????w???????????????o?????
    if (m_pDestTex) {
        m_pDestTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        rtvHandle = m_pDestTex->GetRTV();
    }

    // 2. ?`????[?x?o?b?t?@??Z?b?g
    ctx.cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &ctx.mainDSV);

    // 3. ???i?????e?N?X?`???j??N???A
    const float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f }; // ???????w?i?F
    ctx.cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    ctx.cmdList->ClearDepthStencilView(ctx.mainDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 4. ?r???[?|?[?g??V?U?[??`????
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 5. ?I?u?W?F?N?g??`??
    ObjectManager::GetInstance().Draw(ctx.sceneID);
}

std::string ForwardRenderPass::GetName() const
{
    return "Forward Render Pass";
}