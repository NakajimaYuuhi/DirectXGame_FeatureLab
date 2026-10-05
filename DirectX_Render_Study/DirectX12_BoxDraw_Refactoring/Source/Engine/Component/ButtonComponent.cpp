#include "ButtonComponent.h"
#include "SpriteRenderer.h"
#include "Object.h"
#include "Source/UI/RectTransform.h"
#include "Source/Util/Tween.h"

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
        // 1. Sprite fade in
        if (auto sprite = m_Owner->GetComponent<CSpriteRenderer>())
        {
            sprite->DOFade(1.0f, 0.2f)->SetEase(Ease::OutQuad);
        }

        // 2. RectTransform pop-up scale (center pivot)
        if (auto rect = m_Owner->GetComponent<CRectTransform>())
        {
            rect->DOScale({ 1.1f, 1.1f }, 0.25f)->SetEase(Ease::OutBack);
        }
    }
}

void ButtonComponent::OnDeselect()
{
    m_isSelected = false;
    if (m_Owner)
    {
        // 1. Sprite dim out
        if (auto sprite = m_Owner->GetComponent<CSpriteRenderer>())
        {
            sprite->DOFade(0.4f, 0.2f)->SetEase(Ease::OutQuad);
        }

        // 2. RectTransform return to normal scale
        if (auto rect = m_Owner->GetComponent<CRectTransform>())
        {
            rect->DOScale({ 1.0f, 1.0f }, 0.2f)->SetEase(Ease::OutQuad);
        }
    }
}

void ButtonComponent::OnSubmit()
{
    if (m_Owner)
    {
        // Click feedback punch
        if (auto rect = m_Owner->GetComponent<CRectTransform>())
        {
            rect->DOScale({ 0.93f, 0.93f }, 0.08f)
                ->SetEase(Ease::InOutQuad)
                ->SetLoops(2, LoopType::Yoyo);
        }
    }

    if (m_onClickCallback)
    {
        m_onClickCallback();
    }
    if (m_action != ButtonAction::None)
    {
        ExecuteButtonAction(m_action);
    }
}