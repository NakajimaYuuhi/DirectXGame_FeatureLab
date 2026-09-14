#include "CUIButton.h"

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
    // ??????????
    // SpriteRenderer
    CSpriteRenderer * spriteRenderer = GetComponent<CSpriteRenderer>();

    spriteRenderer->SetColor({ 1.0f,1.0f,1.0f,0.3f });
}

void CUIButton::Update()
{
    CUIObject::Update();
    // ?{?^????L??X?V????????????????L?q
}

void CUIButton::Draw()
{
    CUIObject::Draw();
    
    // ?t?H?[?J?X??????????n?C???C?g?`?????s???????????L?q
    // if (m_isSelected) { ... } else { ... }
}

void CUIButton::OnSelect()
{
    m_isSelected = true;
    // ?t?H?[?J?X??????????X?i????????A?F??X???j

        //SpriteRenderer
    CSpriteRenderer* spriteRenderer = GetComponent<CSpriteRenderer>();

    spriteRenderer->SetColor({ 1.0f,1.0f,1.0f,1.0f });

}

void CUIButton::OnDeselect()
{
    m_isSelected = false;
    // ?t?H?[?J?X???O????????????X

    //SpriteRenderer
    CSpriteRenderer* spriteRenderer = GetComponent<CSpriteRenderer>();

    spriteRenderer->SetColor({ 1.0f,1.0f,1.0f,0.3f });

}

void CUIButton::OnSubmit()
{
    if (m_action != ButtonAction::None)
    {
        ExecuteButtonAction(m_action);
    }
    // ???ŠÎ??R?[???o?b?N????s
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
}

