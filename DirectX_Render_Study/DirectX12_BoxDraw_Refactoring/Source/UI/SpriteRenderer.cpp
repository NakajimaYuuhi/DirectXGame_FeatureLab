#include "SpriteRenderer.h"
#include "Object.h"
#include "Transform.h"
#include "DX12Manager.h"
#include "PSOManager.h"
#include "TextureManager.h"

CSpriteRenderer::CSpriteRenderer() : CComponent("SpriteRenderer")
{
    m_size = { 100.0f, 100.0f }; // default size
    m_color = { 1.0f, 1.0f, 1.0f, 1.0f }; // default color (white)
}

CSpriteRenderer::~CSpriteRenderer()
{
}

void CSpriteRenderer::Init()
{
    CreateBuffers();
}

void CSpriteRenderer::SetTexture(const std::wstring& filePath)
{
    DX12Manager::GetInstance().ForceWait();
    auto allocator = DX12Manager::GetInstance().GetCommandAllocator();
    auto cmdList = DX12Manager::GetInstance().GetCommandList();
    allocator->Reset();
    cmdList->Reset(allocator, nullptr);

    m_texture = TextureManager::GetInstance().GetTexture(
        DX12Manager::GetInstance().GetDevice(),
        cmdList,
        filePath.c_str()
    );

    cmdList->Close();
    ID3D12CommandList* list[] = { cmdList };
    DX12Manager::GetInstance().GetCommandQueue()->ExecuteCommandLists(1, list);
    DX12Manager::GetInstance().ForceWait();
}

void CSpriteRenderer::CreateBuffers()
{
    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();

    // Quad: top-left(0,0) to bottom-right(1,1)
    SpriteVertex vertices[] = {
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
        { { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    };

    uint16_t indices[] = { 0, 1, 2, 2, 1, 3 };

    // Create Vertex Buffer
    UINT vertexBufferSize = sizeof(vertices);
    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = vertexBufferSize;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    device->CreateCommittedResource(
        &heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_vertexBuffer));

    void* mappedData = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, vertices, vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
    m_vertexBufferView.StrideInBytes = sizeof(SpriteVertex);

    // Create Index Buffer
    UINT indexBufferSize = sizeof(indices);
    resDesc.Width = indexBufferSize;

    device->CreateCommittedResource(
        &heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_indexBuffer));

    m_indexBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, indices, indexBufferSize);
    m_indexBuffer->Unmap(0, nullptr);

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = indexBufferSize;
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
}

void CSpriteRenderer::Draw()
{
    if (!m_Owner || !m_texture) return;

    CTransform* transform = m_Owner->GetComponent<CTransform>();
    if (!transform) return;

    auto cmdList = DX12Manager::GetInstance().GetCommandList();
    
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    cmdList->SetDescriptorHeaps(1, heaps);

    // Set Pipeline and Root Signature
    cmdList->SetGraphicsRootSignature(PSOManager::GetInstance().GetSpriteRootSignature());
    cmdList->SetPipelineState(PSOManager::GetInstance().GetSpritePSO());

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    cmdList->IASetIndexBuffer(&m_indexBufferView);

    // Calculate WVP for Orthographic Projection
    // Origin is Top-Left (0,0), Screen Size (1920, 1080)
    DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, 1920.0f, 1080.0f, 0.0f, 0.0f, 1.0f);
    
    DirectX::XMFLOAT3 pos = transform->GetPos();
    // Since UI usually ignores rotation/scale from 3D transform, we just translate and scale by m_size
    DirectX::XMMATRIX world = DirectX::XMMatrixScaling(m_size.x, m_size.y, 1.0f) * DirectX::XMMatrixTranslation(pos.x, pos.y, 0.0f);
    
    DirectX::XMMATRIX wvp = world * proj;
    DirectX::XMMATRIX wvpTranspose = DirectX::XMMatrixTranspose(wvp); // HLSL expects transposed or use column_major

    // Set Root Constants (b0): WVP (16 floats) + Color (4 floats)
    float constants[20];
    memcpy(&constants[0], &wvpTranspose, sizeof(DirectX::XMMATRIX));
    constants[16] = m_color.x;
    constants[17] = m_color.y;
    constants[18] = m_color.z;
    constants[19] = m_color.w;

    cmdList->SetGraphicsRoot32BitConstants(0, 20, constants, 0);

    // Set Descriptor Table (t0)
    cmdList->SetGraphicsRootDescriptorTable(1, m_texture->GetGpuHandle());

    cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}


