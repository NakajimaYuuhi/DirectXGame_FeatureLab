#include "ObjectManager.h"
#include "ObjectInfo.h"
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
    if (!m_currentSelected)
    {
        ApplyFirstSelected();
        if (!m_currentSelected) return;
    }

    CInputManager& input = CInputManager::GetInstance();

    ButtonComponent* currentBtnComp = dynamic_cast<ButtonComponent*>(m_currentSelected);

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
                    if (auto btnComp = obj->GetComponent<ButtonComponent>()) return btnComp;
                }
            }
        }
        return nullptr;
    };

    if (currentBtnComp)
    {
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
    const auto& objectList = ObjectManager::GetInstance().GetObjectList();

    std::vector<ISelectable*> allSelectables;
    ISelectable* targetSelectable = nullptr;

    for (const auto& vec : objectList)
    {
        for (const auto& obj : vec)
        {
            if (!obj || obj->GetIsDestroyed()) continue;

            ISelectable* sel = nullptr;
            if (auto btnComp = obj->GetComponent<ButtonComponent>()) sel = btnComp;

            if (sel)
            {
                allSelectables.push_back(sel);
                sel->OnDeselect();

                CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                if (info && !m_firstSelectedName.empty() && info->GetObjectName() == m_firstSelectedName)
                {
                    targetSelectable = sel;
                }
            }
        }
    }

    if (targetSelectable)
    {
        SetSelectedGameObject(targetSelectable);
    }
    else if (!allSelectables.empty())
    {
        SetSelectedGameObject(allSelectables.front());
    }
}
