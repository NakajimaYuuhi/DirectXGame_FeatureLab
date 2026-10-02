#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "CUIButton.h"
#include "ButtonComponent.h"
#include "ButtonEventManager.h"
#include "InputManager.h"
#include "InspectorUI.h"
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

    if (m_currentSelected)
    {
        m_currentSelected->OnDeselect();
    }

    m_currentSelected = newSelected;

    if (m_currentSelected)
    {
        m_currentSelected->OnSelect();
    }
}

void ButtonEventManager::Update()
{
    if (!CInspectorUI::GetInstance().ShouldUpdateGame()) return;
    if (!m_currentSelected) return;

    CInputManager& input = CInputManager::GetInstance();

    CUIButton* currentBtn = dynamic_cast<CUIButton*>(m_currentSelected);
    ButtonComponent* currentBtnComp = dynamic_cast<ButtonComponent*>(m_currentSelected);

    if (currentBtn)
    {
        if ((input.IsKeyTrigger(VK_UP)||input.IsKeyTrigger('W')) && currentBtn->GetSelectOnUp())
        {
            SetSelectedGameObject(currentBtn->GetSelectOnUp());
            return;
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
    else if (currentBtnComp)
    {
        auto FindSelectableByName = [](const std::string& name) -> ISelectable* {
            if (name.empty()) return nullptr;
            const auto& objectList = ObjectManager::GetInstance().GetObjectList();
            for (const auto& vec : objectList)
            {
                for (const auto& obj : vec)
                {
                    if (!obj || obj->GetIsDestroyed()) continue;
                    CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                    if (info && info->GetObjectName() == name)
                    {
                        if (auto btn = dynamic_cast<CUIButton*>(obj.get())) return btn;
                        if (auto btnComp = obj->GetComponent<ButtonComponent>()) return btnComp;
                    }
                }
            }
            return nullptr;
        };

        if (input.IsKeyTrigger(VK_UP) || input.IsKeyTrigger('W'))
        {
            if (auto next = FindSelectableByName(currentBtnComp->GetUpName())) { SetSelectedGameObject(next); return; }
        }
        else if (input.IsKeyTrigger(VK_DOWN) || input.IsKeyTrigger('S'))
        {
            if (auto next = FindSelectableByName(currentBtnComp->GetDownName())) { SetSelectedGameObject(next); return; }
        }
        else if (input.IsKeyTrigger(VK_LEFT) || input.IsKeyTrigger('A'))
        {
            if (auto next = FindSelectableByName(currentBtnComp->GetLeftName())) { SetSelectedGameObject(next); return; }
        }
        else if (input.IsKeyTrigger(VK_RIGHT) || input.IsKeyTrigger('D'))
        {
            if (auto next = FindSelectableByName(currentBtnComp->GetRightName())) { SetSelectedGameObject(next); return; }
        }
    }

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
                ButtonComponent* btnComp = obj->GetComponent<ButtonComponent>();
                if (btnComp)
                {
                    SetSelectedGameObject(btnComp);
                    return;
                }
            }
        }
    }
}
