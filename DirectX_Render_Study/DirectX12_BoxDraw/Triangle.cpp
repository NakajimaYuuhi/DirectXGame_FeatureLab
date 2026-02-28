//===== インクルード =====
#include "Triangle.h"

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
Vertex vertices[] =
{
    //三角形
    //{ { 0.0f, 0.25f, 0.0f }, { 1,0,0,1 } },
    //{ { 0.25f, -0.25f, 0.0f }, { 0,1,0,1 } },
    //{ { -0.25f, -0.25f, 0.0f }, { 0,0,1,1 } },

    //四角形
    // 上
    //{ { -0.3f,  0.3f, 0.0f }, { 1,0,0,1 } },
    //{ {  0.3f,  0.3f, 0.0f }, { 0,1,0,1 } },
    //{ { -0.3f, -0.3f, 0.0f }, { 0,0,1,1 } },

    //{ { -0.3f, -0.3f, 0.0f }, { 0,0,1,1 } },
    //{ {  0.3f,  0.3f, 0.0f }, { 0,1,0,1 } },
    //{ {  0.3f, -0.3f, 0.0f }, { 1,1,0,1 } },

    ////立方体(非インデックス)
    //    // 前面
    //{{-0.5f,-0.5f,-0.5f},{1,0,0,1}},
    //{{-0.5f, 0.5f,-0.5f},{1,0,0,1}},
    //{{ 0.5f, 0.5f,-0.5f},{1,0,0,1}},
    //{{-0.5f,-0.5f,-0.5f},{1,0,0,1}},
    //{{ 0.5f, 0.5f,-0.5f},{1,0,0,1}},
    //{{ 0.5f,-0.5f,-0.5f},{1,0,0,1}},

    //// 背面
    //{{-0.5f,-0.5f,0.5f},{0,1,0,1}},
    //{{ 0.5f, 0.5f,0.5f},{0,1,0,1}},
    //{{-0.5f, 0.5f,0.5f},{0,1,0,1}},
    //{{-0.5f,-0.5f,0.5f},{0,1,0,1}},
    //{{ 0.5f,-0.5f,0.5f},{0,1,0,1}},
    //{{ 0.5f, 0.5f,0.5f},{0,1,0,1}},

    //// 左
    //{{-0.5f,-0.5f,0.5f},{0,0,1,1}},
    //{{-0.5f, 0.5f,0.5f},{0,0,1,1}},
    //{{-0.5f, 0.5f,-0.5f},{0,0,1,1}},
    //{{-0.5f,-0.5f,0.5f},{0,0,1,1}},
    //{{-0.5f, 0.5f,-0.5f},{0,0,1,1}},
    //{{-0.5f,-0.5f,-0.5f},{0,0,1,1}},

    //// 右
    //{{0.5f,-0.5f,-0.5f},{1,1,0,1}},
    //{{0.5f, 0.5f,-0.5f},{1,1,0,1}},
    //{{0.5f, 0.5f, 0.5f},{1,1,0,1}},
    //{{0.5f,-0.5f,-0.5f},{1,1,0,1}},
    //{{0.5f, 0.5f, 0.5f},{1,1,0,1}},
    //{{0.5f,-0.5f, 0.5f},{1,1,0,1}},

    //// 上
    //{{-0.5f,0.5f,-0.5f},{1,0,1,1}},
    //{{-0.5f,0.5f, 0.5f},{1,0,1,1}},
    //{{ 0.5f,0.5f, 0.5f},{1,0,1,1}},
    //{{-0.5f,0.5f,-0.5f},{1,0,1,1}},
    //{{ 0.5f,0.5f, 0.5f},{1,0,1,1}},
    //{{ 0.5f,0.5f,-0.5f},{1,0,1,1}},

    //// 下
    //{{-0.5f,-0.5f,-0.5f},{0,1,1,1}},
    //{{ 0.5f,-0.5f, 0.5f},{0,1,1,1}},
    //{{-0.5f,-0.5f, 0.5f},{0,1,1,1}},
    //{{-0.5f,-0.5f,-0.5f},{0,1,1,1}},
    //{{ 0.5f,-0.5f,-0.5f},{0,1,1,1}},
    //{{ 0.5f,-0.5f, 0.5f},{0,1,1,1}},

    //立方体(インデックス)
        // ===== 前面 (Z-) =====
    {{-0.5f,-0.5f,-0.5f},{0,0,-1},{0,1}},
    {{-0.5f, 0.5f,-0.5f},{0,0,-1},{0,0}},
    {{ 0.5f, 0.5f,-0.5f},{0,0,-1},{1,0}},
    {{ 0.5f,-0.5f,-0.5f},{0,0,-1},{1,1}},

    // ===== 背面 (Z+) =====
    {{-0.5f,-0.5f,0.5f},{0,0,1},{1,1}},
    {{ 0.5f,-0.5f,0.5f},{0,0,1},{0,1}},
    {{ 0.5f, 0.5f,0.5f},{0,0,1},{0,0}},
    {{-0.5f, 0.5f,0.5f},{0,0,1},{1,0}},

    // ===== 左 (X-) =====
    {{-0.5f,-0.5f, 0.5f},{-1,0,0},{0,1}},
    {{-0.5f, 0.5f, 0.5f},{-1,0,0},{0,0}},
    {{-0.5f, 0.5f,-0.5f},{-1,0,0},{1,0}},
    {{-0.5f,-0.5f,-0.5f},{-1,0,0},{1,1}},

    // ===== 右 (X+) =====
    {{0.5f,-0.5f,-0.5f},{1,0,0},{0,1}},
    {{0.5f, 0.5f,-0.5f},{1,0,0},{0,0}},
    {{0.5f, 0.5f, 0.5f},{1,0,0},{1,0}},
    {{0.5f,-0.5f, 0.5f},{1,0,0},{1,1}},

    // ===== 上 (Y+) =====
    {{-0.5f,0.5f,-0.5f},{0,1,0},{0,1}},
    {{-0.5f,0.5f, 0.5f},{0,1,0},{0,0}},
    {{ 0.5f,0.5f, 0.5f},{0,1,0},{1,0}},
    {{ 0.5f,0.5f,-0.5f},{0,1,0},{1,1}},

    // ===== 下 (Y-) =====
    {{-0.5f,-0.5f, 0.5f},{0,-1,0},{1,0}},
    {{-0.5f,-0.5f,-0.5f},{0,-1,0},{1,1}},
    {{ 0.5f,-0.5f,-0.5f},{0,-1,0},{0,1}},
    {{ 0.5f,-0.5f, 0.5f},{0,-1,0},{0,0}},
};


CTriangle::CTriangle()
{
	//Initialize();
}

//===== メソッド定義 =====
void CTriangle::Initialize(ID3D12Device* _Device)
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
    //変更した
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
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

    psoDesc.RasterizerState = {};
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;

    psoDesc.BlendState = {};
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.NumRenderTargets = 1;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.pRootSignature = m_rootSignature.Get();


    //GraphicsPipelineStateを作成
    hr = _Device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_pipelineState)
    );


	//----- 頂点バッファの作成 -----
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


}

void CTriangle::Draw(ID3D12GraphicsCommandList* _CommandList)
{
    //行列作成
    DirectX::XMMATRIX world =
        DirectX::XMMatrixTranslation(
            m_position.x,
            m_position.y,
            m_position.z);
    DirectX::XMMATRIX view =
        DirectX::XMMatrixLookAtLH(
            DirectX::XMVectorSet(0, 0, -3, 1),
            DirectX::XMVectorSet(0, 0, 0, 1),
            DirectX::XMVectorSet(0, 1, 0, 0));

    DirectX::XMMATRIX proj =
        DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XM_PIDIV4,
            (float)SCREEN_WIDTH / SCREEN_HEIGHT,
            0.1f,
            100.0f);

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
    //_CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    ////三角形
    ////_CommandList->DrawInstanced(3, 1, 0, 0);
    ////四角形
    //_CommandList->DrawInstanced(36, 1, 0, 0);

    //インデックス用に変更
    _CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    _CommandList->IASetIndexBuffer(&m_indexBufferView);
    _CommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}
