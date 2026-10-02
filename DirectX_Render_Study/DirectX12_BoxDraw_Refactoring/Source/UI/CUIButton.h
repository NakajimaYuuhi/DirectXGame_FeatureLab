#include "ButtonAction.h"
#pragma once
#include "UIObject.h"
#include "ISelectable.h"
#include <functional>
#include <string>

class CUIButton : public CUIObject, public ISelectable
{
public:
    CUIButton(const std::string& _Name = "UIButton");
    virtual ~CUIButton();

    virtual void Init() override;
    virtual void Update() override;
    virtual void Draw() override;

    // --- ISelectable ---
    void OnSelect() override;
    void OnDeselect() override;
    void OnSubmit() override;

    void SetOnClickCallback(std::function<void()> callback);
    ButtonAction GetAction() const { return m_action; }
    void SetAction(ButtonAction action) { m_action = action; }

    void SetNavigation(CUIButton* up, CUIButton* down, CUIButton* left, CUIButton* right);

    CUIButton* GetSelectOnUp() const { return m_selectOnUp; }
    CUIButton* GetSelectOnDown() const { return m_selectOnDown; }
    CUIButton* GetSelectOnLeft() const { return m_selectOnLeft; }
    CUIButton* GetSelectOnRight() const { return m_selectOnRight; }

    void SetNavigationNames(const std::string& up, const std::string& down, const std::string& left, const std::string& right)
    {
        m_upName = up;
        m_downName = down;
        m_leftName = left;
        m_rightName = right;
    }

    const std::string& GetUpName() const { return m_upName; }
    const std::string& GetDownName() const { return m_downName; }
    const std::string& GetLeftName() const { return m_leftName; }
    const std::string& GetRightName() const { return m_rightName; }

private:
    std::function<void()> m_onClickCallback;
    ButtonAction m_action = ButtonAction::None;

    CUIButton* m_selectOnUp = nullptr;
    CUIButton* m_selectOnDown = nullptr;
    CUIButton* m_selectOnLeft = nullptr;
    CUIButton* m_selectOnRight = nullptr;

    std::string m_upName;
    std::string m_downName;
    std::string m_leftName;
    std::string m_rightName;

    bool m_isSelected = false;
};
