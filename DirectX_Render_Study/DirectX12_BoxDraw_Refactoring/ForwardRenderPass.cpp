#include "ForwardRenderPass.h"
#include "ObjectManager.h"

ForwardRenderPass::ForwardRenderPass()
{
}

void ForwardRenderPass::Init(ID3D12Device* pDevice)
{
    // パス固有の初期化（PSOの取得など）があればここで行います。
}

void ForwardRenderPass::Execute(const RenderContext& ctx)
{
    // 1. 描画先と深度バッファをセットする
    ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, &ctx.mainDSV);

    // 2. 画面のクリア (ここでは青っぽい色にしています)
    const float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(ctx.backBufferRTV, clearColor, 0, nullptr);
    ctx.cmdList->ClearDepthStencilView(ctx.mainDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 3. ビューポートとシザー矩形の設定
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 4. ★既存の描画ロジックの呼び出し★
    // ここで ObjectManager::Draw() などを呼び出します！
    // (現在はまだ Context に ObjectManager を渡していないため、後ほど繋ぎ込みます)
    ObjectManager::GetInstance().Draw(ctx.sceneID);
}

std::string ForwardRenderPass::GetName() const
{
    return "Forward Render Pass";
}