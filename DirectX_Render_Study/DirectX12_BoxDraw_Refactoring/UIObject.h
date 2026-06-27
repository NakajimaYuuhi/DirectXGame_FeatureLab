#pragma once
#include "Object.h"
#include <string>

class CUIObject : public CObject
{
public:
    CUIObject(const std::string& _Name = "UI");
    virtual ~CUIObject();

    virtual void Init() override;
    virtual void Update() override;
    virtual void LateUpdate() override;
    virtual void Draw() override;

    void SetTexture(const std::wstring& filePath);
    void SetPosition(float x, float y);
    void SetSize(float width, float height);

private:
};
