#include "PostProcessPass.h"
#include <d3dcompiler.h>
#include <stdexcept>

// D3DCompileを使うためのライブラリリンク
#pragma comment(lib, "d3dcompiler.lib")

// 必要に応じて d3dx12.h をインクルードします（プロジェクト内にある想定）
#include "d3dx12.h" 
#include "DX12Manager.h" // GetMainSRVHeap 等を使うため

PostProcessPass::PostProcessPass(RenderTexture* pSourceTex)
    : m_pSourceTex(pSourceTex)
{
}

void PostProcessPass::Init(ID3D12Device* pDevice) {
    // --------------------------------------------------------
    // 1. ルートシグネチャの作成 (SRV x1, サンプラ x1)
    // --------------------------------------------------------
    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

    // テクスチャをサンプリングするためのスタティックサンプラ
    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    // 頂点バッファ(InputLayout)を使わないので ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT は外してもOK
    rootSigDesc.Init_1_1(1, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ID3DBlob* signature = nullptr;
    ID3DBlob* error = nullptr;
    D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
    if (signature) signature->Release();
    if (error) error->Release();

    // --------------------------------------------------------
    // 2. シェーダーのコンパイル
    // --------------------------------------------------------
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    D3DCompileFromFile(L"Assets/Shader/PostProcess.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vsBlob, nullptr);
    D3DCompileFromFile(L"Assets/Shader/PostProcess.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &psBlob, nullptr);

    // --------------------------------------------------------
    // 3. PSO (パイプラインステート) の作成
    // --------------------------------------------------------
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_pRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob);
    
    // 頂点レイアウトは空っぽ (SV_VertexIDを使うため)
    psoDesc.InputLayout = { nullptr, 0 };
    
    // ブレンドステートとラスタライザ (デフォルト)
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    // カリングなし
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    
    psoDesc.DepthStencilState.DepthEnable = FALSE; // ポストプロセスでは深度テスト不要
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    // バックバッファのフォーマットに合わせる (大抵は R8G8B8A8_UNORM)
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; 
    psoDesc.SampleDesc.Count = 1;

    pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));

    if (vsBlob) vsBlob->Release();
    if (psBlob) psBlob->Release();
}

void PostProcessPass::Execute(const RenderContext& ctx) {
    if (!m_pSourceTex || !m_pPipelineState) return;

    // 1. オフスクリーンテクスチャを 読み込み用 (SRV) に遷移
    m_pSourceTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // 2. 描画先をバックバッファに設定し、クリアする
    ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, nullptr);
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(ctx.backBufferRTV, clearColor, 0, nullptr);

    // ビューポート設定
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)ctx.screenWidth, (float)ctx.screenHeight, 0.0f, 1.0f };
    D3D12_RECT scissor = { 0, 0, (LONG)ctx.screenWidth, (LONG)ctx.screenHeight };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissor);

    // 3. PSO とテクスチャのバインド
    ctx.cmdList->SetGraphicsRootSignature(m_pRootSignature);
    ctx.cmdList->SetPipelineState(m_pPipelineState);
    
    // SRVヒープをセット (RenderTextureが間借りしているメインSRVヒープ)
    // ※ DX12Managerに GetSRVHeap() がある前提です
    ID3D12DescriptorHeap* ppHeaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, ppHeaps);
    
    // オフスクリーンテクスチャの SRVハンドル(GPU側) をシェーダーの register(t0) に渡す
    ctx.cmdList->SetGraphicsRootDescriptorTable(0, m_pSourceTex->GetSRV());

    // 4. 三角形を1枚描画 (SV_VertexIDを使って画面全体を覆う)
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(3, 1, 0, 0);
}
