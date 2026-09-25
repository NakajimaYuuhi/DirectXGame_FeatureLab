#include "ButtonAction.h"
#pragma once
#include "UIObject.h"
#include "ISelectable.h"
#include <functional>

class CUIButton : public CUIObject, public ISelectable
{
public:
    CUIButton(const std::string& _Name = "UIButton");
    virtual ~CUIButton();

    virtual void Init() override;
    virtual void Update() override;
    virtual void Draw() override;

    // --- ISelectable????? ---
    void OnSelect() override;
    void OnDeselect() override;
    void OnSubmit() override;

    // ?N???b?N?i????j????R?[???o?b?N???
    void SetOnClickCallback(std::function<void()> callback);
    ButtonAction GetAction() const { return m_action; }
    void SetAction(ButtonAction action) { m_action = action; }

    // ?i?r?Q?[?V?????p??|?C???^???i?????E?j
    void SetNavigation(CUIButton* up, CUIButton* down, CUIButton* left, CUIButton* right);

    // ?i?r?Q?[?V?????èÔ?p
    CUIButton* GetSelectOnUp() const { return m_selectOnUp; }
    CUIButton* GetSelectOnDown() const { return m_selectOnDown; }
    CUIButton* GetSelectOnLeft() const { return m_selectOnLeft; }
    CUIButton* GetSelectOnRight() const { return m_selectOnRight; }

private:
    std::function<void()> m_onClickCallback;
    ButtonAction m_action = ButtonAction::None;

    // Unity??Explicit?i?r?Q?[?V?????????
    CUIButton* m_selectOnUp;
    CUIButton* m_selectOnDown;
    CUIButton* m_selectOnLeft;
    CUIButton* m_selectOnRight;

    // ?t?H?[?J?X???i?`???????????g?p?j
    bool m_isSelected;
};
