#include "Mesh.h"
#include "DX12Manager.h"
#include "PSOManager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "BasicSettings.h"
#include <stdio.h>

//Object
#include "Object.h"

//Transform
#include "Transform.h"
#include "SceneTest.h"

#include "Material.h"



//TODO:同じ形のプリミティブ E、E  点バッファを E通にしたぁE
//一旦頂点?E  を外から Eれるのは、後回しで



//頂点チE Eタの作 E
MeshVertex mesh_vertices[] =
{
    //立方?EインチE  クス)
    // ===== 丁E(Y+) =====
    {{-0.5f,0.5f,-0.5f},{0,1,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{-0.5f,0.5f, 0.5f},{0,1,0},{0,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,0.5f, 0.5f},{0,1,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,0.5f,-0.5f},{0,1,0},{1,1},{0,0,0,0},{1,0,0,0}},
    
    // ===== 丁E(Y-) =====
    {{-0.5f,-0.5f, 0.5f},{0,-1,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f,-0.5f,-0.5f},{0,-1,0},{1,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f,-0.5f},{0,-1,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f, 0.5f},{0,-1,0},{0,0},{0,0,0,0},{1,0,0,0}},

    // ===== 前面 (Z-) =====
    {{-0.5f,-0.5f,-0.5f},{0,0,-1},{0,1},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f,-0.5f},{0,0,-1},{0,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f, 0.5f,-0.5f},{0,0,-1},{1,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f,-0.5f},{0,0,-1},{1,1},{0,0,0,0},{1,0,0,0}},

    // ===== 背面 (Z+) =====
    {{-0.5f,-0.5f,0.5f},{0,0,1},{1,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f,0.5f},{0,0,1},{0,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f, 0.5f,0.5f},{0,0,1},{0,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f,0.5f},{0,0,1},{1,0},{0,0,0,0},{1,0,0,0}},

    // ===== 左 (X-) =====
    {{-0.5f,-0.5f, 0.5f},{-1,0,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f, 0.5f},{-1,0,0},{0,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f,-0.5f},{-1,0,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f,-0.5f,-0.5f},{-1,0,0},{1,1},{0,0,0,0},{1,0,0,0}},

    // ===== 右 (X+) =====
    {{0.5f,-0.5f,-0.5f},{1,0,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{0.5f, 0.5f,-0.5f},{1,0,0},{0,0},{0,0,0,0},{1,0,0,0}},
    {{0.5f, 0.5f, 0.5f},{1,0,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{0.5f,-0.5f, 0.5f},{1,0,0},{1,1},{0,0,0,0},{1,0,0,0}},


};

uint32_t mesh_indices[] =
{
    0,1,2, 0,2,3,        // ?E
    4,5,6, 4,6,7,        // ?E
    8,9,10, 8,10,11,     // 左
    12,13,14, 12,14,15,  // 右
    16,17,18, 16,18,19,  // 丁E
    20,21,22, 20,22,23   // 丁E
};

//




//Initializeをどこかで呼ぶ?E  有めE
//Initializeをどこかで呼ぶ?E  有めE
CMesh::CMesh()
{  
    //ここで、E  点?E  、インチE  クス?E  をデフォルトでセチE  (仮実裁E
    m_Vertices.assign(std::begin(mesh_vertices), std::end(mesh_vertices));//assignで入れれるらしい
    m_Indices.assign(std::begin(mesh_indices), std::end(mesh_indices));

}

void CMesh::Init()
{

    // m_Transform is no longer used.


    ////----- インチE  クスバッファの作 E -----
    ////サイズ計?E
    //const UINT indexBufferSize = sizeof(uint16_t) * m_Indices.size();

    ////リソース作 E E EploadHeap E E
    //D3D12_HEAP_PROPERTIES heapProps2 = {};
    //heapProps2.Type = D3D12_HEAP_TYPE_UPLOAD;
    //heapProps2.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    //heapProps2.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    //heapProps2.CreationNodeMask = 1;
    //heapProps2.VisibleNodeMask = 1;

    //D3D12_RESOURCE_DESC resourceDesc2 = {};
    //resourceDesc2.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    //resourceDesc2.Alignment = 0;
    //resourceDesc2.Width = indexBufferSize;
    //resourceDesc2.Height = 1;
    //resourceDesc2.DepthOrArraySize = 1;
    //resourceDesc2.MipLevels = 1;
    //resourceDesc2.Format = DXGI_FORMAT_UNKNOWN;
    //resourceDesc2.SampleDesc.Count = 1;
    //resourceDesc2.SampleDesc.Quality = 0;
    //resourceDesc2.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    //resourceDesc2.Flags = D3D12_RESOURCE_FLAG_NONE;

    //device->CreateCommittedResource(
    //    &heapProps2,
    //    D3D12_HEAP_FLAG_NONE,
    //    &resourceDesc2,
    //    D3D12_RESOURCE_STATE_GENERIC_READ,
    //    nullptr,
    //    IID_PPV_ARGS(&m_indexBuffer)
    //);

    ////インチE  クスチE EタをバチE  ァにコピ E
    //uint8_t* mappedData2 = nullptr;
    //m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData2));
    //memcpy(mappedData2, m_Indices.data(), indexBufferSize);
    //m_indexBuffer->Unmap(0, nullptr);

    ////インチE  クスバッファビューの設?E
    //m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    //m_indexBufferView.SizeInBytes = indexBufferSize;
    //m_indexBufferView.Format = DXGI_FORMAT_R16_UINT; // uint16_tならこめE


}

void CMesh::Update()
{
}



void CMesh::Draw(CTransform* transform, CMaterial* material, BlendMode blendMode)
{
    // --コマンドリスト
    ID3D12GraphicsCommandList* commandList = CDX12Manager::GetInstance().GetCommandLIst();


    // --行列取得
    DirectX::XMMATRIX world = transform->GetWorld();
    DirectX::XMMATRIX view = CDX12Manager::GetInstance().GetView();
    DirectX::XMMATRIX proj = CDX12Manager::GetInstance().GetProj();


    // --掛け算
    DirectX::XMMATRIX wvp = world * view * proj;

    // --定数バッファ用のデータにセットする
    
    if (blendMode == BlendMode::Additive)
    {
        commandList->SetPipelineState(PSOManager::GetInstance().GetAdditivePSO());
    }
    else
    {
        commandList->SetPipelineState(PSOManager::GetInstance().GetMeshPSO());
    }
    
    commandList->SetGraphicsRootSignature(PSOManager::GetInstance().GetMeshRootSignature());

    //SRVチEEチEEセチE  
    //ID3D12DescriptorHeap* heaps[] = { CDX12Manager::GetInstance().GetSRVHeap() };

    //commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootDescriptorTable(2, m_BoneSrvGpuHandle);


    struct RootConstantsData {
        DirectX::XMMATRIX wvp;
        DirectX::XMFLOAT2 uvOffset;
        DirectX::XMFLOAT2 uvScale;
    };
    
    RootConstantsData rcData;
    rcData.wvp = XMMatrixTranspose(wvp);
    rcData.uvOffset = transform->GetUVOffset();
    rcData.uvScale = transform->GetUVScale();

    commandList->SetGraphicsRoot32BitConstants(
        0,
        20,
        &rcData,
        0
    );

    commandList->SetGraphicsRootDescriptorTable(
        1,
        material->GetGpuHandle()
    );



    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //インチE  クス用に変更
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(static_cast<UINT>(m_Indices.size()), 1, 0, 0, 0);
}

void CMesh::BindBoneSRV(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    ID3D12GraphicsCommandList* commandList = CDX12Manager::GetInstance().GetCommandLIst();
    commandList->SetGraphicsRootDescriptorTable(2, handle);
}

void CMesh::SetVertex(const MeshVertex* vertices, size_t vertexCount, const uint32_t* indices, size_t indexCount)
{

    ID3D12Device* device = CDX12Manager::GetInstance().GetDevice();

    m_Vertices.clear();
    m_Indices.clear();

    m_Vertices.assign(vertices, vertices + vertexCount);
    m_Indices.assign(indices, indices + indexCount);

    //----- 頂点バッファの作 E -----
    //サイズ計?E
    UINT vertexBufferSize = sizeof(MeshVertex) * m_Vertices.size();//型 Eサイズに掛け?E

    //リソース作 E E EploadHeap E E
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

    //頂点チE EタをバチE  ァにコピ E
    void* mappedData = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, m_Vertices.data(), vertexBufferSize);//これ
    m_vertexBuffer->Unmap(0, nullptr);

    //頂点バッファビューの設?E
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;//これ
    m_vertexBufferView.StrideInBytes = sizeof(MeshVertex);//これ


    //----- インチE  クスバッファの作 E -----
    //サイズ計?E
    const UINT indexBufferSize = sizeof(uint32_t) * m_Indices.size();

    //リソース作 E E EploadHeap E E
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

    //インチE  クスチE EタをバチE  ァにコピ E
    uint8_t* mappedData2 = nullptr;
    m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData2));
    memcpy(mappedData2, m_Indices.data(), indexBufferSize);
    m_indexBuffer->Unmap(0, nullptr);

    //インチE  クスバッファビューの設?E
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = indexBufferSize;
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // uint16_tならこめE
}


void CMesh::RegisterOwner(CObject* _Owner)
{
    m_Owner = _Owner;
}
