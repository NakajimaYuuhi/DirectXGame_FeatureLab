#include "PSOManager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

void PSOManager::Init(ID3D12Device* device)
{
    // ===== 変数宣言 =====

	// --エラーハンドリング用
    HRESULT hr;                 // DirectX関連

    // --シェーダ関連
    // メッシュ用
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

    // --スプライト用
    ComPtr<ID3DBlob> spriteVertexShader;
    ComPtr<ID3DBlob> spritePixelShader;


    // =========================================================
	//  1. シェーダーコンパイル 
    // =========================================================
    
	// Todo : エラーハンドリングをもう少しわかりやすくする
    // Todo : 固定のシェーダをコンパイルしているので、なんとかする

    // ----- 変数宣言 -----
    ComPtr<ID3DBlob> errorBlob; // シェーダ用

    // ----- メッシュ用のシェーダのコンパイル -----

    // 頂点シェーダ(共通)
    // コンパイル
    hr = D3DCompileFromFile(
        L"Triangle.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, &errorBlob
    );

    // エラー処理
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
    }

	// ピクセルシェーダ(共通)
    // コンパイル
    hr = D3DCompileFromFile(
        L"Triangle.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, &errorBlob
    );

    // エラー処理
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
    }


    // ----- スプライト用のシェーダのコンパイル -----
    
    // --頂点シェーダ(共通)
    // コンパイル
    hr = D3DCompileFromFile(
        L"Sprite.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &spriteVertexShader, &errorBlob
    );

    // エラー処理
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
    }

    // --ピクセルシェーダ(共通)
    // コンパイル
    hr = D3DCompileFromFile(
        L"Sprite.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &spritePixelShader, &errorBlob
    );

    // エラー処理
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
    }


    // =========================================================
    //  2. ルートシグネチャ作成 
    // =========================================================

    // ----- 変数宣言 -----
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;


    // ----- メッシュ用のルートシグネチャ作成 -----
    D3D12_ROOT_PARAMETER rootParams[3] = {};

    // --Root Constants (b0)
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[0].Constants.ShaderRegister = 0; // b0
    rootParams[0].Constants.RegisterSpace = 0;
    rootParams[0].Constants.Num32BitValues = 20; // WVP (16) + uvOffset (2) + uvScale (2) = 20 floats
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // --- SRV (t0 & t1)
    D3D12_DESCRIPTOR_RANGE srvRanges[2] = {};
    srvRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRanges[0].NumDescriptors = 1;
    srvRanges[0].BaseShaderRegister = 0; // t0
    srvRanges[0].RegisterSpace = 0;
    srvRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    srvRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRanges[1].NumDescriptors = 1;
    srvRanges[1].BaseShaderRegister = 1; // t1
    srvRanges[1].RegisterSpace = 0;
    srvRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // 1. Texture SRV (t0)
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &srvRanges[0];
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // 2. Bone SRV (t1)
    rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges = &srvRanges[1];
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // --Sampler (s0)
    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ShaderRegister = 0; // s0
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // ディスクの作成、設定
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = _countof(rootParams);
    rootSignatureDesc.pParameters = rootParams;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &sampler;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // バイナリデータに変換
    D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );

    // <ルートシグネチャの作成>
    hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_meshRootSignature)
    );


    // ----- スプライト用のルートシグネチャ作成 -----
    D3D12_ROOT_PARAMETER spriteRootParams[2] = {};

    // Root Constants (b0) -> WVP (16 floats) + color (4 floats) = 20 floats
    spriteRootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    spriteRootParams[0].Constants.ShaderRegister = 0; // b0
    spriteRootParams[0].Constants.RegisterSpace = 0;
    spriteRootParams[0].Constants.Num32BitValues = 20;
    spriteRootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Descriptor Table (t0) -> SRV
    D3D12_DESCRIPTOR_RANGE spriteSrvRange = {};
    spriteSrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    spriteSrvRange.NumDescriptors = 1;
    spriteSrvRange.BaseShaderRegister = 0; // t0
    spriteSrvRange.RegisterSpace = 0;
    spriteSrvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    spriteRootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    spriteRootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    spriteRootParams[1].DescriptorTable.pDescriptorRanges = &spriteSrvRange;
    spriteRootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC spriteSampler = sampler; // Reuse sampler from mesh PSO

    D3D12_ROOT_SIGNATURE_DESC spriteRootSignatureDesc{};
    spriteRootSignatureDesc.NumParameters = _countof(spriteRootParams);
    spriteRootSignatureDesc.pParameters = spriteRootParams;
    spriteRootSignatureDesc.NumStaticSamplers = 1;
    spriteRootSignatureDesc.pStaticSamplers = &spriteSampler;
    spriteRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    hr = D3D12SerializeRootSignature(
        &spriteRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error
    );

    hr = device->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_spriteRootSignature)
    );



    // =========================================================
    // 共通・部品Desc（パーツ）の準備
    // =========================================================

    // ===== インプットレイアウトの定義 =====
	// ----- メッシュ用のインプットレイアウトの定義 -----
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // 位置
		{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // 法線
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // UV座標
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // ボーンインデックス
		{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // ボーンウェイト
    };

    // ----- スプライト用のインプットレイアウトの定義 -----
    D3D12_INPUT_ELEMENT_DESC spriteInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };


    // ===== ラスタライザ設定 =====
    // ----- メッシュ用のラスタライザ設定 -----
    D3D12_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Rasterizer (No culling for 2D typically, or just backface)
    D3D12_RASTERIZER_DESC spriteRasterDesc = rasterDesc;
    spriteRasterDesc.CullMode = D3D12_CULL_MODE_NONE; // Often want none for 2D
    spriteRasterDesc.DepthClipEnable = FALSE;



    // ===== 深度設定 =====
    // ----- メッシュ用の深度設定 -----
    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;

    // ----- スプライト用の深度設定 -----
    D3D12_DEPTH_STENCIL_DESC spriteDepthDesc = {};
    spriteDepthDesc.DepthEnable = FALSE;
    spriteDepthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    spriteDepthDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    spriteDepthDesc.StencilEnable = FALSE;


    // ===== ブレンド設定 =====
    // ----- 不透明メッシュは特にセットするもの無し -----

    // ----- 加算合成メッシュのブレンド設定 -----   
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
    blendDesc.BlendEnable = TRUE;
    blendDesc.LogicOpEnable = FALSE;
    blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;         // SRC
    blendDesc.DestBlend = D3D12_BLEND_ONE;              // DEST (1 = 加算)
    blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // ----- スプライト用のブレンド設定 -----
    D3D12_BLEND_DESC spriteBlendDesc = {};
    spriteBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    spriteBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    spriteBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    spriteBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    spriteBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    spriteBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    spriteBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    spriteBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


    // =========================================================
    // PSOの組み立てと生成
    // =========================================================

    // ----- メッシュ用のPSOの設定 -----
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    // ルートシグネチャ
    psoDesc.pRootSignature = m_meshRootSignature.Get();

    // 頂点レイアウト・トポロジ
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // シェーダ
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

    // ラスタライザ
    psoDesc.RasterizerState = rasterDesc;

    // テスト設定(深度/ステンシルテスト)
    psoDesc.DepthStencilState = depthDesc;

    // 合成設定
    psoDesc.BlendState = {};
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // 出力フォーマット・マルチサンプリング
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.NumRenderTargets = 1;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = UINT_MAX;

    // --作成
    hr = device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_meshPipelineState)
    );


    // ----- 加算合成メッシュ用のPSOの設定 -----
    // 設定はほぼ引き継ぎ
    D3D12_GRAPHICS_PIPELINE_STATE_DESC additivePsoDesc = psoDesc;

    // テスト設定(深度/ステンシルテスト)
    // Zバッファへの書き込みを無効化（半透明描画の基本)
    additivePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    // 合成設定
    additivePsoDesc.BlendState.RenderTarget[0] = blendDesc;
    
    //--作成
    hr = device->CreateGraphicsPipelineState(
        &additivePsoDesc,
        IID_PPV_ARGS(&m_additivePipelineState)
    );


    // ----- スプライト用のPSOの設定 -----
    D3D12_GRAPHICS_PIPELINE_STATE_DESC spritePsoDesc = {};

    // ルートシグネチャ
    spritePsoDesc.pRootSignature = m_spriteRootSignature.Get();
    
    // 頂点レイアウト・トポロジ
    spritePsoDesc.InputLayout.pInputElementDescs = spriteInputLayout;
    spritePsoDesc.InputLayout.NumElements = _countof(spriteInputLayout);
    spritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // シェーダ
    spritePsoDesc.VS = { spriteVertexShader->GetBufferPointer(), spriteVertexShader->GetBufferSize() };
    spritePsoDesc.PS = { spritePixelShader->GetBufferPointer(), spritePixelShader->GetBufferSize() };
    
    // ラスタライザ
    spritePsoDesc.RasterizerState = spriteRasterDesc;

    // テスト設定(深度/ステンシルテスト)
    spritePsoDesc.DepthStencilState = spriteDepthDesc;

    // 合成設定
    spritePsoDesc.BlendState = spriteBlendDesc;

    // 出力フォーマット・マルチサンプリング
    spritePsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN; // DepthBufferは使わない
    spritePsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    spritePsoDesc.NumRenderTargets = 1;
    spritePsoDesc.SampleDesc.Count = 1;
    spritePsoDesc.SampleMask = UINT_MAX;

    // --作成
    hr = device->CreateGraphicsPipelineState(
        &spritePsoDesc,
        IID_PPV_ARGS(&m_spritePipelineState)
    );
}
