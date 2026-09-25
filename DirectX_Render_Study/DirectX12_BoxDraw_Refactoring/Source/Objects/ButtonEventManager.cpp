#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "CUIButton.h"
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

    // ?A??I?u?W?F?N?g????t?H?[?J?X??O??
    if (m_currentSelected)
    {
        m_currentSelected->OnDeselect();
    }

    m_currentSelected = newSelected;

    // ?V?????I?u?W?F?N?g??t?H?[?J?X?????
    if (m_currentSelected)
    {
        m_currentSelected->OnSelect();
    }
}

void ButtonEventManager::Update()
{
    if (!m_currentSelected) return;

    CInputManager& input = CInputManager::GetInstance();

    // ?????t?H?[?J?X??CUIButton??????`?F?b?N????i?r?Q?[?V?????????
    CUIButton* currentBtn = dynamic_cast<CUIButton*>(m_currentSelected);
    if (currentBtn)
    {
        // ?L?[?{?[?h???????i?r?Q?[?V????
        if ((input.IsKeyTrigger(VK_UP)||input.IsKeyTrigger('W')) && currentBtn->GetSelectOnUp())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnUp());
            return; // 1?t???[???????????????????return
        }
        else if ((input.IsKeyTrigger(VK_DOWN)||input.IsKeyTrigger('S')) && currentBtn->GetSelectOnDown())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnDown());
            return;
        }
        else if ((input.IsKeyTrigger(VK_LEFT)||input.IsKeyTrigger('A')) && currentBtn->GetSelectOnLeft())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnLeft());
            return;
        }
        else if ((input.IsKeyTrigger(VK_RIGHT)||input.IsKeyTrigger('D')) && currentBtn->GetSelectOnRight())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnRight());
            return;
        }
    }

    // ???????iEnter?L?[???j
    if (input.IsKeyTrigger(VK_RETURN) || input.IsKeyTrigger(VK_SPACE))
    {
        m_currentSelected->OnSubmit();
    }
}

void ButtonEventManager::ApplyFirstSelected()
{
    if (m_firstSelectedName.empty()) return;
    const auto& objectList = ObjectManager::GetInstance().GetObjectList();
    for (const auto& vec : objectList)
    {
        for (const auto& obj : vec)
        {
            if (!obj || obj->GetIsDestroyed()) continue;
            CObjectInfo* info = obj->GetComponent<CObjectInfo>();
            if (info && info->GetObjectName() == m_firstSelectedName)
            {
                CUIButton* btn = dynamic_cast<CUIButton*>(obj.get());
                if (btn)
                {
                    SetSelectedGameObject(btn);
                    return;
                }
            }
        }
    }
}
