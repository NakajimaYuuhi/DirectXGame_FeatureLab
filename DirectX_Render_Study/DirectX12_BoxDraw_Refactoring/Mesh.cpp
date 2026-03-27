#include "Mesh.h"
#include "DX12Manager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "BasicSettings.h"

//Object
#include "Object.h"

//Transform
#include "Transform.h"
#include "SceneTest.h"

#include "Material.h"



//TODO:同じ形のプリミティブは、頂点バッファを共通にしたい
//一旦頂点情報を外から入れるのは、後回しで

//----- 後で移すもの -----
//struct MeshVertex
//{
//    float position[3];
//    float color[4];
//    float uv[2];
//};

struct MeshVertex
{
    float position[3];
    float normal[3];
    float uv[2];

    uint8_t boneIndices[4];  // ボーン番号（最大255体まで）
    float   boneWeights[4];  // ボーンの重み
};

//頂点データの作成
MeshVertex mesh_vertices[] =
{
    //立方体(インデックス)
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


};

uint16_t mesh_indices[] =
{
    0,1,2, 0,2,3,        // 前
    4,5,6, 4,6,7,        // 背
    8,9,10, 8,10,11,     // 左
    12,13,14, 12,14,15,  // 右
    16,17,18, 16,18,19,  // 上
    20,21,22, 20,22,23   // 下
};




//Initializeをどこかで呼ぶ必要有り
CMesh::CMesh(CMaterial* _Material)
    :m_Material(_Material)
{ }

void CMesh::Init()
{

    //Transformの登録
    RegisterTransform();

    //----- シェーダーコンパイル -----
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;

    //デバイス
    ID3D12Device* device = CDX12Manager::GetInstance().GetDevice();
    ID3D12GraphicsCommandList* cmdList = CDX12Manager::GetInstance().GetCommandLIst();

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
    //複数送るので、配列に変更
    D3D12_ROOT_PARAMETER rootParams[2] = {};

    // --CBV (b0)
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0; // b0
    rootParams[0].Descriptor.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // --- SRV (t0)
    D3D12_DESCRIPTOR_RANGE srvRange{};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 0; // t0
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &srvRange;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // --Sampler (s0)
    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ShaderRegister = 0; // s0
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // --RootSignatureDesc
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = _countof(rootParams);
    rootSignatureDesc.pParameters = rootParams;    //パラメータを設定するように変更
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &sampler;   //サンプラーも登録
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
    hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)
    );




    //頂点レイアウトの作成
    //0,12,28がマジックナンバー
    //D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    //{
    //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
    //      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

    //    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
    //      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

    //    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28,
    //      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    //};

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        // ボーン番号（uint8 × 4） → R8G8B8A8_UINT
        { "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        // ボーンの重み（float × 4）
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
    hr = device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_pipelineState)
    );

    if (FAILED(hr))
    {
        MessageBoxA(nullptr, "PSO creation failed", "Error", MB_OK);
    }


    //----- 頂点バッファの作成 -----
    //サイズ計算
    UINT vertexBufferSize = sizeof(mesh_vertices);

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

    device->CreateCommittedResource(
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
    memcpy(mappedData, mesh_vertices, vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    //頂点バッファビューの設定
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
    m_vertexBufferView.StrideInBytes = sizeof(MeshVertex);


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

    device->CreateCommittedResource(
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
    const UINT indexBufferSize = sizeof(mesh_indices);

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

    device->CreateCommittedResource(
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
    memcpy(mappedData2, mesh_indices, indexBufferSize);
    m_indexBuffer->Unmap(0, nullptr);

    //インデックスバッファビューの設定
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = sizeof(mesh_indices);
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT; // uint16_tならこれ




    //----- 消す -----

    // --- 初期化処理などの安全な場所で ---
    //本当は、全体で一括でコマンドリストの開閉を行う

    //// 1. コマンドリストを開く
    //CDX12Manager::GetInstance().GetCommandAllocator()->Reset();
    //cmdList->Reset(CDX12Manager::GetInstance().GetCommandAllocator(), nullptr);


    ////-----Texture関連(仮実装)-----
    //m_Texture.LoadTexture(device, cmdList, L"Assets/Texture/Sample1.jpg", 0);
    //m_Texture.CreateSRV(device);

    //// --- ここでコマンドを実行して投げる ---
    //cmdList->Close();
    //ID3D12CommandList* list[] = { cmdList };
    //CDX12Manager::GetInstance().GetCommandQueue()->ExecuteCommandLists(1, list);

    //// ★ここでGPUがコピーを終えるまで、CPUをストップさせる！
    //CDX12Manager::GetInstance().ForceWait();


}

void CMesh::Update()
{
}



void CMesh::Draw()
{
    // --コマンドリスト
    ID3D12GraphicsCommandList* commandList = CDX12Manager::GetInstance().GetCommandLIst();


    // --行列取得
    DirectX::XMMATRIX world = m_Transform->GetWorld();
    DirectX::XMMATRIX view = CDX12Manager::GetInstance().GetView();
    DirectX::XMMATRIX proj = CDX12Manager::GetInstance().GetProj();


    // --掛け算
    DirectX::XMMATRIX wvp = world * view * proj;

    // --定数バッファ用のデータにセットする
    //1
    m_cbData->WVP = XMMatrixTranspose(wvp);//ここでセットしたやつ使ってる？　使ってなさそう

    //2
    ConstantBufferData* cbData = nullptr;
    m_constantBuffer->Map(0, nullptr, (void**)&cbData);
    cbData->WVP = XMMatrixTranspose(wvp);
    m_constantBuffer->Unmap(0, nullptr);
    //1,2のどっちかでいい


    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    //SRVヒープのセット
    ID3D12DescriptorHeap* heaps[] = { CDX12Manager::GetInstance().GetSRVHeap() };

    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootDescriptorTable(
        1,
        m_Material->GetGpuHandle()
    );


    commandList->SetGraphicsRootConstantBufferView(
        0,
        m_constantBuffer->GetGPUVirtualAddress()
    );

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //インデックス用に変更
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(sizeof(mesh_indices) / sizeof(mesh_indices[0]), 1, 0, 0, 0);
}

//Transformの登録
void CMesh::RegisterTransform()
{
    //GetComponentをする
    m_Transform = m_Owner->GetComponent<CTransform>();
}

void CMesh::RegisterOwner(CObject* _Owner)
{
    m_Owner = _Owner;
}
