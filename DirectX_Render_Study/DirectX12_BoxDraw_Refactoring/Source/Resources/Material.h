#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <memory>
#include "StringAlias.h"
#include "SmartPtrAlias.h"
#include "Texture.h"
#include "BasicSettings.h"

// Alias
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMFLOAT2 = DirectX::XMFLOAT2;
using Color = XMFLOAT4;
using pTexture = SharedPtr<CTexture>;

// Material constant buffer structure
struct MaterialBufferData
{
    XMFLOAT4 baseColor;
    XMFLOAT2 uvTiling;
    XMFLOAT2 uvOffset;
    float    roughness;
    float    metallic;
    float    padding[2];
    XMFLOAT4 customParams[2];

    MaterialBufferData()
        : baseColor(1.0f, 1.0f, 1.0f, 1.0f)
        , uvTiling(1.0f, 1.0f)
        , uvOffset(0.0f, 0.0f)
        , roughness(0.5f)
        , metallic(0.0f)
        , padding{ 0.0f, 0.0f }
        , customParams{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
    {
    }
};

class CMaterial : public std::enable_shared_from_this<CMaterial>
{
public:
    CMaterial();
    CMaterial(wstring _FilePath, XMFLOAT4 _Color, wstring shaderFile = L"Assets/Shader/Triangle.hlsl", string vsEntry = "VSMain", string psEntry = "PSMain", BlendMode blendMode = BlendMode::Opaque);
    ~CMaterial();

    std::shared_ptr<CMaterial> Clone() const;

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle();
    D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUAddress();

    void LoadTexture(wstring _FilePath);
    void SetTexture(pTexture texture, const wstring& filePath = L"");
    pTexture GetTexture() const { return m_Texture; }
    const wstring& GetTextureFilePath() const { return m_TextureFilePath; }

    const wstring& GetShaderFile() const { return m_ShaderFile; }
    const string& GetVsEntry() const { return m_VsEntry; }
    const string& GetPsEntry() const { return m_PsEntry; }
    BlendMode GetBlendMode() const { return m_BlendMode; }
    D3D12_CULL_MODE GetCullMode() const { return m_CullMode; }

    void SetShader(wstring shaderFile, string vsEntry = "VSMain", string psEntry = "PSMain")
    {
        m_ShaderFile = shaderFile;
        m_VsEntry = vsEntry;
        m_PsEntry = psEntry;
    }

    void SetBlendMode(BlendMode blendMode) { m_BlendMode = blendMode; }
    void SetCullMode(D3D12_CULL_MODE cullMode) { m_CullMode = cullMode; }

    MaterialBufferData& GetData() { m_isDirty = true; return m_Data; }
    const MaterialBufferData& GetData() const { return m_Data; }

    void SetColor(const XMFLOAT4& color) { m_Data.baseColor = color; m_isDirty = true; }
    const XMFLOAT4& GetColor() const { return m_Data.baseColor; }

    void SetUVTiling(const XMFLOAT2& tiling) { m_Data.uvTiling = tiling; m_isDirty = true; }
    const XMFLOAT2& GetUVTiling() const { return m_Data.uvTiling; }

    void SetUVOffset(const XMFLOAT2& offset) { m_Data.uvOffset = offset; m_isDirty = true; }
    const XMFLOAT2& GetUVOffset() const { return m_Data.uvOffset; }

    void SetRoughness(float roughness) { m_Data.roughness = roughness; m_isDirty = true; }
    float GetRoughness() const { return m_Data.roughness; }

    void SetMetallic(float metallic) { m_Data.metallic = metallic; m_isDirty = true; }
    float GetMetallic() const { return m_Data.metallic; }

    void SetCustomParam(int vectorIndex, int elementIndex, float value);
    float GetCustomParam(int vectorIndex, int elementIndex) const;

    void UpdateBuffer();
    void Bind(ID3D12GraphicsCommandList* cmdList, UINT rootParameterIndexCBV, UINT rootParameterIndexSRV);

    bool SaveToFile(const std::string& filePath);
    bool LoadFromFile(const std::string& filePath);

private:
    void CreateConstantBuffer();

private:
    pTexture m_Texture;
    wstring  m_TextureFilePath;

    MaterialBufferData m_Data;

    wstring         m_ShaderFile;
    string          m_VsEntry;
    string          m_PsEntry;
    BlendMode       m_BlendMode;
    D3D12_CULL_MODE m_CullMode;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
    void* m_mappedData = nullptr;
    bool  m_isDirty = true;
};
