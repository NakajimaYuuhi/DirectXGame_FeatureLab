#include "ButtonEventManager.h"
#include "InputManager.h"
#include "CUIButton.h"
#include <windows.h>

ButtonEventManager& ButtonEventManager::GetInstance()
{
    static ButtonEventManager instance;
    return instance;
}

ButtonEventManager::ButtonEventManager()
    : m_currentSelected(nullptr)
{
}

void ButtonEventManager::SetSelectedGameObject(ISelectable* newSelected)
{
    if (m_currentSelected == newSelected) return;

    // 古いオブジェクトからフォーカスを外す
    if (m_currentSelected)
    {
        m_currentSelected->OnDeselect();
    }

    m_currentSelected = newSelected;

    // 新しいオブジェクトにフォーカスを当てる
    if (m_currentSelected)
    {
        m_currentSelected->OnSelect();
    }
}

void ButtonEventManager::Update()
{
    if (!m_currentSelected) return;

    CInputManager& input = CInputManager::GetInstance();

    // 現在のフォーカスがCUIButtonであるかチェックしてナビゲーションを処理
    CUIButton* currentBtn = dynamic_cast<CUIButton*>(m_currentSelected);
    if (currentBtn)
    {
        // キーボード入力によるナビゲーション
        if (input.IsKeyTrigger(VK_UP) && currentBtn->GetSelectOnUp())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnUp());
            return; // 1フレームで複数移動しないようにreturn
        }
        else if (input.IsKeyTrigger(VK_DOWN) && currentBtn->GetSelectOnDown())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnDown());
            return;
        }
        else if (input.IsKeyTrigger(VK_LEFT) && currentBtn->GetSelectOnLeft())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnLeft());
            return;
        }
        else if (input.IsKeyTrigger(VK_RIGHT) && currentBtn->GetSelectOnRight())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnRight());
            return;
        }
    }

    // 決定処理（Enterキーなど）
    if (input.IsKeyTrigger(VK_RETURN) || input.IsKeyTrigger(VK_SPACE))
    {
        m_currentSelected->OnSubmit();
    }
}
