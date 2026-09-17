#pragma once
#include "Component.h"
#include "Texture.h"
#include "RenderLayer.h"
#include <DirectXMath.h>
#include <memory>
#include <string>

struct SpriteVertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
};

class CSpriteRenderer : public CComponent
{
public:
    CSpriteRenderer();
    virtual ~CSpriteRenderer();

    virtual void Init() override;

    // Render layer
    RenderLayer GetRenderLayer() const { return m_renderLayer; }
    void SetRenderLayer(RenderLayer layer) { m_renderLayer = layer; }

    void SetTexture(const std::wstring& filePath);
    const std::wstring& GetTexturePath() const { return m_texturePath; }
    void SetSize(float width, float height) { m_size = { width, height }; }
    DirectX::XMFLOAT2 GetSize() const { return m_size; }
    void SetColor(const DirectX::XMFLOAT4& color) { m_color = color; }
    const DirectX::XMFLOAT4& GetColor() const { return m_color; }

    void Draw();

private:
    void CreateBuffers();

    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

    std::shared_ptr<CTexture> m_texture;
    std::wstring m_texturePath;
    DirectX::XMFLOAT2 m_size;
    DirectX::XMFLOAT4 m_color;
    RenderLayer m_renderLayer = RenderLayer::UI;
};

