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
    if (!lightMgr.IsShadowEnabled()) return;

    ShadowMap* shadowMap = lightMgr.GetShadowMap();
    if (!shadowMap) return;

    // 0. Bind main SRV descriptor heap to command list
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

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

    // 2. Update Light Constant Buffer with current matrices and parameters
    lightMgr.UpdateBuffer(targetPos, cameraPos);

    // 3. Transition Shadow Map to DEPTH_WRITE & Clear
    shadowMap->TransitionToDepthWrite(ctx.cmdList);
    shadowMap->Clear(ctx.cmdList);

    // 4. Set Render Target (Depth Only) & Viewport
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = shadowMap->GetDSV();
    ctx.cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

    D3D12_VIEWPORT vp = shadowMap->GetViewport();
    D3D12_RECT scissor = shadowMap->GetScissorRect();
    ctx.cmdList->RSSetViewports(1, &vp);
    ctx.cmdList->RSSetScissorRects(1, &scissor);

    // 5. Set Shadow Pipeline State and Root Signature
    ID3D12PipelineState* shadowPso = PSOManager::GetInstance().GetShadowPSO();
    ID3D12RootSignature* shadowRootSig = PSOManager::GetInstance().GetShadowRootSignature();
    if (shadowPso && shadowRootSig)
    {
        ctx.cmdList->SetPipelineState(shadowPso);
        ctx.cmdList->SetGraphicsRootSignature(shadowRootSig);

        // 6. Draw Shadow Casters
        ObjectManager::GetInstance().DrawShadow(lightMgr.GetLightViewProjMatrix());
    }

    // 7. Transition Shadow Map to PIXEL_SHADER_RESOURCE for subsequent forward pass sampling
    shadowMap->TransitionToPixelShaderResource(ctx.cmdList);
}
