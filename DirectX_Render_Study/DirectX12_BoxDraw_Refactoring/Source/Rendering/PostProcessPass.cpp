#include "PostProcessPass.h"
#include <d3dcompiler.h>
#include <stdexcept>
#include "d3dx12.h"
#include "DX12Manager.h"
#include "LightManager.h"
#include "BasicSettings.h"
#include "ViewportUI.h"

#pragma comment(lib, "d3dcompiler.lib")

PostProcessPass::PostProcessPass(RenderTexture* pSourceTex)
    : m_pSourceTex(pSourceTex)
{
}

void PostProcessPass::Init(ID3D12Device* pDevice)
{
    // --------------------------------------------------------
    // 1. ルートシグネチャ作成
    // Param 0: 32-bit Constants (8 DWORD = 32 bytes, register b0)
    // Param 1: Descriptor Table (1 SRV, register t0) - Input / Main
    // Param 2: Descriptor Table (1 SRV, register t1) - Bloom blur
    // Sampler 0: Static Sampler (register s0) - Linear Clamp
    // --------------------------------------------------------
    CD3DX12_ROOT_PARAMETER1 rootParams[3];
    
    // Param 0: Root Constants
    rootParams[0].InitAsConstants(8, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    // Param 1: Descriptor Table t0
    CD3DX12_DESCRIPTOR_RANGE1 rangeT0;
    rangeT0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
    rootParams[1].InitAsDescriptorTable(1, &rangeT0, D3D12_SHADER_VISIBILITY_PIXEL);

    // Param 2: Descriptor Table t1
    CD3DX12_DESCRIPTOR_RANGE1 rangeT1;
    rangeT1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
    rootParams[2].InitAsDescriptorTable(1, &rangeT1, D3D12_SHADER_VISIBILITY_PIXEL);

    // Static Sampler (s0: Linear Clamp)
    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(3, rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_1,
        &signatureBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        throw std::runtime_error("Failed to serialize PostProcess RootSignature");
    }

    hr = pDevice->CreateRootSignature(
        0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&m_pRootSignature)
    );
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create PostProcess RootSignature");
    }

    // --------------------------------------------------------
    // 2. シェーダーコンパイル
    // --------------------------------------------------------
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psPassThroughBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBrightBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlurBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psCompositeBlob;

    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    auto CompileShader = [&](const char* entryPoint, const char* target, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        HRESULT h = D3DCompileFromFile(
            L"Assets/Shader/PostProcess.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint, target, compileFlags, 0, &blob, &error
        );
        if (FAILED(h))
        {
            if (error) OutputDebugStringA((char*)error->GetBufferPointer());
            throw std::runtime_error(std::string("Failed to compile PostProcess shader: ") + entryPoint);
        }
    };

    CompileShader("VSMain",        "vs_5_0", vsBlob);
    CompileShader("PSPassThrough", "ps_5_0", psPassThroughBlob);
    CompileShader("PSBrightPass",  "ps_5_0", psBrightBlob);
    CompileShader("PSBlurPass",    "ps_5_0", psBlurBlob);
    CompileShader("PSComposite",   "ps_5_0", psCompositeBlob);

    // --------------------------------------------------------
    // 3. 各PSO作成ヘルパー
    // --------------------------------------------------------
    auto BuildPSO = [&](ID3DBlob* ps, Microsoft::WRL::ComPtr<ID3D12PipelineState>& outPSO)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_pRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps);

        // フルスクリーンクアッドのため頂点レイアウトは空
        psoDesc.InputLayout = { nullptr, 0 };

        // ブレンドステート (不透明上書き)
        CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
        psoDesc.BlendState = blendDesc;

        // ラスタライザーステート (カリングなし)
        CD3DX12_RASTERIZER_DESC rastDesc(D3D12_DEFAULT);
        rastDesc.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.RasterizerState = rastDesc;

        // 深度テスト無効
        CD3DX12_DEPTH_STENCIL_DESC depthDesc(D3D12_DEFAULT);
        depthDesc.DepthEnable = FALSE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        psoDesc.DepthStencilState = depthDesc;

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        HRESULT h = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&outPSO));
        if (FAILED(h))
        {
            throw std::runtime_error("Failed to create PostProcess PSO");
        }
    };

    BuildPSO(psPassThroughBlob.Get(), m_pPassThroughPSO);
    BuildPSO(psBrightBlob.Get(),      m_pBrightPSO);
    BuildPSO(psBlurBlob.Get(),        m_pBlurPSO);
    BuildPSO(psCompositeBlob.Get(),   m_pCompositePSO);

    // --------------------------------------------------------
    // 4. ブルーム用 1/2 解像度テクスチャの作成
    // --------------------------------------------------------
    UINT bloomWidth  = SCREEN_WIDTH / 2;
    UINT bloomHeight = SCREEN_HEIGHT / 2;

    m_pBrightTex   = std::make_unique<RenderTexture>(pDevice, bloomWidth, bloomHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_pBlurTexTemp = std::make_unique<RenderTexture>(pDevice, bloomWidth, bloomHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    // --------------------------------------------------------
    // 5. 最終出力用 フル解像度テクスチャの作成 (Viewport表示用)
    // --------------------------------------------------------
    m_pFinalTex = std::make_unique<RenderTexture>(pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void PostProcessPass::Execute(const RenderContext& ctx)
{
    if (!m_pSourceTex || !m_pCompositePSO || !m_pFinalTex) return;

    // 0. SRVデスクリプタヒープバインド
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    ctx.cmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const LightManager& lightMgr = LightManager::GetInstance();
    bool bloomEnabled = lightMgr.IsBloomEnabled();
    int effectType = lightMgr.GetPostProcessEffectType();

    struct PostProcessConstants
    {
        float threshold;
        float knee;
        float intensity;
        float spread;
        float dirX;
        float dirY;
        float bloomEnabled;
        float effectType;
    };

    D3D12_VIEWPORT vpFull = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scFull = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };

    if (bloomEnabled && m_pBrightTex && m_pBlurTexTemp)
    {
        float threshold = lightMgr.GetBloomThreshold();
        float intensity = lightMgr.GetBloomIntensity();
        float spread    = lightMgr.GetBloomSpread();

        UINT bloomW = m_pBrightTex->GetWidth();
        UINT bloomH = m_pBrightTex->GetHeight();

        D3D12_VIEWPORT vpHalf = { 0.0f, 0.0f, static_cast<float>(bloomW), static_cast<float>(bloomH), 0.0f, 1.0f };
        D3D12_RECT scHalf = { 0, 0, static_cast<LONG>(bloomW), static_cast<LONG>(bloomH) };

        // --------------------------------------------------------
        // Step 1: 輝度抽出 (Bright Pass: SceneTex -> BrightTex)
        // --------------------------------------------------------
        m_pSourceTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE brightRtv = m_pBrightTex->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &brightRtv, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpHalf);
        ctx.cmdList->RSSetScissorRects(1, &scHalf);

        ctx.cmdList->SetPipelineState(m_pBrightPSO.Get());

        PostProcessConstants cb{};
        cb.threshold    = threshold;
        cb.knee         = 0.2f;
        cb.intensity    = intensity;
        cb.spread       = spread;
        cb.dirX         = 0.0f;
        cb.dirY         = 0.0f;
        cb.bloomEnabled = 1.0f;
        cb.effectType   = static_cast<float>(effectType);
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pSourceTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // --------------------------------------------------------
        // Step 2: ガウスブラー (Horizontal Blur: BrightTex -> BlurTexTemp)
        // --------------------------------------------------------
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBlurTexTemp->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE blurRtv = m_pBlurTexTemp->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &blurRtv, FALSE, nullptr);

        ctx.cmdList->SetPipelineState(m_pBlurPSO.Get());

        cb.dirX = 1.0f / static_cast<float>(bloomW);
        cb.dirY = 0.0f;
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pBrightTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // --------------------------------------------------------
        // Step 3: ガウスブラー (Vertical Blur: BlurTexTemp -> BrightTex)
        // --------------------------------------------------------
        m_pBlurTexTemp->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        ctx.cmdList->OMSetRenderTargets(1, &brightRtv, FALSE, nullptr);

        cb.dirX = 0.0f;
        cb.dirY = 1.0f / static_cast<float>(bloomH);
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pBlurTexTemp->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // --------------------------------------------------------
        // Step 4: 最終合成 (Composite: SceneTex + BrightTex -> m_pFinalTex)
        // --------------------------------------------------------
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pFinalTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE finalRtv = m_pFinalTex->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &finalRtv, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpFull);
        ctx.cmdList->RSSetScissorRects(1, &scFull);

        ctx.cmdList->SetPipelineState(m_pCompositePSO.Get());

        cb.bloomEnabled = 1.0f;
        cb.effectType   = static_cast<float>(effectType);
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pSourceTex->GetSRV());
        ctx.cmdList->SetGraphicsRootDescriptorTable(2, m_pBrightTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // Viewport表示用にテクスチャをPIXEL_SHADER_RESOURCEへ遷移
        m_pFinalTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        CViewportUI::GetInstance().SetTextureSRV(m_pFinalTex->GetSRV());

        // --------------------------------------------------------
        // Step 5: バックバッファへの出力 (m_pFinalTex -> BackBuffer)
        // ※DebugビルドでViewportUI表示時はViewportのみ出力し、バックバッファへは描画しない
        // --------------------------------------------------------
        bool outputToBackBuffer = true;
#ifdef _DEBUG
        if (CViewportUI::GetInstance().IsVisible())
        {
            outputToBackBuffer = false;
        }
#endif
        if (outputToBackBuffer)
        {
            ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, nullptr);
            ctx.cmdList->RSSetViewports(1, &vpFull);
            ctx.cmdList->RSSetScissorRects(1, &scFull);

            ctx.cmdList->SetPipelineState(m_pPassThroughPSO.Get());
            ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pFinalTex->GetSRV());
            ctx.cmdList->DrawInstanced(3, 1, 0, 0);
        }
    }
    else
    {
        // --------------------------------------------------------
        // ブルーム無効: パススルー (SceneTex -> m_pFinalTex -> BackBuffer)
        // --------------------------------------------------------
        m_pSourceTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pFinalTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE finalRtv = m_pFinalTex->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &finalRtv, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpFull);
        ctx.cmdList->RSSetScissorRects(1, &scFull);

        ctx.cmdList->SetPipelineState(m_pPassThroughPSO.Get());

        PostProcessConstants cb{};
        cb.effectType = static_cast<float>(effectType);
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pSourceTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // Viewport表示用にテクスチャをPIXEL_SHADER_RESOURCEへ遷移
        m_pFinalTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        CViewportUI::GetInstance().SetTextureSRV(m_pFinalTex->GetSRV());

        // バックバッファへ出力
        bool outputToBackBuffer = true;
#ifdef _DEBUG
        if (CViewportUI::GetInstance().IsVisible())
        {
            outputToBackBuffer = false;
        }
#endif
        if (outputToBackBuffer)
        {
            ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, nullptr);
            ctx.cmdList->RSSetViewports(1, &vpFull);
            ctx.cmdList->RSSetScissorRects(1, &scFull);

            ctx.cmdList->SetPipelineState(m_pPassThroughPSO.Get());
            ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pFinalTex->GetSRV());
            ctx.cmdList->DrawInstanced(3, 1, 0, 0);
        }
    }
}
