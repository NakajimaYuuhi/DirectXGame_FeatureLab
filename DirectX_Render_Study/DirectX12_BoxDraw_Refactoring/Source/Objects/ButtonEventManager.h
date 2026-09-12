#pragma once
#include "ISelectable.h"
#include <string>

class ButtonEventManager
{
public:
    static ButtonEventManager& GetInstance();

    void SetSelectedGameObject(ISelectable* newSelected);
    ISelectable* GetSelectedGameObject() const { return m_currentSelected; }

    void Update();
    void ClearSelectedGameObject() { m_currentSelected = nullptr; }

    void SetFirstSelectedName(const std::string& name) { m_firstSelectedName = name; }
    const std::string& GetFirstSelectedName() const { return m_firstSelectedName; }
    void ApplyFirstSelected();

private:
    ButtonEventManager();
    ~ButtonEventManager() = default;
    ButtonEventManager(const ButtonEventManager&) = delete;
    ButtonEventManager& operator=(const ButtonEventManager&) = delete;

    ISelectable* m_currentSelected = nullptr;
    std::string m_firstSelectedName;
};
