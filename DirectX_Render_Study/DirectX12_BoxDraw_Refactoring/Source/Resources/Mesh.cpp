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



//TODO:同じ形のプリミティチEE、E  点バッファめEE通にしたぁE
//?旦頂点?E  を外かめEEれるのは、後回しで



//頂点チE Eタの?EE
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


    ////----- インチE  クスバッファの?EE -----
    ////サイズ?EE
    //const UINT indexBufferSize = sizeof(uint16_t) * m_Indices.size();

    ////リソース?EE E EploadHeap E E
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

    ////インチE  クスチE EタをバチE  ァにコチEE
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
    if (!transform || !material)
    {
        return;
    }

    if (!m_vertexBuffer || !m_indexBuffer || m_Indices.empty())
    {
        return;
    }

    ID3D12GraphicsCommandList* commandList = DX12Manager::GetInstance().GetCommandList();
    if (!commandList) return;

    // SRVディスクリプタヒープを確実にセット
    ID3D12DescriptorHeap* srvHeap = DX12Manager::GetInstance().GetSRVHeap();
    if (srvHeap)
    {
        ID3D12DescriptorHeap* heaps[] = { srvHeap };
        commandList->SetDescriptorHeaps(1, heaps);
    }

    // 行列計算
    DirectX::XMMATRIX world = transform->GetWorld();
    DirectX::XMMATRIX view = DX12Manager::GetInstance().GetView();
    DirectX::XMMATRIX proj = DX12Manager::GetInstance().GetProj();
    DirectX::XMMATRIX wvp = world * view * proj;

    ID3D12PipelineState* pso = PSOManager::GetInstance().GetPSO(material, PSOManager::GetInstance().GetMeshRootSignature());
    if (pso) 
    { 
        commandList->SetPipelineState(pso); 
    }
    else
    {
        commandList->SetPipelineState(PSOManager::GetInstance().GetMeshPSO());
    }
    
    commandList->SetGraphicsRootSignature(PSOManager::GetInstance().GetMeshRootSignature());

    // Root Parameter 0: 32bit Constants (WVP + uvOffset + uvScale)
    struct RootConstantsData {
        DirectX::XMMATRIX wvp;
        DirectX::XMFLOAT2 uvOffset;
        DirectX::XMFLOAT2 uvScale;
    };
    RootConstantsData rcData;
    rcData.wvp = XMMatrixTranspose(wvp);
    rcData.uvOffset = transform->GetUVOffset();
    rcData.uvScale = transform->GetUVScale();

    commandList->SetGraphicsRoot32BitConstants(0, 20, &rcData, 0);

    // Root Parameter 1: Material Texture Descriptor Table
    D3D12_GPU_DESCRIPTOR_HANDLE matHandle = material->GetGpuHandle();
    if (matHandle.ptr != 0)
    {
        commandList->SetGraphicsRootDescriptorTable(1, matHandle);
    }

    // Root Parameter 2: Bone SRV Descriptor Table
    if (m_BoneSrvGpuHandle.ptr != 0)
    {
        commandList->SetGraphicsRootDescriptorTable(2, m_BoneSrvGpuHandle);
    }

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(static_cast<UINT>(m_Indices.size()), 1, 0, 0, 0);
}

void CMesh::BindBoneSRV(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    ID3D12GraphicsCommandList* commandList = DX12Manager::GetInstance().GetCommandList();
    commandList->SetGraphicsRootDescriptorTable(2, handle);
}

void CMesh::SetVertex(const MeshVertex* vertices, size_t vertexCount, const uint32_t* indices, size_t indexCount)
{
    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
    if (!device)
    {
        OutputDebugStringA("[CMesh::SetVertex] Error: DX12 Device is NULL!\n");
        return;
    }

    if (!vertices || vertexCount == 0)
    {
        OutputDebugStringA("[CMesh::SetVertex] Warning: vertices is null or vertexCount is 0, skipping buffer creation.\n");
        return;
    }

    if (!indices || indexCount == 0)
    {
        OutputDebugStringA("[CMesh::SetVertex] Warning: indices is null or indexCount is 0, skipping buffer creation.\n");
        return;
    }

    m_Vertices.clear();
    m_Indices.clear();

    m_Vertices.assign(vertices, vertices + vertexCount);
    m_Indices.assign(indices, indices + indexCount);

    //----- 頂点バッファ生成 -----
    UINT vertexBufferSize = static_cast<UINT>(sizeof(MeshVertex) * m_Vertices.size());

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

    m_vertexBuffer.Reset();
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );

    if (FAILED(hr) || !m_vertexBuffer)
    {
        OutputDebugStringA("[CMesh::SetVertex] Error: Failed to CreateCommittedResource for VertexBuffer!\n");
        return;
    }

    // 頂点データをバッファにコピー
    void* mappedData = nullptr;
    hr = m_vertexBuffer->Map(0, nullptr, &mappedData);
    if (FAILED(hr) || !mappedData)
    {
        OutputDebugStringA("[CMesh::SetVertex] Error: Failed to Map VertexBuffer!\n");
        m_vertexBuffer.Reset();
        return;
    }
    memcpy(mappedData, m_Vertices.data(), vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // 頂点バッファビューの設定
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
    m_vertexBufferView.StrideInBytes = sizeof(MeshVertex);

    //----- インデックスバッファ生成 -----
    const UINT indexBufferSize = static_cast<UINT>(sizeof(uint32_t) * m_Indices.size());

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

    m_indexBuffer.Reset();
    hr = device->CreateCommittedResource(
        &heapProps2,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc2,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)
    );

    if (FAILED(hr) || !m_indexBuffer)
    {
        OutputDebugStringA("[CMesh::SetVertex] Error: Failed to CreateCommittedResource for IndexBuffer!\n");
        return;
    }

    // インデックスデータをバッファにコピー
    uint8_t* mappedData2 = nullptr;
    hr = m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData2));
    if (FAILED(hr) || !mappedData2)
    {
        OutputDebugStringA("[CMesh::SetVertex] Error: Failed to Map IndexBuffer!\n");
        m_indexBuffer.Reset();
        return;
    }
    memcpy(mappedData2, m_Indices.data(), indexBufferSize);
    m_indexBuffer->Unmap(0, nullptr);

    // インデックスバッファビューの設定
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = indexBufferSize;
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void CMesh::RegisterOwner(CObject* _Owner)
{
    m_Owner = _Owner;
}
