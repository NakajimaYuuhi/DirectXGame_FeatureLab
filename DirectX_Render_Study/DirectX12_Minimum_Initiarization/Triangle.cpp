//===== インクルード =====
#include "Triangle.h"

//----- シェーダーコンパイル用 -----
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


struct Vertex
{
    float position[3];
    float color[4];
};

//頂点データの作成
Vertex vertices[] =
{
    { { 0.0f, 0.25f, 0.0f }, { 1,0,0,1 } },
    { { 0.25f, -0.25f, 0.0f }, { 0,1,0,1 } },
    { { -0.25f, -0.25f, 0.0f }, { 0,0,1,1 } },
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

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = 0;
    rootSignatureDesc.pParameters = nullptr;
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
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
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


}

void CTriangle::Draw(ID3D12GraphicsCommandList* _CommandList)
{
    _CommandList->SetPipelineState(m_pipelineState.Get());
    _CommandList->SetGraphicsRootSignature(m_rootSignature.Get());

    _CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    _CommandList->DrawInstanced(3, 1, 0, 0);
}
