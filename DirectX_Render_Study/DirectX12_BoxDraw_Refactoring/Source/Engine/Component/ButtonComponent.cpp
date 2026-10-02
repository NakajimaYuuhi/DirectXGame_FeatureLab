#include "ButtonComponent.h"
#include "SpriteRenderer.h"
#include "Object.h"

ButtonComponent::ButtonComponent()
    : CComponent("ButtonComponent")
{
}

void ButtonComponent::Init()
{
}

void ButtonComponent::Update(float deltaTime)
{
}

void ButtonComponent::OnSelect()
{
    m_isSelected = true;
    if (m_Owner)
    {
        CSpriteRenderer* sprite = m_Owner->GetComponent<CSpriteRenderer>();
        if (sprite)
        {
            sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        }
    }
}

void ButtonComponent::OnDeselect()
{
    m_isSelected = false;
    if (m_Owner)
    {
        CSpriteRenderer* sprite = m_Owner->GetComponent<CSpriteRenderer>();
        if (sprite)
        {
            sprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.3f });
        }
    }
}

void ButtonComponent::OnSubmit()
{
    if (m_onClickCallback)
    {
        m_onClickCallback();
    }
    if (m_action != ButtonAction::None)
    {
        ExecuteButtonAction(m_action);
    }
}
