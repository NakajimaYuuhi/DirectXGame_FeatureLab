#include "ShadowMapPass.h"
#include "LightManager.h"
#include "ObjectManager.h"
#include "PSOManager.h"
#include "CameraComponent.h"
#include "Transform.h"
#include "DX12Manager.h"

void ShadowMapPass::Init(ID3D12Device* pDevice)
{
    LightManager::GetInstance().Init(pDevice);
}

void ShadowMapPass::Execute(const RenderContext& ctx)
{
    LightManager& lightMgr = LightManager::GetInstance();

    // 1. Determine target focus center (Player position or origin) and camera position
    DirectX::XMFLOAT3 targetPos = { 0.0f, 0.0f, 0.0f };
    CObject* player = ObjectManager::GetInstance().GetPlayer();
    if (player)
    {
        CTransform* t = player->GetComponent<CTransform>();
        if (t) targetPos = t->GetPos();
    }

    DirectX::XMFLOAT3 cameraPos = { 0.0f, 2.0f, -5.0f };
    CameraComponent* camera = ObjectManager::GetInstance().GetCamera();
    if (camera)
    {
        cameraPos = camera->GetEyePosition();
    }

    // 2. Always update Light Constant Buffer with current matrices and parameters
    lightMgr.UpdateBuffer(targetPos, cameraPos);

    if (!lightMgr.IsShadowEnabled())
    {
        for (int c = 0; c < 3; ++c)
        {
            if (ShadowMap* sm = lightMgr.GetShadowMap(c))
            {
                sm->TransitionToPixelShaderResource(ctx.cmdList);
            }
        }
        return;
    }

    // 3. Bind main SRV descriptor heap to command list
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    // 4. Set Shadow Pipeline State and Root Signature (supports custom shadow shader & rasterizer bias)
    std::wstring wShaderPath(m_shaderPath.begin(), m_shaderPath.end());
    ID3D12PipelineState* shadowPso = PSOManager::GetInstance().GetCustomShadowPSO(
        wShaderPath,
        m_vsEntry,
        m_depthBias,
        m_slopeScaledDepthBias
    );
    ID3D12RootSignature* shadowRootSig = PSOManager::GetInstance().GetShadowRootSignature();

    int cascadeCount = (lightMgr.GetShadowMode() == ShadowMode::CSM) ? 3 : 1;

    for (int c = 0; c < cascadeCount; ++c)
    {
        ShadowMap* shadowMap = lightMgr.GetShadowMap(c);
        if (!shadowMap) continue;

        shadowMap->TransitionToDepthWrite(ctx.cmdList);
        shadowMap->Clear(ctx.cmdList);

        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = shadowMap->GetDSV();
        ctx.cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

        D3D12_VIEWPORT vp = shadowMap->GetViewport();
        D3D12_RECT scissor = shadowMap->GetScissorRect();
        ctx.cmdList->RSSetViewports(1, &vp);
        ctx.cmdList->RSSetScissorRects(1, &scissor);

        if (shadowPso && shadowRootSig)
        {
            ctx.cmdList->SetPipelineState(shadowPso);
            ctx.cmdList->SetGraphicsRootSignature(shadowRootSig);
            ObjectManager::GetInstance().DrawShadow(lightMgr.GetLightViewProjMatrix(c));
        }

        shadowMap->TransitionToPixelShaderResource(ctx.cmdList);
    }

    // Ensure any inactive cascade shadow maps are in PIXEL_SHADER_RESOURCE state for the t2..t4 SRV table
    for (int c = cascadeCount; c < 3; ++c)
    {
        if (ShadowMap* sm = lightMgr.GetShadowMap(c))
        {
            sm->TransitionToPixelShaderResource(ctx.cmdList);
        }
    }
}
