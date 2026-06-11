#pragma once
#include "Component.h"
#include "Texture.h"
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

    void SetTexture(const std::wstring& filePath);
    void SetSize(float width, float height) { m_size = { width, height }; }
    DirectX::XMFLOAT2 GetSize() const { return m_size; }
    void SetColor(const DirectX::XMFLOAT4& color) { m_color = color; }

    void Draw();

private:
    void CreateBuffers();

    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    std::shared_ptr<CTexture> m_texture;
    DirectX::XMFLOAT2 m_size;
    DirectX::XMFLOAT4 m_color;
};
