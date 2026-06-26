#include "TextObject.h"
#include "Transform.h"
#include "TextRenderer.h"

TextObject::TextObject(const std::string& _Name)
{
    SetName(_Name);

    // テキストオブジェクトに必要なコンポーネントを追加
    AddComponent<CTransform>();
    AddComponent<CTextRenderer>();
}

TextObject::~TextObject()
{

}

void TextObject::Init()
{
    for (auto& c : components)
    {
        c->Init();
    }
}

void TextObject::Update()
{
}

void TextObject::LateUpdate()
{
}

void TextObject::Draw()
{
    auto textRenderer = GetComponent<CTextRenderer>();
    if (textRenderer)
    {
        textRenderer->Draw();
    }
}

void TextObject::SetText(const std::wstring& text)
{
    auto textRenderer = GetComponent<CTextRenderer>();
    if (textRenderer)
    {
        textRenderer->SetText(text);
    }
}

void TextObject::SetPosition(float x, float y)
{
    auto transform = GetComponent<CTransform>();
    if (transform)
    {
        transform->SetPos({ x, y, 0.0f });
    }
}

void TextObject::SetFontSize(float size)
{
    auto textRenderer = GetComponent<CTextRenderer>();
    if (textRenderer)
    {
        textRenderer->SetFontSize(size);
    }
}

void TextObject::SetColor(const D2D1::ColorF& color)
{
    auto textRenderer = GetComponent<CTextRenderer>();
    if (textRenderer)
    {
        textRenderer->SetColor(color);
    }
}

void TextObject::SetFontFamily(const std::wstring& family)
{
    auto textRenderer = GetComponent<CTextRenderer>();
    if (textRenderer)
    {
        textRenderer->SetFontFamily(family);
    }
}
