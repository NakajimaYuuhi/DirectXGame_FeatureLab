#include "RectTransform.h"
#include "Source/Objects/Object.h"
#include "Source/Engine/Component/Transform.h"
#include "Source/Util/TweenManager.h"
#include <cmath>

using namespace DirectX;

CRectTransform::CRectTransform()
    : CComponent("RectTransform")
{
    m_anchorMin = { 0.5f, 0.5f };
    m_anchorMax = { 0.5f, 0.5f };
    m_pivot = { 0.5f, 0.5f };
    m_anchoredPosition = { 0.0f, 0.0f };
    m_sizeDelta = { 100.0f, 100.0f };
    m_rotationZ = 0.0f;
    m_scale = { 1.0f, 1.0f };
    m_isDirty = true;
}
CRectTransform::~CRectTransform()
{
    KillTweens();
}


void CRectTransform::SetAnchorMin(const XMFLOAT2& min)
{
    m_anchorMin = min;
    SetDirty();
}

void CRectTransform::SetAnchorMax(const XMFLOAT2& max)
{
    m_anchorMax = max;
    SetDirty();
}

void CRectTransform::SetAnchors(const XMFLOAT2& min, const XMFLOAT2& max)
{
    m_anchorMin = min;
    m_anchorMax = max;
    SetDirty();
}

void CRectTransform::SetAnchorPreset(AnchorPreset preset, bool setRecommendedPivot)
{
    switch (preset)
    {
    case AnchorPreset::TopLeft:
        m_anchorMin = { 0.0f, 0.0f };
        m_anchorMax = { 0.0f, 0.0f };
        if (setRecommendedPivot) m_pivot = { 0.0f, 0.0f };
        break;
    case AnchorPreset::TopCenter:
        m_anchorMin = { 0.5f, 0.0f };
        m_anchorMax = { 0.5f, 0.0f };
        if (setRecommendedPivot) m_pivot = { 0.5f, 0.0f };
        break;
    case AnchorPreset::TopRight:
        m_anchorMin = { 1.0f, 0.0f };
        m_anchorMax = { 1.0f, 0.0f };
        if (setRecommendedPivot) m_pivot = { 1.0f, 0.0f };
        break;
    case AnchorPreset::MiddleLeft:
        m_anchorMin = { 0.0f, 0.5f };
        m_anchorMax = { 0.0f, 0.5f };
        if (setRecommendedPivot) m_pivot = { 0.0f, 0.5f };
        break;
    case AnchorPreset::MiddleCenter:
        m_anchorMin = { 0.5f, 0.5f };
        m_anchorMax = { 0.5f, 0.5f };
        if (setRecommendedPivot) m_pivot = { 0.5f, 0.5f };
        break;
    case AnchorPreset::MiddleRight:
        m_anchorMin = { 1.0f, 0.5f };
        m_anchorMax = { 1.0f, 0.5f };
        if (setRecommendedPivot) m_pivot = { 1.0f, 0.5f };
        break;
    case AnchorPreset::BottomLeft:
        m_anchorMin = { 0.0f, 1.0f };
        m_anchorMax = { 0.0f, 1.0f };
        if (setRecommendedPivot) m_pivot = { 0.0f, 1.0f };
        break;
    case AnchorPreset::BottomCenter:
        m_anchorMin = { 0.5f, 1.0f };
        m_anchorMax = { 0.5f, 1.0f };
        if (setRecommendedPivot) m_pivot = { 0.5f, 1.0f };
        break;
    case AnchorPreset::BottomRight:
        m_anchorMin = { 1.0f, 1.0f };
        m_anchorMax = { 1.0f, 1.0f };
        if (setRecommendedPivot) m_pivot = { 1.0f, 1.0f };
        break;
    case AnchorPreset::StretchHorizontal:
        m_anchorMin = { 0.0f, 0.5f };
        m_anchorMax = { 1.0f, 0.5f };
        if (setRecommendedPivot) m_pivot = { 0.5f, 0.5f };
        break;
    case AnchorPreset::StretchVertical:
        m_anchorMin = { 0.5f, 0.0f };
        m_anchorMax = { 0.5f, 1.0f };
        if (setRecommendedPivot) m_pivot = { 0.5f, 0.5f };
        break;
    case AnchorPreset::StretchAll:
        m_anchorMin = { 0.0f, 0.0f };
        m_anchorMax = { 1.0f, 1.0f };
        if (setRecommendedPivot) m_pivot = { 0.5f, 0.5f };
        break;
    default:
        break;
    }
    SetDirty();
}

AnchorPreset CRectTransform::GetCurrentAnchorPreset() const
{
    auto isNear = [](float a, float b) { return std::abs(a - b) < 0.001f; };

    if (isNear(m_anchorMin.x, 0.0f) && isNear(m_anchorMin.y, 0.0f) && isNear(m_anchorMax.x, 0.0f) && isNear(m_anchorMax.y, 0.0f)) return AnchorPreset::TopLeft;
    if (isNear(m_anchorMin.x, 0.5f) && isNear(m_anchorMin.y, 0.0f) && isNear(m_anchorMax.x, 0.5f) && isNear(m_anchorMax.y, 0.0f)) return AnchorPreset::TopCenter;
    if (isNear(m_anchorMin.x, 1.0f) && isNear(m_anchorMin.y, 0.0f) && isNear(m_anchorMax.x, 1.0f) && isNear(m_anchorMax.y, 0.0f)) return AnchorPreset::TopRight;

    if (isNear(m_anchorMin.x, 0.0f) && isNear(m_anchorMin.y, 0.5f) && isNear(m_anchorMax.x, 0.0f) && isNear(m_anchorMax.y, 0.5f)) return AnchorPreset::MiddleLeft;
    if (isNear(m_anchorMin.x, 0.5f) && isNear(m_anchorMin.y, 0.5f) && isNear(m_anchorMax.x, 0.5f) && isNear(m_anchorMax.y, 0.5f)) return AnchorPreset::MiddleCenter;
    if (isNear(m_anchorMin.x, 1.0f) && isNear(m_anchorMin.y, 0.5f) && isNear(m_anchorMax.x, 1.0f) && isNear(m_anchorMax.y, 0.5f)) return AnchorPreset::MiddleRight;

    if (isNear(m_anchorMin.x, 0.0f) && isNear(m_anchorMin.y, 1.0f) && isNear(m_anchorMax.x, 0.0f) && isNear(m_anchorMax.y, 1.0f)) return AnchorPreset::BottomLeft;
    if (isNear(m_anchorMin.x, 0.5f) && isNear(m_anchorMin.y, 1.0f) && isNear(m_anchorMax.x, 0.5f) && isNear(m_anchorMax.y, 1.0f)) return AnchorPreset::BottomCenter;
    if (isNear(m_anchorMin.x, 1.0f) && isNear(m_anchorMin.y, 1.0f) && isNear(m_anchorMax.x, 1.0f) && isNear(m_anchorMax.y, 1.0f)) return AnchorPreset::BottomRight;

    if (isNear(m_anchorMin.x, 0.0f) && isNear(m_anchorMin.y, 0.5f) && isNear(m_anchorMax.x, 1.0f) && isNear(m_anchorMax.y, 0.5f)) return AnchorPreset::StretchHorizontal;
    if (isNear(m_anchorMin.x, 0.5f) && isNear(m_anchorMin.y, 0.0f) && isNear(m_anchorMax.x, 0.5f) && isNear(m_anchorMax.y, 1.0f)) return AnchorPreset::StretchVertical;
    if (isNear(m_anchorMin.x, 0.0f) && isNear(m_anchorMin.y, 0.0f) && isNear(m_anchorMax.x, 1.0f) && isNear(m_anchorMax.y, 1.0f)) return AnchorPreset::StretchAll;

    return AnchorPreset::Custom;
}

void CRectTransform::SetPivot(const XMFLOAT2& pivot)
{
    m_pivot = pivot;
    SetDirty();
}

void CRectTransform::SetPivot(float px, float py)
{
    m_pivot = { px, py };
    SetDirty();
}

void CRectTransform::SetPivotPreset(PivotPreset preset)
{
    switch (preset)
    {
    case PivotPreset::TopLeft:      m_pivot = { 0.0f, 0.0f }; break;
    case PivotPreset::TopCenter:    m_pivot = { 0.5f, 0.0f }; break;
    case PivotPreset::TopRight:     m_pivot = { 1.0f, 0.0f }; break;
    case PivotPreset::MiddleLeft:   m_pivot = { 0.0f, 0.5f }; break;
    case PivotPreset::MiddleCenter: m_pivot = { 0.5f, 0.5f }; break;
    case PivotPreset::MiddleRight:  m_pivot = { 1.0f, 0.5f }; break;
    case PivotPreset::BottomLeft:   m_pivot = { 0.0f, 1.0f }; break;
    case PivotPreset::BottomCenter: m_pivot = { 0.5f, 1.0f }; break;
    case PivotPreset::BottomRight:  m_pivot = { 1.0f, 1.0f }; break;
    default: break;
    }
    SetDirty();
}

void CRectTransform::SetAnchoredPosition(const XMFLOAT2& pos)
{
    m_anchoredPosition = pos;
    SetDirty();
}

void CRectTransform::SetAnchoredPosition(float x, float y)
{
    m_anchoredPosition = { x, y };
    SetDirty();
}

void CRectTransform::SetSizeDelta(const XMFLOAT2& size)
{
    m_sizeDelta = size;
    SetDirty();
}

void CRectTransform::SetSizeDelta(float width, float height)
{
    m_sizeDelta = { width, height };
    SetDirty();
}

void CRectTransform::SetRotationZ(float degrees)
{
    m_rotationZ = degrees;
    SetDirty();
}

void CRectTransform::SetScale(const XMFLOAT2& scale)
{
    m_scale = scale;
    SetDirty();
}

void CRectTransform::SetScale(float sx, float sy)
{
    m_scale = { sx, sy };
    SetDirty();
}

void CRectTransform::SetDirty() const
{
    if (m_isDirty) return;
    m_isDirty = true;

    // Propagate dirty flag to children
    if (auto owner = GetOwner())
    {
        if (auto transform = owner->GetTransform())
        {
            for (auto* childTransform : transform->GetChildren())
            {
                if (childTransform)
                {
                    if (auto childOwner = childTransform->GetOwner())
                    {
                        if (auto childRect = childOwner->GetComponent<CRectTransform>())
                        {
                            childRect->SetDirty();
                        }
                    }
                }
            }
        }
    }
}

DirectX::XMFLOAT4 CRectTransform::GetParentRect() const
{
    if (auto owner = GetOwner())
    {
        if (auto transform = owner->GetTransform())
        {
            if (auto parentTransform = transform->GetParent())
            {
                if (auto parentOwner = parentTransform->GetOwner())
                {
                    if (auto parentRect = parentOwner->GetComponent<CRectTransform>())
                    {
                        return parentRect->GetScreenRect();
                    }
                }
            }
        }
    }

    // Default: Root screen space
    return { 0.0f, 0.0f, s_referenceResolution.x, s_referenceResolution.y };
}

void CRectTransform::UpdateRect() const
{
    if (!m_isDirty) return;

    XMFLOAT4 parentRect = GetParentRect();
    float px = parentRect.x;
    float py = parentRect.y;
    float pw = parentRect.z;
    float ph = parentRect.w;

    // Anchor points in screen coordinates
    float aMinX = px + pw * m_anchorMin.x;
    float aMinY = py + ph * m_anchorMin.y;
    float aMaxX = px + pw * m_anchorMax.x;
    float aMaxY = py + ph * m_anchorMax.y;

    // Anchor dimension
    float aWidth  = aMaxX - aMinX;
    float aHeight = aMaxY - aMinY;

    // Final dimension including size delta and local scale
    float baseWidth   = aWidth  + m_sizeDelta.x;
    float baseHeight  = aHeight + m_sizeDelta.y;
    float finalWidth  = baseWidth  * m_scale.x;
    float finalHeight = baseHeight * m_scale.y;

    // Anchor center reference
    float anchorCenterX = (aMinX + aMaxX) * 0.5f;
    float anchorCenterY = (aMinY + aMaxY) * 0.5f;

    // Center position (Pivot position in screen space)
    m_centerPos.x = anchorCenterX + m_anchoredPosition.x;
    m_centerPos.y = anchorCenterY + m_anchoredPosition.y;

    // Top-Left corner in screen space
    float left = m_centerPos.x - finalWidth  * m_pivot.x;
    float top  = m_centerPos.y - finalHeight * m_pivot.y;

    m_screenRect = { left, top, finalWidth, finalHeight };

    // Matrix calculation for rendering
    // Pivot offset (local unit quad [0,1] shifted so pivot is at origin)
    XMMATRIX matPivot = XMMatrixTranslation(-m_pivot.x, -m_pivot.y, 0.0f);
    XMMATRIX matScale = XMMatrixScaling(finalWidth, finalHeight, 1.0f);
    XMMATRIX matRot   = XMMatrixRotationZ(XMConvertToRadians(m_rotationZ));
    XMMATRIX matTrans = XMMatrixTranslation(m_centerPos.x, m_centerPos.y, 0.0f);

    m_worldMatrix = matPivot * matScale * matRot * matTrans;

    m_isDirty = false;
}

DirectX::XMFLOAT4 CRectTransform::GetScreenRect() const
{
    if (m_isDirty) UpdateRect();
    return m_screenRect;
}

DirectX::XMMATRIX CRectTransform::GetWorldMatrix() const
{
    if (m_isDirty) UpdateRect();
    return m_worldMatrix;
}

DirectX::XMFLOAT2 CRectTransform::GetCenterPosition() const
{
    if (m_isDirty) UpdateRect();
    return m_centerPos;
}

void CRectTransform::SetReferenceResolution(float width, float height)
{
    s_referenceResolution = { width, height };
}

DirectX::XMFLOAT2 CRectTransform::GetReferenceResolution()
{
    return s_referenceResolution;
}

// -------------------------------------------------------------
// Tween Animation Shortcuts
// -------------------------------------------------------------
CTween* CRectTransform::DOAnchorPos(const XMFLOAT2& targetPos, float duration)
{
    return Tween::To<XMFLOAT2>(
        [this]() { return GetAnchoredPosition(); },
        [this](const XMFLOAT2& val) { SetAnchoredPosition(val); },
        targetPos, duration, this
    );
}

CTween* CRectTransform::DOAnchorPos(float targetX, float targetY, float duration)
{
    return DOAnchorPos(XMFLOAT2(targetX, targetY), duration);
}

CTween* CRectTransform::DOSize(const XMFLOAT2& targetSize, float duration)
{
    return Tween::To<XMFLOAT2>(
        [this]() { return GetSizeDelta(); },
        [this](const XMFLOAT2& val) { SetSizeDelta(val); },
        targetSize, duration, this
    );
}

CTween* CRectTransform::DOSize(float targetWidth, float targetHeight, float duration)
{
    return DOSize(XMFLOAT2(targetWidth, targetHeight), duration);
}

CTween* CRectTransform::DOScale(const XMFLOAT2& targetScale, float duration)
{
    return Tween::To<XMFLOAT2>(
        [this]() { return GetScale(); },
        [this](const XMFLOAT2& val) { SetScale(val); },
        targetScale, duration, this
    );
}

CTween* CRectTransform::DOScale(float targetScaleX, float targetScaleY, float duration)
{
    return DOScale(XMFLOAT2(targetScaleX, targetScaleY), duration);
}

CTween* CRectTransform::DORotation(float targetDegrees, float duration)
{
    return Tween::To<float>(
        [this]() { return GetRotationZ(); },
        [this](float val) { SetRotationZ(val); },
        targetDegrees, duration, this
    );
}

void CRectTransform::KillTweens()
{
    Tween::Kill(this);
}
