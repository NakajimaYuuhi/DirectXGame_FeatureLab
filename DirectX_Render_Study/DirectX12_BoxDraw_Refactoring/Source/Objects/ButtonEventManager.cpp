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
    if (!m_currentSelected)
    {
        ApplyFirstSelected();
        if (!m_currentSelected) return;
    }

    CInputManager& input = CInputManager::GetInstance();

    CUIButton* currentBtn = dynamic_cast<CUIButton*>(m_currentSelected);
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
                    if (auto btn = dynamic_cast<CUIButton*>(obj.get())) return btn;
                    if (auto btnComp = obj->GetComponent<ButtonComponent>()) return btnComp;
                }
            }
        }
        return nullptr;
    };

    if (currentBtn)
    {
        if (input.IsKeyTrigger(VK_UP) || input.IsKeyTrigger('W'))
        {
            ISelectable* next = FindSelectableByName(currentBtn->GetUpName());
            if (!next) next = currentBtn->GetSelectOnUp();
            if (next) { SetSelectedGameObject(next); return; }
        }
        else if (input.IsKeyTrigger(VK_DOWN) || input.IsKeyTrigger('S'))
        {
            ISelectable* next = FindSelectableByName(currentBtn->GetDownName());
            if (!next) next = currentBtn->GetSelectOnDown();
            if (next) { SetSelectedGameObject(next); return; }
        }
        else if (input.IsKeyTrigger(VK_LEFT) || input.IsKeyTrigger('A'))
        {
            ISelectable* next = FindSelectableByName(currentBtn->GetLeftName());
            if (!next) next = currentBtn->GetSelectOnLeft();
            if (next) { SetSelectedGameObject(next); return; }
        }
        else if (input.IsKeyTrigger(VK_RIGHT) || input.IsKeyTrigger('D'))
        {
            ISelectable* next = FindSelectableByName(currentBtn->GetRightName());
            if (!next) next = currentBtn->GetSelectOnRight();
            if (next) { SetSelectedGameObject(next); return; }
        }
    }
    else if (currentBtnComp)
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

    // 1. 全ボタンを収集し、初期状態として非選択化（色を薄くする）
    std::vector<ISelectable*> allSelectables;
    ISelectable* targetSelectable = nullptr;

    for (const auto& vec : objectList)
    {
        for (const auto& obj : vec)
        {
            if (!obj || obj->GetIsDestroyed()) continue;

            ISelectable* sel = nullptr;
            CUIButton* btn = dynamic_cast<CUIButton*>(obj.get());
            if (btn) sel = btn;
            else if (auto btnComp = obj->GetComponent<ButtonComponent>()) sel = btnComp;

            if (sel)
            {
                allSelectables.push_back(sel);
                sel->OnDeselect(); // 初期状態として非選択色にする

                CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                if (info && !m_firstSelectedName.empty() && info->GetObjectName() == m_firstSelectedName)
                {
                    targetSelectable = sel;
                }
            }
        }
    }

    // 2. 指定された名前のボタンを選択、見つからなければ先頭のボタンをフォールバック選択
    if (targetSelectable)
    {
        SetSelectedGameObject(targetSelectable);
    }
    else if (!allSelectables.empty())
    {
        SetSelectedGameObject(allSelectables.front());
    }
}
