#include "ForwardRenderPass.h"
#include "ObjectManager.h" // ObjectManagerを使う場合
ForwardRenderPass::ForwardRenderPass(RenderTexture* pDestTex)
    : m_pDestTex(pDestTex)
{
}

void ForwardRenderPass::Init(ID3D12Device* pDevice)
{
}

void ForwardRenderPass::Execute(const RenderContext& ctx)
{
    // 1. 描画先を決定する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = ctx.backBufferRTV; // デフォルトは画面出力

    // オフスクリーン用テクスチャが指定されていればそちらに出力する
    if (m_pDestTex) {
        m_pDestTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        rtvHandle = m_pDestTex->GetRTV();
    }

    // 2. 描画先と深度バッファをセット
    ctx.cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &ctx.mainDSV);

    // 3. 画面（またはテクスチャ）のクリア
    const float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f }; // 青っぽい背景色
    ctx.cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    ctx.cmdList->ClearDepthStencilView(ctx.mainDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 4. ビューポートとシザー矩形の設定
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 5. オブジェクトの描画
    ObjectManager::GetInstance().Draw(ctx.sceneID);
}

std::string ForwardRenderPass::GetName() const
{
    return "Forward Render Pass";
}