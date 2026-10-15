#include "CompositePass.h"
#include <d3dcompiler.h>
#include <stdexcept>
#include "d3dx12.h"
#include "DX12Manager.h"
#include "BasicSettings.h"
#include "ViewportUI.h"
#include "RenderBlackboard.h"

#pragma comment(lib, "d3dcompiler.lib")

CompositePass::CompositePass(const std::string& outputTexName)
{
    m_outputResourceName = outputTexName.empty() ? "CompositeFinal" : outputTexName;

    // Default layers
    Layer l0;
    l0.textureName = "SceneColor";
    l0.blendMode = BlendMode::Alpha;
    l0.opacity = 1.0f;
    l0.enabled = true;
    m_layers.push_back(l0);
}

void CompositePass::RemoveLayer(size_t index)
{
    if (index < m_layers.size())
    {
        m_layers.erase(m_layers.begin() + index);
    }
}

void CompositePass::Init(ID3D12Device* pDevice)
{
    if (!pDevice) return;

    // 1. Root Signature
    CD3DX12_ROOT_PARAMETER1 rootParams[2];
    rootParams[0].InitAsConstants(4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL); // b0

    CD3DX12_DESCRIPTOR_RANGE1 rangeT0;
    rangeT0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
    rootParams[1].InitAsDescriptorTable(1, &rangeT0, D3D12_SHADER_VISIBILITY_PIXEL); // t0

    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(2, rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob, errBlob;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &sigBlob, &errBlob);
    if (FAILED(hr)) {
        if (errBlob) OutputDebugStringA((char*)errBlob->GetBufferPointer());
        throw std::runtime_error("Failed to serialize CompositePass root signature");
    }
    pDevice->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));

    // 2. Compile Shaders
    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, compileErr;
    hr = D3DCompileFromFile(L"Assets/Shader/Composite.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_5_0", compileFlags, 0, &vsBlob, &compileErr);
    if (FAILED(hr)) {
        if (compileErr) OutputDebugStringA((char*)compileErr->GetBufferPointer());
        throw std::runtime_error("Failed to compile Composite VS");
    }

    hr = D3DCompileFromFile(L"Assets/Shader/Composite.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_5_0", compileFlags, 0, &psBlob, &compileErr);
    if (FAILED(hr)) {
        if (compileErr) OutputDebugStringA((char*)compileErr->GetBufferPointer());
        throw std::runtime_error("Failed to compile Composite PS");
    }

    // 3. Build Pipeline State Objects for various blend modes
    auto CreateBlendPSO = [&](D3D12_BLEND_DESC blendDesc, Microsoft::WRL::ComPtr<ID3D12PipelineState>& outPSO)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.pRootSignature = m_pRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
        psoDesc.BlendState = blendDesc;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&outPSO));
    };

    // Copy (No blend)
    D3D12_BLEND_DESC copyBlend = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    CreateBlendPSO(copyBlend, m_psoCopy);

    // Alpha Blend
    D3D12_BLEND_DESC alphaBlend = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    alphaBlend.RenderTarget[0].BlendEnable = TRUE;
    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    CreateBlendPSO(alphaBlend, m_psoAlpha);

    // Additive Blend
    D3D12_BLEND_DESC addBlend = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    addBlend.RenderTarget[0].BlendEnable = TRUE;
    addBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    addBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    addBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    addBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    addBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    addBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    CreateBlendPSO(addBlend, m_psoAdditive);

    // Multiply Blend
    D3D12_BLEND_DESC mulBlend = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    mulBlend.RenderTarget[0].BlendEnable = TRUE;
    mulBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_DEST_COLOR;
    mulBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    mulBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    mulBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_DEST_ALPHA;
    mulBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    mulBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    CreateBlendPSO(mulBlend, m_psoMultiply);

    // Screen Blend
    D3D12_BLEND_DESC screenBlend = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    screenBlend.RenderTarget[0].BlendEnable = TRUE;
    screenBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    screenBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_COLOR;
    screenBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    screenBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    screenBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    screenBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    CreateBlendPSO(screenBlend, m_psoScreen);

    // 4. Create Output Render Target
    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_pOutputTex = std::make_unique<RenderTexture>(pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, clearColor);
}

void CompositePass::Execute(const RenderContext& ctx)
{
    if (!m_pOutputTex || !m_pRootSignature) return;

    // Register final output texture to blackboard
    if (ctx.blackboard && !m_outputResourceName.empty()) {
        ctx.blackboard->RegisterTextureRef(m_outputResourceName, m_pOutputTex.get());
    }

    // 0. Bind descriptor heap & root signature
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    ctx.cmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 1. Prepare Output Render Target
    m_pOutputTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pOutputTex->GetRTV();
    ctx.cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ctx.cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 2. Render each active layer in order
    for (const auto& layer : m_layers)
    {
        if (!layer.enabled || layer.textureName.empty()) continue;

        RenderTexture* pLayerTex = ctx.blackboard ? ctx.blackboard->GetTexture(layer.textureName) : nullptr;
        if (!pLayerTex) continue;

        pLayerTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // Select PSO by blend mode
        ID3D12PipelineState* pso = m_psoAlpha.Get();
        switch (layer.blendMode)
        {
        case BlendMode::Alpha:    pso = m_psoAlpha.Get(); break;
        case BlendMode::Additive: pso = m_psoAdditive.Get(); break;
        case BlendMode::Multiply: pso = m_psoMultiply.Get(); break;
        case BlendMode::Screen:   pso = m_psoScreen.Get(); break;
        default:                  pso = m_psoAlpha.Get(); break;
        }

        ctx.cmdList->SetPipelineState(pso);

        // Set Root Constants (opacity)
        float constData[4] = { layer.opacity, 0.0f, 0.0f, 0.0f };
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 4, constData, 0);

        // Bind Texture
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, pLayerTex->GetSRV());

        // Fullscreen Quad Draw
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);
    }

    // 3. Transition output texture to SRV and update Viewport
    m_pOutputTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    CViewportUI::GetInstance().SetTextureSRV(m_pOutputTex->GetSRV());
}
