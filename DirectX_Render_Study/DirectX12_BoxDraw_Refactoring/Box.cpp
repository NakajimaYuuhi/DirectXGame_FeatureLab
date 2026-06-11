//===== インクルード =====
#include "Box.h"
#include "DX12Manager.h"

//----- シェーダーコンパイル用 -----
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


#include "BasicSettings.h"



//TODO:構造を変更する
struct Vertex
{
    float position[3];
    float color[4];
    float uv[2];
};






//頂点データの作成
//四角形のレイアウトに変更
//外から作成できるようにする
Vertex vertices[] =
{
    //立方体(インデックス)
        // ===== 前面 (Z-) =====
    {{-0.5f,-0.5f,-0.5f},{1,0,0,1},{0,1}},
    {{-0.5f, 0.5f,-0.5f},{1,0,0,1},{0,0}},
    {{ 0.5f, 0.5f,-0.5f},{1,0,0,1},{1,0}},
    {{ 0.5f,-0.5f,-0.5f},{1,0,0,1},{1,1}},

    // ===== 背面 (Z+) =====
    {{-0.5f,-0.5f,0.5f},{0,1,0,1},{1,1}},
    {{ 0.5f,-0.5f,0.5f},{0,1,0,1},{0,1}},
    {{ 0.5f, 0.5f,0.5f},{0,1,0,1},{0,0}},
    {{-0.5f, 0.5f,0.5f},{0,1,0,1},{1,0}},

    // ===== 左 (X-) =====
    {{-0.5f,-0.5f, 0.5f},{0,0,1,1},{0,1}},
    {{-0.5f, 0.5f, 0.5f},{0,0,1,1},{0,0}},
    {{-0.5f, 0.5f,-0.5f},{0,0,1,1},{1,0}},
    {{-0.5f,-0.5f,-0.5f},{0,0,1,1},{1,1}},

    // ===== 右 (X+) =====
    {{0.5f,-0.5f,-0.5f},{1,1,0,1},{0,1}},
    {{0.5f, 0.5f,-0.5f},{1,1,0,1},{0,0}},
    {{0.5f, 0.5f, 0.5f},{1,1,0,1},{1,0}},
    {{0.5f,-0.5f, 0.5f},{1,1,0,1},{1,1}},

    // ===== 上 (Y+) =====
    {{-0.5f,0.5f,-0.5f},{1,0,1,1},{0,1}},
    {{-0.5f,0.5f, 0.5f},{1,0,1,1},{0,0}},
    {{ 0.5f,0.5f, 0.5f},{1,0,1,1},{1,0}},
    {{ 0.5f,0.5f,-0.5f},{1,0,1,1},{1,1}},

    // ===== 下 (Y-) =====
    {{-0.5f,-0.5f, 0.5f},{0,1,1,1},{1,0}},
    {{-0.5f,-0.5f,-0.5f},{0,1,1,1},{1,1}},
    {{ 0.5f,-0.5f,-0.5f},{0,1,1,1},{0,1}},
    {{ 0.5f,-0.5f, 0.5f},{0,1,1,1},{0,0}},
};

uint16_t indices[] =
{
    0,1,2, 0,2,3,        // 前
    4,5,6, 4,6,7,        // 背
    8,9,10, 8,10,11,     // 左
    12,13,14, 12,14,15,  // 右
    16,17,18, 16,18,19,  // 上
    20,21,22, 20,22,23   // 下
};

CBox::CBox()
{
	//Initialize();
}

//===== メソッド定義 =====
void CBox::Initialize(ID3D12Device* _Device)
{


	//----- シェーダーコンパイル -----
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr;

    //頂点シェーダのコンパイル
    hr = D3DCompileFromFile(
        L"Triangle.hlsl",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        0,
        0,
        &vertexShader,
        &errorBlob
    );

	//ピクセルシェーダのコンパイル
    hr = D3DCompileFromFile(
        L"Triangle.hlsl",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        0,
        0,
        &pixelShader,
        &errorBlob
    );

    //----- ルートシグネチャ -----
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    //WVPようにパラメータを設定
    D3D12_ROOT_PARAMETER rootParam{};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam.Descriptor.ShaderRegister = 0; // b0
    rootParam.Descriptor.RegisterSpace = 0;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;


    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParam;    //パラメータを設定するように変更
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ルートシグネチャのシリアライズ
    D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );

	//ルートシグネチャの作成
    hr = _Device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)
    );

    //頂点レイアウトの作成
    //0,12,28がマジックナンバー
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    //PSOの設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);

    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.NumRenderTargets = 1;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = UINT_MAX;

	//ラスタライザーステートの設定
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

    psoDesc.RasterizerState = rasterDesc;

    psoDesc.BlendState = {};
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


    //ここを変える？
    //psoDesc.DepthStencilState.DepthEnable = FALSE;
    //psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    //psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    //psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    
    //----- 深度バッファリソース作成 -----
   
    
	//----- DepthStencilStateの設定 -----
    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;

    psoDesc.DepthStencilState = depthDesc;

    // これ超重要
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.NumRenderTargets = 1;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.pRootSignature = m_rootSignature.Get();


    //GraphicsPipelineStateを作成
    hr = _Device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_pipelineState)
    );

    if (FAILED(hr))
    {
        MessageBoxA(nullptr, "PSO creation failed", "Error", MB_OK);
    }


	//----- 頂点バッファの作成 -----
    //サイズ計算
    UINT vertexBufferSize = sizeof(vertices);

    //リソース作成（UploadHeap）
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = vertexBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    _Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );

	//頂点データをバッファにコピー
    void* mappedData = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, vertices, vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビューの設定
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);


	//----- 定数バッファの作成 -----
    //定数バッファのサイズを256バイトの倍数にする
    UINT constantBufferSize =
        (sizeof(ConstantBufferData) + 255) & ~255;

	//UnloadHeapでリソース作成
    //D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    //D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = constantBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    _Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)
    );

    m_constantBuffer->Map(0, nullptr, (void**)&m_cbData);


	//----- インデックスバッファの作成 -----
	//サイズ計算
    const UINT indexBufferSize = sizeof(indices);

	//リソース作成（UploadHeap）
    D3D12_HEAP_PROPERTIES heapProps2 = {};
    heapProps2.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps2.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps2.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps2.CreationNodeMask = 1;
    heapProps2.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc2 = {};
    resourceDesc2.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc2.Alignment = 0;
    resourceDesc2.Width = indexBufferSize;
    resourceDesc2.Height = 1;
    resourceDesc2.DepthOrArraySize = 1;
    resourceDesc2.MipLevels = 1;
    resourceDesc2.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc2.SampleDesc.Count = 1;
    resourceDesc2.SampleDesc.Quality = 0;
    resourceDesc2.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc2.Flags = D3D12_RESOURCE_FLAG_NONE;

    _Device->CreateCommittedResource(
        &heapProps2,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc2,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)
    );

	//インデックスデータをバッファにコピー
    uint8_t* mappedData2 = nullptr;
    m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData2));
    memcpy(mappedData2, indices, indexBufferSize);
    m_indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューの設定
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = sizeof(indices);
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT; // uint16_tならこれ


    

}

void CBox::Draw(ID3D12GraphicsCommandList* _CommandList)
{
    //行列作成
    DirectX::XMMATRIX world =
        DirectX::XMMatrixScaling(
            m_Scale.x,
            m_Scale.y,
            m_Scale.z)
        *
        DirectX::XMMatrixTranslation(
            m_Position.x,
            m_Position.y,
            m_Position.z);

	//ビューとプロジェクションはDX12Managerから取得
	DirectX::XMMATRIX view = DX12Manager::GetInstance().GetView();
    DirectX::XMMATRIX proj = DX12Manager::GetInstance().GetProj();


    DirectX::XMMATRIX wvp = world * view * proj;

    m_cbData->WVP = XMMatrixTranspose(wvp);



    ConstantBufferData* cbData = nullptr;
    m_constantBuffer->Map(0, nullptr, (void**)&cbData);
    cbData->WVP = XMMatrixTranspose(wvp);
    m_constantBuffer->Unmap(0, nullptr);


    _CommandList->SetPipelineState(m_pipelineState.Get());
    _CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
    
    _CommandList->SetGraphicsRootConstantBufferView(
        0,
        m_constantBuffer->GetGPUVirtualAddress()
    );

    _CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //インデックス用に変更
    _CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    _CommandList->IASetIndexBuffer(&m_indexBufferView);
    _CommandList->DrawIndexedInstanced(sizeof(indices)/sizeof(indices[0]), 1, 0, 0, 0);
}
