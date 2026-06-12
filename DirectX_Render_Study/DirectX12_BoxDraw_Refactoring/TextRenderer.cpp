#include "TextRenderer.h"
#include "D2DTextRenderer.h"
#include "Transform.h"
#include "Object.h"

CTextRenderer::CTextRenderer(const std::string& name)
    : CComponent(name)
{
}

void CTextRenderer::Init()
{
}

void CTextRenderer::Draw()
{
    if (!GetIsVarid()) return;

    float drawX = m_x;
    float drawY = m_y;

    if (m_Owner)
    {
        auto transform = m_Owner->GetComponent<CTransform>();
        if (transform)
        {
            const auto& pos = transform->GetPos();
            drawX += pos.x;
            drawY += pos.y;
        }
    }

    D2DTextRenderer::GetInstance().DrawTextStr(m_text, drawX, drawY, m_fontSize, m_color, m_fontFamily);
}
