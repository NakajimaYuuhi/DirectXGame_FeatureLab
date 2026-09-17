#pragma once
#include "Component.h"
#include "RenderLayer.h"
#include <string>
#include <d2d1.h>

class CTextRenderer : public CComponent
{
public:
    CTextRenderer(const std::string& name = "TextRenderer");
    virtual ~CTextRenderer() = default;

    virtual void Init() override;

    // Draw text (registers to D2DTextRenderer)
    void Draw();

    // Render layer
    RenderLayer GetRenderLayer() const { return m_renderLayer; }
    void SetRenderLayer(RenderLayer layer) { m_renderLayer = layer; }

    // Getters and setters
    void SetText(const std::wstring& text) { m_text = text; }
    void SetPosition(float x, float y) { m_x = x; m_y = y; }
    void SetFontSize(float size) { m_fontSize = size; }
    void SetColor(const D2D1::ColorF& color) { m_color = color; }
    void SetFontFamily(const std::wstring& family) { m_fontFamily = family; }

    const std::wstring& GetText() const { return m_text; }
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetFontSize() const { return m_fontSize; }
    D2D1::ColorF GetColor() const { return m_color; }
    const std::wstring& GetFontFamily() const { return m_fontFamily; }

private:
    std::wstring m_text = L"Default Text";
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_fontSize = 24.0f;
    D2D1::ColorF m_color = D2D1::ColorF::White;
    std::wstring m_fontFamily = L"Meiryo";
    RenderLayer m_renderLayer = RenderLayer::UI;
};
