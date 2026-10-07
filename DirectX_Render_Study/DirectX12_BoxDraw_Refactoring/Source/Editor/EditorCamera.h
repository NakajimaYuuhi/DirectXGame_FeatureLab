#pragma once
#include <DirectXMath.h>
#include <Windows.h>

class EditorCamera
{
public:
    static EditorCamera& GetInstance();

    void Init();
    void Update(float deltaTime);

    DirectX::XMMATRIX GetViewMatrix() const { return m_viewMatrix; }
    DirectX::XMMATRIX GetProjectionMatrix() const { return m_projMatrix; }
    DirectX::XMFLOAT3 GetPosition() const { return m_position; }
    DirectX::XMFLOAT3 GetFocalPoint() const { return m_focalPoint; }

    DirectX::XMFLOAT3 GetForward() const;
    DirectX::XMFLOAT3 GetRight() const;
    DirectX::XMFLOAT3 GetUp() const;

    float GetYaw() const { return m_yaw; }
    float GetPitch() const { return m_pitch; }

    // フォーカス（選択オブジェクト等のワールド座標を中心にする）
    void FocusOn(const DirectX::XMFLOAT3& targetPos, float distance = 5.0f);

    void SetFov(float fov) { m_fov = fov; UpdateProjection(); }
    float GetFov() const { return m_fov; }

    void SetAspectRatio(float aspect) { m_aspectRatio = aspect; UpdateProjection(); }
    void UpdateProjection();

    bool IsNavigating() const { return m_isNavigating; }

private:
    EditorCamera();
    ~EditorCamera() = default;
    EditorCamera(const EditorCamera&) = delete;
    EditorCamera& operator=(const EditorCamera&) = delete;

    void UpdateViewMatrix();

    DirectX::XMFLOAT3 m_position = { 0.0f, 4.0f, -8.0f };
    DirectX::XMFLOAT3 m_focalPoint = { 0.0f, 1.0f, 0.0f };
    float m_distance = 8.0f;

    float m_pitch = 0.35f; // 約20度見下ろし
    float m_yaw = 0.0f;

    float m_flySpeed = 10.0f;
    float m_fastFlyMultiplier = 2.5f;
    float m_panSpeed = 0.008f;
    float m_orbitSensitivity = 0.005f;
    float m_mouseSensitivity = 0.003f;
    float m_zoomSensitivity = 1.0f;

    float m_fov = DirectX::XM_PIDIV4; // 45度
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearZ = 0.1f;
    float m_farZ = 1000.0f;

    DirectX::XMMATRIX m_viewMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX m_projMatrix = DirectX::XMMatrixIdentity();

    bool m_isNavigating = false;
};
