#include "UIObject.h"
#include "Transform.h"
#include "SpriteRenderer.h"

CUIObject::CUIObject(const std::string& _Name)
    : m_name(_Name)
{
    SetName(_Name);

    // Add components necessary for 2D UI
    AddComponent<CTransform>();
    AddComponent<CSpriteRenderer>();
}

CUIObject::~CUIObject()
{
}

void CUIObject::Init()
{
    for (auto& c : components)
    {
        c->Init();
    }
}

void CUIObject::Update()
{
    // Update logic for UI if any
}

void CUIObject::LateUpdate()
{
}

void CUIObject::Draw()
{
    auto sprite = GetComponent<CSpriteRenderer>();
    if (sprite)
    {
        sprite->Draw();
    }
}

void CUIObject::SetTexture(const std::wstring& filePath)
{
    auto sprite = GetComponent<CSpriteRenderer>();
    if (sprite)
    {
        sprite->SetTexture(filePath);
    }
}

void CUIObject::SetPosition(float x, float y)
{
    auto transform = GetComponent<CTransform>();
    if (transform)
    {
        transform->SetPos({ x, y, 0.0f });
    }
}

void CUIObject::SetSize(float width, float height)
{
    auto sprite = GetComponent<CSpriteRenderer>();
    if (sprite)
    {
        sprite->SetSize(width, height);
    }
}
