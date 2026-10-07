#include "Material.h"
#include "DX12Manager.h"
#include "TextureManager.h"
#include "PSOManager.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

// 256 bytes alignment helper
static inline size_t Align256(size_t size)
{
    return (size + 255) & ~255;
}

// -----------------------------------------------------------------------------
// Constructor / Destructor
// -----------------------------------------------------------------------------
CMaterial::CMaterial()
    : m_ShaderFile(L"Assets/Shader/Triangle.hlsl")
    , m_VsEntry("VSMain")
    , m_PsEntry("PSMain")
    , m_BlendMode(BlendMode::Opaque)
    , m_CullMode(D3D12_CULL_MODE_BACK)
    , m_isDirty(true)
{
    CreateConstantBuffer();
}

CMaterial::CMaterial(wstring _FilePath, XMFLOAT4 _Color, wstring shaderFile, string vsEntry, string psEntry, BlendMode blendMode)
    : m_ShaderFile(shaderFile)
    , m_VsEntry(vsEntry)
    , m_PsEntry(psEntry)
    , m_BlendMode(blendMode)
    , m_CullMode(D3D12_CULL_MODE_BACK)
    , m_isDirty(true)
{
    m_Data.baseColor = _Color;
    CreateConstantBuffer();

    if (!_FilePath.empty())
    {
        LoadTexture(_FilePath);
    }
}

CMaterial::~CMaterial()
{
    if (m_constantBuffer && m_mappedData)
    {
        m_constantBuffer->Unmap(0, nullptr);
        m_mappedData = nullptr;
    }
}

// -----------------------------------------------------------------------------
// Constant Buffer creation
// -----------------------------------------------------------------------------
void CMaterial::CreateConstantBuffer()
{
    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
    if (!device) return;

    size_t bufferSize = Align256(sizeof(MaterialBufferData));

    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resDesc{};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = bufferSize;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)
    );

    if (SUCCEEDED(hr) && m_constantBuffer)
    {
        D3D12_RANGE readRange{ 0, 0 };
        m_constantBuffer->Map(0, &readRange, &m_mappedData);
        m_isDirty = true;
        UpdateBuffer();
    }
}

// -----------------------------------------------------------------------------
// Clone
// -----------------------------------------------------------------------------
std::shared_ptr<CMaterial> CMaterial::Clone() const
{
    auto clone = std::make_shared<CMaterial>();
    clone->m_Data = this->m_Data;
    clone->m_ShaderFile = this->m_ShaderFile;
    clone->m_VsEntry = this->m_VsEntry;
    clone->m_PsEntry = this->m_PsEntry;
    clone->m_BlendMode = this->m_BlendMode;
    clone->m_CullMode = this->m_CullMode;
    clone->m_Texture = this->m_Texture;
    clone->m_TextureFilePath = this->m_TextureFilePath;
    clone->m_isDirty = true;
    clone->UpdateBuffer();
    return clone;
}

// -----------------------------------------------------------------------------
// Texture Load / Set
// -----------------------------------------------------------------------------
void CMaterial::LoadTexture(wstring _FilePath)
{
    m_TextureFilePath = _FilePath;
    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
    ID3D12GraphicsCommandList* cmdList = DX12Manager::GetInstance().GetCommandList();
    if (!device || !cmdList) return;

    DX12Manager::GetInstance().ForceWait();
    DX12Manager::GetInstance().GetCommandAllocator()->Reset();
    cmdList->Reset(DX12Manager::GetInstance().GetCommandAllocator(), nullptr);

    m_Texture = TextureManager::GetInstance().GetTexture(device, cmdList, _FilePath.c_str());

    cmdList->Close();
    ID3D12CommandList* list[] = { cmdList };
    DX12Manager::GetInstance().GetCommandQueue()->ExecuteCommandLists(1, list);
    DX12Manager::GetInstance().ForceWait();
}

void CMaterial::SetTexture(pTexture texture, const wstring& filePath)
{
    m_Texture = texture;
    m_TextureFilePath = filePath;
}

// -----------------------------------------------------------------------------
// Handles & GPU address
// -----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE CMaterial::GetGpuHandle()
{
    if (m_Texture)
    {
        return m_Texture->GetGpuHandle();
    }
    D3D12_GPU_DESCRIPTOR_HANDLE handle{};
    handle.ptr = 0;
    return handle;
}

D3D12_GPU_VIRTUAL_ADDRESS CMaterial::GetConstantBufferGPUAddress()
{
    if (m_constantBuffer)
    {
        return m_constantBuffer->GetGPUVirtualAddress();
    }
    return 0;
}

// -----------------------------------------------------------------------------
// Parameters
// -----------------------------------------------------------------------------
void CMaterial::SetCustomParam(int vectorIndex, int elementIndex, float value)
{
    if (vectorIndex >= 0 && vectorIndex < 2 && elementIndex >= 0 && elementIndex < 4)
    {
        float* p = reinterpret_cast<float*>(&m_Data.customParams[vectorIndex]);
        p[elementIndex] = value;
        m_isDirty = true;
    }
}

float CMaterial::GetCustomParam(int vectorIndex, int elementIndex) const
{
    if (vectorIndex >= 0 && vectorIndex < 2 && elementIndex >= 0 && elementIndex < 4)
    {
        const float* p = reinterpret_cast<const float*>(&m_Data.customParams[vectorIndex]);
        return p[elementIndex];
    }
    return 0.0f;
}

// -----------------------------------------------------------------------------
// Update & Bind
// -----------------------------------------------------------------------------
void CMaterial::UpdateBuffer()
{
    if (m_isDirty && m_mappedData)
    {
        memcpy(m_mappedData, &m_Data, sizeof(MaterialBufferData));
        m_isDirty = false;
    }
}

void CMaterial::Bind(ID3D12GraphicsCommandList* cmdList, UINT rootParameterIndexCBV, UINT rootParameterIndexSRV)
{
    if (!cmdList) return;

    UpdateBuffer();

    if (m_constantBuffer)
    {
        cmdList->SetGraphicsRootConstantBufferView(rootParameterIndexCBV, m_constantBuffer->GetGPUVirtualAddress());
    }

    if (m_Texture)
    {
        cmdList->SetGraphicsRootDescriptorTable(rootParameterIndexSRV, m_Texture->GetGpuHandle());
    }
}

// -----------------------------------------------------------------------------
// JSON Save / Load
// -----------------------------------------------------------------------------
static std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
    return str;
}

static std::wstring StringToWString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

bool CMaterial::SaveToFile(const std::string& filePath)
{
    try
    {
        json j;
        j["shaderFile"] = WStringToString(m_ShaderFile);
        j["vsEntry"] = m_VsEntry;
        j["psEntry"] = m_PsEntry;
        j["blendMode"] = static_cast<int>(m_BlendMode);
        j["cullMode"] = static_cast<int>(m_CullMode);
        j["textureFile"] = WStringToString(m_TextureFilePath);

        j["baseColor"] = { m_Data.baseColor.x, m_Data.baseColor.y, m_Data.baseColor.z, m_Data.baseColor.w };
        j["uvTiling"] = { m_Data.uvTiling.x, m_Data.uvTiling.y };
        j["uvOffset"] = { m_Data.uvOffset.x, m_Data.uvOffset.y };
        j["roughness"] = m_Data.roughness;
        j["metallic"] = m_Data.metallic;

        j["customParams"][0] = { m_Data.customParams[0].x, m_Data.customParams[0].y, m_Data.customParams[0].z, m_Data.customParams[0].w };
        j["customParams"][1] = { m_Data.customParams[1].x, m_Data.customParams[1].y, m_Data.customParams[1].z, m_Data.customParams[1].w };

        std::ofstream file(filePath);
        if (!file.is_open()) return false;
        file << j.dump(4);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool CMaterial::LoadFromFile(const std::string& filePath)
{
    try
    {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        json j;
        file >> j;

        if (j.contains("shaderFile")) m_ShaderFile = StringToWString(j["shaderFile"].get<std::string>());
        if (j.contains("vsEntry")) m_VsEntry = j["vsEntry"].get<std::string>();
        if (j.contains("psEntry")) m_PsEntry = j["psEntry"].get<std::string>();
        if (j.contains("blendMode")) m_BlendMode = static_cast<BlendMode>(j["blendMode"].get<int>());
        if (j.contains("cullMode")) m_CullMode = static_cast<D3D12_CULL_MODE>(j["cullMode"].get<int>());

        if (j.contains("baseColor"))
        {
            auto col = j["baseColor"];
            m_Data.baseColor = { col[0], col[1], col[2], col[3] };
        }
        if (j.contains("uvTiling"))
        {
            auto t = j["uvTiling"];
            m_Data.uvTiling = { t[0], t[1] };
        }
        if (j.contains("uvOffset"))
        {
            auto o = j["uvOffset"];
            m_Data.uvOffset = { o[0], o[1] };
        }
        if (j.contains("roughness")) m_Data.roughness = j["roughness"].get<float>();
        if (j.contains("metallic")) m_Data.metallic = j["metallic"].get<float>();

        if (j.contains("customParams"))
        {
            auto cp = j["customParams"];
            if (cp.size() > 0) m_Data.customParams[0] = { cp[0][0], cp[0][1], cp[0][2], cp[0][3] };
            if (cp.size() > 1) m_Data.customParams[1] = { cp[1][0], cp[1][1], cp[1][2], cp[1][3] };
        }

        m_isDirty = true;
        UpdateBuffer();

        if (j.contains("textureFile"))
        {
            std::string texPath = j["textureFile"].get<std::string>();
            if (!texPath.empty())
            {
                LoadTexture(StringToWString(texPath));
            }
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}
