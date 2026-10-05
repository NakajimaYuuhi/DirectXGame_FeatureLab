#pragma once
#include "Source/Engine/Component/Component.h"
#include <DirectXMath.h>
#include <string>
class CTween;

// Anchor preset enum for intuitive UE-like setting
enum class AnchorPreset
{
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    StretchHorizontal,
    StretchVertical,
    StretchAll,
    Custom
};

// Pivot preset enum
enum class PivotPreset
{
    TopLeft,      // (0.0, 0.0)
    TopCenter,    // (0.5, 0.0)
    TopRight,     // (1.0, 0.0)
    MiddleLeft,   // (0.0, 0.5)
    MiddleCenter, // (0.5, 0.5)
    MiddleRight,  // (1.0, 0.5)
    BottomLeft,   // (0.0, 1.0)
    BottomCenter, // (0.5, 1.0)
    BottomRight,  // (1.0, 1.0)
    Custom
};

class CRectTransform : public CComponent
{
public:
    CRectTransform();
    virtual ~CRectTransform();

    // Anchor Settings
    void SetAnchorMin(const DirectX::XMFLOAT2& min);
    void SetAnchorMax(const DirectX::XMFLOAT2& max);
    void SetAnchors(const DirectX::XMFLOAT2& min, const DirectX::XMFLOAT2& max);
    const DirectX::XMFLOAT2& GetAnchorMin() const { return m_anchorMin; }
    const DirectX::XMFLOAT2& GetAnchorMax() const { return m_anchorMax; }

    void SetAnchorPreset(AnchorPreset preset, bool setRecommendedPivot = false);
    AnchorPreset GetCurrentAnchorPreset() const;

    // Pivot Settings
    void SetPivot(const DirectX::XMFLOAT2& pivot);
    void SetPivot(float px, float py);
    void SetPivotPreset(PivotPreset preset);
    const DirectX::XMFLOAT2& GetPivot() const { return m_pivot; }

    // Position & Size Settings
    void SetAnchoredPosition(const DirectX::XMFLOAT2& pos);
    void SetAnchoredPosition(float x, float y);
    const DirectX::XMFLOAT2& GetAnchoredPosition() const { return m_anchoredPosition; }

    void SetSizeDelta(const DirectX::XMFLOAT2& size);
    void SetSizeDelta(float width, float height);
    void SetSize(float width, float height) { SetSizeDelta(width, height); }
    const DirectX::XMFLOAT2& GetSizeDelta() const { return m_sizeDelta; }
    DirectX::XMFLOAT2 GetSize() const { return m_sizeDelta; }

    // 2D Rotation & Scale
    void SetRotationZ(float degrees);
    float GetRotationZ() const { return m_rotationZ; }

    void SetScale(const DirectX::XMFLOAT2& scale);
    void SetScale(float sx, float sy);
    const DirectX::XMFLOAT2& GetScale() const { return m_scale; }

    // Evaluation & Matrix Retrieval (Lazy Evaluation with Dirty Flag)
    void SetDirty() const;
    void UpdateRect() const;

    // Screen-space Rectangle (X, Y, Width, Height)
    DirectX::XMFLOAT4 GetScreenRect() const;

    // World Transformation Matrix for UI rendering
    DirectX::XMMATRIX GetWorldMatrix() const;

    // Center point in screen coordinates (Pivot position)
    DirectX::XMFLOAT2 GetCenterPosition() const;

    // Global Reference Resolution (Default: 1920 x 1080)
    static void SetReferenceResolution(float width, float height);
    static DirectX::XMFLOAT2 GetReferenceResolution();

    // Tween Animation Shortcuts
    CTween* DOAnchorPos(const DirectX::XMFLOAT2& targetPos, float duration);
    CTween* DOAnchorPos(float targetX, float targetY, float duration);
    CTween* DOSize(const DirectX::XMFLOAT2& targetSize, float duration);
    CTween* DOSize(float targetWidth, float targetHeight, float duration);
    CTween* DOScale(const DirectX::XMFLOAT2& targetScale, float duration);
    CTween* DOScale(float targetScaleX, float targetScaleY, float duration);
    CTween* DORotation(float targetDegrees, float duration);
    void KillTweens();

private:
    DirectX::XMFLOAT4 GetParentRect() const;

private:
    DirectX::XMFLOAT2 m_anchorMin = { 0.5f, 0.5f };
    DirectX::XMFLOAT2 m_anchorMax = { 0.5f, 0.5f };
    DirectX::XMFLOAT2 m_pivot = { 0.5f, 0.5f };

    DirectX::XMFLOAT2 m_anchoredPosition = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 m_sizeDelta = { 100.0f, 100.0f };

    float m_rotationZ = 0.0f; // Degrees
    DirectX::XMFLOAT2 m_scale = { 1.0f, 1.0f };

    // Cached evaluation results
    mutable bool m_isDirty = true;
    mutable DirectX::XMFLOAT4 m_screenRect = { 0.0f, 0.0f, 100.0f, 100.0f };
    mutable DirectX::XMFLOAT2 m_centerPos = { 0.0f, 0.0f };
    mutable DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();

    // Default reference resolution
    inline static DirectX::XMFLOAT2 s_referenceResolution = { 1920.0f, 1080.0f };
};
