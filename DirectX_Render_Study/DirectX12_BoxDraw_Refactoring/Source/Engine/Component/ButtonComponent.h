#pragma once
#include "Component.h"
#include "ISelectable.h"
#include "ButtonAction.h"
#include <functional>
#include <string>

class ButtonComponent : public CComponent, public ISelectable
{
public:
    ButtonComponent();
    virtual ~ButtonComponent() = default;

    virtual void Init() override;
    virtual void Update(float deltaTime) override;

    // --- ISelectable Interface ---
    virtual void OnSelect() override;
    virtual void OnDeselect() override;
    virtual void OnSubmit() override;

    // Callbacks & Actions
    void SetOnClickCallback(std::function<void()> callback) { m_onClickCallback = callback; }
    ButtonAction GetAction() const { return m_action; }
    void SetAction(ButtonAction action) { m_action = action; }

    // Navigation target names
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

    bool IsSelected() const { return m_isSelected; }

private:
    std::function<void()> m_onClickCallback;
    ButtonAction m_action = ButtonAction::None;

    std::string m_upName;
    std::string m_downName;
    std::string m_leftName;
    std::string m_rightName;

    bool m_isSelected = false;
};
