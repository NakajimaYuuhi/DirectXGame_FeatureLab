#include "CUIButton.h"
#include "ObjectInfo.h"
#include "SpriteRenderer.h"
#include "ButtonEventManager.h"

CUIButton::CUIButton(const std::string& _Name)
    : CUIObject(_Name)
    , m_selectOnUp(nullptr)
    , m_selectOnDown(nullptr)
    , m_selectOnLeft(nullptr)
    , m_selectOnRight(nullptr)
    , m_isSelected(false)
{
}

CUIButton::~CUIButton()
{
	if (ButtonEventManager::GetInstance().GetSelectedGameObject() == this)
	{
		ButtonEventManager::GetInstance().SetSelectedGameObject(nullptr);
	}
}

void CUIButton::Init()
{
    CUIObject::Init();
    CSpriteRenderer* spriteRenderer = GetComponent<CSpriteRenderer>();
    if (spriteRenderer)
    {
        spriteRenderer->SetColor({ 1.0f, 1.0f, 1.0f, 0.3f });
    }
}

void CUIButton::Update()
{
    CUIObject::Update();
}

void CUIButton::Draw()
{
    CUIObject::Draw();
}

void CUIButton::OnSelect()
{
    m_isSelected = true;
    CSpriteRenderer* spriteRenderer = GetComponent<CSpriteRenderer>();
    if (spriteRenderer)
    {
        spriteRenderer->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    }
}

void CUIButton::OnDeselect()
{
    m_isSelected = false;
    CSpriteRenderer* spriteRenderer = GetComponent<CSpriteRenderer>();
    if (spriteRenderer)
    {
        spriteRenderer->SetColor({ 1.0f, 1.0f, 1.0f, 0.3f });
    }
}

void CUIButton::OnSubmit()
{
    if (m_action != ButtonAction::None)
    {
        ExecuteButtonAction(m_action);
    }
    if (m_onClickCallback)
    {
        m_onClickCallback();
    }
}

void CUIButton::SetOnClickCallback(std::function<void()> callback)
{
    m_onClickCallback = callback;
}

void CUIButton::SetNavigation(CUIButton* up, CUIButton* down, CUIButton* left, CUIButton* right)
{
    m_selectOnUp = up;
    m_selectOnDown = down;
    m_selectOnLeft = left;
    m_selectOnRight = right;

    auto GetName = [](CUIButton* btn) -> std::string {
        if (!btn) return "";
        auto info = btn->GetComponent<CObjectInfo>();
        return info ? info->GetObjectName() : "";
    };

    if (up) m_upName = GetName(up);
    if (down) m_downName = GetName(down);
    if (left) m_leftName = GetName(left);
    if (right) m_rightName = GetName(right);
}
