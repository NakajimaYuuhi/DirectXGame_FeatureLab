#include "CUIButton.h"

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
}

void CUIButton::Init()
{
    CUIObject::Init();
    // 初期化処理
}

void CUIButton::Update()
{
    CUIObject::Update();
    // ボタン固有の更新処理があればここに記述
}

void CUIButton::Draw()
{
    CUIObject::Draw();
    
    // フォーカス状態に応じたハイライト描画などを行う場合はここに記述
    // if (m_isSelected) { ... } else { ... }
}

void CUIButton::OnSelect()
{
    m_isSelected = true;
    // フォーカス時の見た目変更（画像切り替え、色変更など）
}

void CUIButton::OnDeselect()
{
    m_isSelected = false;
    // フォーカスが外れた時の見た目変更
}

void CUIButton::OnSubmit()
{
    // 決定時にコールバックを実行
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
