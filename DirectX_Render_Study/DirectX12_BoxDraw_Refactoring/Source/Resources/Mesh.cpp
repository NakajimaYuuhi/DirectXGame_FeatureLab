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



//TODO:蜷後§蠖｢縺ｮ繝励Μ繝溘ユ繧｣繝・E縲・  轤ｹ繝舌ャ繝輔ぃ繧・E騾壹↓縺励◆縺・
//荳譌ｦ鬆らせ?E  繧貞､悶°繧・E繧後ｋ縺ｮ縺ｯ縲∝ｾ悟屓縺励〒



//鬆らせ繝・ E繧ｿ縺ｮ菴・E
MeshVertex mesh_vertices[] =
{
    //遶区婿?E繧､繝ｳ繝・  繧ｯ繧ｹ)
    // ===== 荳・(Y+) =====
    {{-0.5f,0.5f,-0.5f},{0,1,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{-0.5f,0.5f, 0.5f},{0,1,0},{0,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,0.5f, 0.5f},{0,1,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,0.5f,-0.5f},{0,1,0},{1,1},{0,0,0,0},{1,0,0,0}},
    
    // ===== 荳・(Y-) =====
    {{-0.5f,-0.5f, 0.5f},{0,-1,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f,-0.5f,-0.5f},{0,-1,0},{1,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f,-0.5f},{0,-1,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f, 0.5f},{0,-1,0},{0,0},{0,0,0,0},{1,0,0,0}},

    // ===== 蜑埼擇 (Z-) =====
    {{-0.5f,-0.5f,-0.5f},{0,0,-1},{0,1},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f,-0.5f},{0,0,-1},{0,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f, 0.5f,-0.5f},{0,0,-1},{1,0},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f,-0.5f},{0,0,-1},{1,1},{0,0,0,0},{1,0,0,0}},

    // ===== 閭碁擇 (Z+) =====
    {{-0.5f,-0.5f,0.5f},{0,0,1},{1,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f,-0.5f,0.5f},{0,0,1},{0,1},{0,0,0,0},{1,0,0,0}},
    {{ 0.5f, 0.5f,0.5f},{0,0,1},{0,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f,0.5f},{0,0,1},{1,0},{0,0,0,0},{1,0,0,0}},

    // ===== 蟾ｦ (X-) =====
    {{-0.5f,-0.5f, 0.5f},{-1,0,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f, 0.5f},{-1,0,0},{0,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f, 0.5f,-0.5f},{-1,0,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{-0.5f,-0.5f,-0.5f},{-1,0,0},{1,1},{0,0,0,0},{1,0,0,0}},

    // ===== 蜿ｳ (X+) =====
    {{0.5f,-0.5f,-0.5f},{1,0,0},{0,1},{0,0,0,0},{1,0,0,0}},
    {{0.5f, 0.5f,-0.5f},{1,0,0},{0,0},{0,0,0,0},{1,0,0,0}},
    {{0.5f, 0.5f, 0.5f},{1,0,0},{1,0},{0,0,0,0},{1,0,0,0}},
    {{0.5f,-0.5f, 0.5f},{1,0,0},{1,1},{0,0,0,0},{1,0,0,0}},


};

uint32_t mesh_indices[] =
{
    0,1,2, 0,2,3,        // ?E
    4,5,6, 4,6,7,        // ?E
    8,9,10, 8,10,11,     // 蟾ｦ
    12,13,14, 12,14,15,  // 蜿ｳ
    16,17,18, 16,18,19,  // 荳・
    20,21,22, 20,22,23   // 荳・
};

//




//Initialize繧偵←縺薙°縺ｧ蜻ｼ縺ｶ?E  譛峨ａE
//Initialize繧偵←縺薙°縺ｧ蜻ｼ縺ｶ?E  譛峨ａE
CMesh::CMesh()
{  
    //縺薙％縺ｧ縲・  轤ｹ?E  縲√う繝ｳ繝・  繧ｯ繧ｹ?E  繧偵ョ繝輔か繝ｫ繝医〒繧ｻ繝・  (莉ｮ螳溯｣・
    m_Vertices.assign(std::begin(mesh_vertices), std::end(mesh_vertices));//assign縺ｧ蜈･繧後ｌ繧九ｉ縺励＞
    m_Indices.assign(std::begin(mesh_indices), std::end(mesh_indices));

}

void CMesh::Init()
{

    // m_Transform is no longer used.


    ////----- 繧､繝ｳ繝・  繧ｯ繧ｹ繝舌ャ繝輔ぃ縺ｮ菴・E -----
    ////繧ｵ繧､繧ｺ險・E
    //const UINT indexBufferSize = sizeof(uint16_t) * m_Indices.size();

    ////繝ｪ繧ｽ繝ｼ繧ｹ菴・E E EploadHeap E E
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

    ////繧､繝ｳ繝・  繧ｯ繧ｹ繝・ E繧ｿ繧偵ヰ繝・  繧｡縺ｫ繧ｳ繝・E
    //uint8_t* mappedData2 = nullptr;
    //m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData2));
    //memcpy(mappedData2, m_Indices.data(), indexBufferSize);
    //m_indexBuffer->Unmap(0, nullptr);

    ////繧､繝ｳ繝・  繧ｯ繧ｹ繝舌ャ繝輔ぃ繝薙Η繝ｼ縺ｮ險ｭ?E
    //m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    //m_indexBufferView.SizeInBytes = indexBufferSize;
    //m_indexBufferView.Format = DXGI_FORMAT_R16_UINT; // uint16_t縺ｪ繧峨％繧・


}

void CMesh::Update()
{
}



void CMesh::Draw(CTransform* transform, CMaterial* material, BlendMode blendMode)
{
    // --繧ｳ繝槭Φ繝峨Μ繧ｹ繝・
    ID3D12GraphicsCommandList* commandList = DX12Manager::GetInstance().GetCommandList();


    // --陦悟・蜿門ｾ・
    DirectX::XMMATRIX world = transform->GetWorld();
    DirectX::XMMATRIX view = DX12Manager::GetInstance().GetView();
    DirectX::XMMATRIX proj = DX12Manager::GetInstance().GetProj();


    // --謗帙￠邂・
    DirectX::XMMATRIX wvp = world * view * proj;

    // --螳壽焚繝舌ャ繝輔ぃ逕ｨ縺ｮ繝・・繧ｿ縺ｫ繧ｻ繝・ヨ縺吶ｋ
    
    ID3D12PipelineState* pso = PSOManager::GetInstance().GetPSO(material, PSOManager::GetInstance().GetMeshRootSignature());
    if (pso) 
    { 
        commandList->SetPipelineState(pso); 
    }
    else
    {
        // 取得・コンパイルに失敗した場合は、安全のためデフォルトのPSOを使用する
        commandList->SetPipelineState(PSOManager::GetInstance().GetMeshPSO());
    }
    
    commandList->SetGraphicsRootSignature(PSOManager::GetInstance().GetMeshRootSignature());

    //SRV繝・E繝・E繧ｻ繝・  
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

    //繧､繝ｳ繝・  繧ｯ繧ｹ逕ｨ縺ｫ螟画峩
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

    m_Vertices.clear();
    m_Indices.clear();

    m_Vertices.assign(vertices, vertices + vertexCount);
    m_Indices.assign(indices, indices + indexCount);

    //----- 鬆らせ繝舌ャ繝輔ぃ縺ｮ菴・E -----
    //繧ｵ繧､繧ｺ險・E
    UINT vertexBufferSize = sizeof(MeshVertex) * m_Vertices.size();//蝙・E繧ｵ繧､繧ｺ縺ｫ謗帙￠?E

    //繝ｪ繧ｽ繝ｼ繧ｹ菴・E E EploadHeap E E
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

    //鬆らせ繝・ E繧ｿ繧偵ヰ繝・  繧｡縺ｫ繧ｳ繝・E
    void* mappedData = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, m_Vertices.data(), vertexBufferSize);//縺薙ｌ
    m_vertexBuffer->Unmap(0, nullptr);

    //鬆らせ繝舌ャ繝輔ぃ繝薙Η繝ｼ縺ｮ險ｭ?E
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;//縺薙ｌ
    m_vertexBufferView.StrideInBytes = sizeof(MeshVertex);//縺薙ｌ


    //----- 繧､繝ｳ繝・  繧ｯ繧ｹ繝舌ャ繝輔ぃ縺ｮ菴・E -----
    //繧ｵ繧､繧ｺ險・E
    const UINT indexBufferSize = sizeof(uint32_t) * m_Indices.size();

    //繝ｪ繧ｽ繝ｼ繧ｹ菴・E E EploadHeap E E
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

    //繧､繝ｳ繝・  繧ｯ繧ｹ繝・ E繧ｿ繧偵ヰ繝・  繧｡縺ｫ繧ｳ繝・E
    uint8_t* mappedData2 = nullptr;
    m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData2));
    memcpy(mappedData2, m_Indices.data(), indexBufferSize);
    m_indexBuffer->Unmap(0, nullptr);

    //繧､繝ｳ繝・  繧ｯ繧ｹ繝舌ャ繝輔ぃ繝薙Η繝ｼ縺ｮ險ｭ?E
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = indexBufferSize;
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // uint16_t縺ｪ繧峨％繧・
}


void CMesh::RegisterOwner(CObject* _Owner)
{
    m_Owner = _Owner;
}
