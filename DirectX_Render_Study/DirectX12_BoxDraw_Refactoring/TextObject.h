#pragma once
#include "Object.h"
#include <string>
#include <d2d1.h>

class TextObject : public CObject
{
public:
    TextObject(const std::string& _Name = "TextObject");
    virtual ~TextObject();

    virtual void Init() override;
    virtual void Update() override;
    virtual void LateUpdate() override;
    virtual void Draw() override;

    // パラメータ設定用のラッパーメソッド
    void SetText(const std::wstring& text);
    void SetPosition(float x, float y);
    void SetFontSize(float size);
    void SetColor(const D2D1::ColorF& color);
    void SetFontFamily(const std::wstring& family);

private:
    std::string m_name;
};
