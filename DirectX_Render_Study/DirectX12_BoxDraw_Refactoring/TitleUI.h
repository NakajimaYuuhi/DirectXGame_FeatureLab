#pragma once
#include "UIObject.h"

class TitleUI : public CUIObject
{
public:
    TitleUI(const std::string& _Name = "TitleUI");
    virtual ~TitleUI();

    virtual void Init() override;
    virtual void Update() override;
    virtual void LateUpdate() override;
    virtual void Draw() override;
};
