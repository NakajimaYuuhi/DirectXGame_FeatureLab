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

    // --- ISelectableの実装 ---
    void OnSelect() override;
    void OnDeselect() override;
    void OnSubmit() override;

    // クリック（決定）時のコールバック設定
    void SetOnClickCallback(std::function<void()> callback);

    // ナビゲーション用のポインタ設定（上下左右）
    void SetNavigation(CUIButton* up, CUIButton* down, CUIButton* left, CUIButton* right);

    // ナビゲーション取得用
    CUIButton* GetSelectOnUp() const { return m_selectOnUp; }
    CUIButton* GetSelectOnDown() const { return m_selectOnDown; }
    CUIButton* GetSelectOnLeft() const { return m_selectOnLeft; }
    CUIButton* GetSelectOnRight() const { return m_selectOnRight; }

private:
    std::function<void()> m_onClickCallback;

    // UnityのExplicitナビゲーションに相当
    CUIButton* m_selectOnUp;
    CUIButton* m_selectOnDown;
    CUIButton* m_selectOnLeft;
    CUIButton* m_selectOnRight;

    // フォーカス状態（描画の切り替えなどに使用）
    bool m_isSelected;
};
