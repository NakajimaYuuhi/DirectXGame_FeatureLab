#include "ForwardRenderPass.h"
#include "ObjectManager.h"
#include "DX12Manager.h"
#include "ViewportUI.h"
#include "RenderBlackboard.h"
#include "RenderLayer.h"

ForwardRenderPass::ForwardRenderPass(RenderTexture* pDestTex)
    : m_pDestTex(pDestTex)
{
    m_outputResourceName = "SceneColor";
}

void ForwardRenderPass::Init(ID3D12Device* pDevice)
{
}

void ForwardRenderPass::SetFilterPreset(ObjectFilterPreset preset)
{
    m_filterPreset = preset;
    switch (preset)
    {
    case ObjectFilterPreset::All:
        m_tagMask = 0xFFFFFFFFu;
        break;
    case ObjectFilterPreset::GroundAndBackground:
        m_tagMask = (1u << ObjectTag::BACKGROUND) |
                    (1u << ObjectTag::FIELD) |
                    (1u << ObjectTag::TRIANGLE) |
                    (1u << ObjectTag::BILLBOARD);
        break;
    case ObjectFilterPreset::Characters:
        m_tagMask = (1u << ObjectTag::PLAYER) |
                    (1u << ObjectTag::ENEMY) |
                    (1u << ObjectTag::PLAYER_BULLET) |
                    (1u << ObjectTag::ENEMY_BULLET);
        break;
    case ObjectFilterPreset::UIOnly:
        m_tagMask = (1u << ObjectTag::UI) |
                    (1u << ObjectTag::TEXT) |
                    (1u << ObjectTag::MANAGER);
        break;
    case ObjectFilterPreset::Custom:
    default:
        break;
    }
}

void ForwardRenderPass::Execute(const RenderContext& ctx)
{
    // 0. Bind main SRV descriptor heap
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    // Resolve destination texture from blackboard (fallback to m_pDestTex)
    RenderTexture* pDest = nullptr;
    if (ctx.blackboard && !m_outputResourceName.empty()) {
        pDest = ctx.blackboard->GetTexture(m_outputResourceName);
    }
    if (!pDest) {
        pDest = m_pDestTex;
    }

    // 1. Render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = ctx.backBufferRTV;

    if (pDest) {
        pDest->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        rtvHandle = pDest->GetRTV();
    }

    // 2. Set render target & depth stencil
    ctx.cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &ctx.mainDSV);

    // 3. Clear render target & depth stencil (independently controlled)
    if (m_clearColorEnabled) {
        const float defaultClearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };
        const float* clearColor = pDest ? pDest->GetClearColor() : defaultClearColor;
        ctx.cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    }
    if (m_clearDepthEnabled) {
        ctx.cmdList->ClearDepthStencilView(ctx.mainDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    // 4. Set viewport & scissor
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 5. Draw objects (filtered by tagMask and layerFilter)
    if (m_tagMask == 0xFFFFFFFFu && m_layerFilter == -1) {
        ObjectManager::GetInstance().Draw(ctx.sceneID);
    } else {
        ObjectManager::GetInstance().DrawFiltered(m_tagMask, m_layerFilter);
    }

    // 6. Update Viewport preview if drawing to offscreen texture
    if (pDest) {
        CViewportUI::GetInstance().SetTextureSRV(pDest->GetSRV());
    }
}

std::string ForwardRenderPass::GetName() const
{
    if (!m_customName.empty()) {
        return m_customName;
    }

    std::string suffix;
    switch (m_filterPreset)
    {
    case ObjectFilterPreset::GroundAndBackground: suffix = "Ground & BG"; break;
    case ObjectFilterPreset::Characters:          suffix = "Characters"; break;
    case ObjectFilterPreset::UIOnly:              suffix = "UI Objects"; break;
    case ObjectFilterPreset::Custom:              suffix = "Custom Tags"; break;
    default:                                      break;
    }

    if (m_layerFilter >= 0 && m_layerFilter < static_cast<int>(RenderLayer::COUNT)) {
        std::string layerStr = GetRenderLayerName(static_cast<RenderLayer>(m_layerFilter));
        if (!suffix.empty()) {
            return "Forward Pass [" + suffix + " / " + layerStr + "]";
        }
        return "Forward Pass [" + layerStr + "]";
    }

    if (!suffix.empty()) {
        return "Forward Pass [" + suffix + "]";
    }
    return "Forward Render Pass";
}
