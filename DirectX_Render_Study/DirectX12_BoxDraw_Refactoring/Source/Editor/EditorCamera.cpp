#include "EditorCamera.h"
#include "InputManager.h"
#include "ViewportUI.h"
#include "InspectorUI.h"
#include "BasicSettings.h"
#include "ObjectManager.h"
#include "Transform.h"
#include "imgui.h"
#include <cmath>
#include <algorithm>

EditorCamera& EditorCamera::GetInstance()
{
    static EditorCamera instance;
    return instance;
}

EditorCamera::EditorCamera()
{
    m_aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
    Init();
}

void EditorCamera::Init()
{
    UpdateProjection();
    UpdateViewMatrix();
}

void EditorCamera::UpdateProjection()
{
    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(
        m_fov,
        m_aspectRatio,
        m_nearZ,
        m_farZ
    );
}

DirectX::XMFLOAT3 EditorCamera::GetForward() const
{
    float cp = cosf(m_pitch);
    float sp = sinf(m_pitch);
    float cy = cosf(m_yaw);
    float sy = sinf(m_yaw);
    return DirectX::XMFLOAT3(-sy * cp, -sp, cy * cp);
}

DirectX::XMFLOAT3 EditorCamera::GetRight() const
{
    float cy = cosf(m_yaw);
    float sy = sinf(m_yaw);
    return DirectX::XMFLOAT3(cy, 0.0f, sy);
}

DirectX::XMFLOAT3 EditorCamera::GetUp() const
{
    DirectX::XMFLOAT3 f = GetForward();
    DirectX::XMFLOAT3 r = GetRight();
    DirectX::XMVECTOR vF = DirectX::XMLoadFloat3(&f);
    DirectX::XMVECTOR vR = DirectX::XMLoadFloat3(&r);
    DirectX::XMVECTOR vU = DirectX::XMVector3Cross(vF, vR);
    DirectX::XMFLOAT3 up;
    DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(vU));
    return up;
}

void EditorCamera::FocusOn(const DirectX::XMFLOAT3& targetPos, float distance)
{
    m_focalPoint = targetPos;
    m_distance = (std::max)(distance, 1.0f);

    DirectX::XMFLOAT3 forward = GetForward();
    m_position = {
        m_focalPoint.x - forward.x * m_distance,
        m_focalPoint.y - forward.y * m_distance,
        m_focalPoint.z - forward.z * m_distance
    };

    UpdateViewMatrix();
}

void EditorCamera::Update(float deltaTime)
{
    bool isEdit = CInspectorUI::GetInstance().IsEditMode() || CInspectorUI::GetInstance().IsPrefabEditMode();
    if (!isEdit)
    {
        m_isNavigating = false;
        return;
    }

    bool isHovered = CViewportUI::GetInstance().IsHovered();
    ImGuiIO& io = ImGui::GetIO();

    bool rightPress = CInputManager::GetInstance().IsMousePress(MouseButton::Right);
    bool middlePress = CInputManager::GetInstance().IsMousePress(MouseButton::Middle);
    bool altPress = CInputManager::GetInstance().IsKeyPress(VK_MENU);
    bool leftPress = CInputManager::GetInstance().IsMousePress(MouseButton::Left);

    bool rightTrigger = CInputManager::GetInstance().IsMouseTrigger(MouseButton::Right);
    bool middleTrigger = CInputManager::GetInstance().IsMouseTrigger(MouseButton::Middle);
    bool leftTrigger = CInputManager::GetInstance().IsMouseTrigger(MouseButton::Left);

    // 操作開始判定（ビューポート内でクリック開始）
    if (isHovered)
    {
        if (rightTrigger || middleTrigger || (altPress && leftTrigger))
        {
            m_isNavigating = true;
        }
    }

    // ボタンがすべて離されたらナビゲーション終了
    if (!rightPress && !middlePress && !(altPress && leftPress))
    {
        m_isNavigating = false;
    }

    float deltaX = CInputManager::GetInstance().GetMouseDeltaX();
    float deltaY = CInputManager::GetInstance().GetMouseDeltaY();

    DirectX::XMFLOAT3 forward = GetForward();
    DirectX::XMFLOAT3 right = GetRight();
    DirectX::XMFLOAT3 up = GetUp();

    if (m_isNavigating)
    {
        if (rightPress)
        {
            // -------------------------------------------------------------
            // 1. フライスルー移動（右ボタンドラッグ + WASD / QE）
            // -------------------------------------------------------------
            m_yaw -= deltaX * m_mouseSensitivity;
            m_pitch += deltaY * m_mouseSensitivity;

            // ピッチ角クランプ (-85度 ~ +85度)
            const float maxPitch = DirectX::XM_PIDIV2 * 0.94f;
            m_pitch = std::clamp(m_pitch, -maxPitch, maxPitch);

            // フライ速度調整（右ドラッグ中にホイール回転）
            float wheel = CInputManager::GetInstance().GetMouseWheel();
            if (wheel != 0.0f)
            {
                m_flySpeed += wheel * 2.0f;
                if (m_flySpeed < 1.0f) m_flySpeed = 1.0f;
                if (m_flySpeed > 100.0f) m_flySpeed = 100.0f;
            }

            // 移動入力（WASD / QE）
            float speed = m_flySpeed;
            if (CInputManager::GetInstance().IsKeyPress(VK_SHIFT))
            {
                speed *= m_fastFlyMultiplier;
            }

            DirectX::XMFLOAT3 move = { 0.0f, 0.0f, 0.0f };
            if (CInputManager::GetInstance().IsKeyPress('W'))
            {
                move.x += forward.x; move.y += forward.y; move.z += forward.z;
            }
            if (CInputManager::GetInstance().IsKeyPress('S'))
            {
                move.x -= forward.x; move.y -= forward.y; move.z -= forward.z;
            }
            if (CInputManager::GetInstance().IsKeyPress('D'))
            {
                move.x += right.x; move.y += right.y; move.z += right.z;
            }
            if (CInputManager::GetInstance().IsKeyPress('A'))
            {
                move.x -= right.x; move.y -= right.y; move.z -= right.z;
            }
            if (CInputManager::GetInstance().IsKeyPress('E'))
            {
                move.y += 1.0f; // 上昇
            }
            if (CInputManager::GetInstance().IsKeyPress('Q'))
            {
                move.y -= 1.0f; // 下降
            }

            float lenSq = move.x * move.x + move.y * move.y + move.z * move.z;
            if (lenSq > 0.0001f)
            {
                float len = sqrtf(lenSq);
                move.x /= len; move.y /= len; move.z /= len;

                m_position.x += move.x * speed * deltaTime;
                m_position.y += move.y * speed * deltaTime;
                m_position.z += move.z * speed * deltaTime;

                // 注視点もカメラ移動に合わせて追従
                m_focalPoint = {
                    m_position.x + forward.x * m_distance,
                    m_position.y + forward.y * m_distance,
                    m_position.z + forward.z * m_distance
                };
            }
        }
        else if (middlePress)
        {
            // -------------------------------------------------------------
            // 2. パン移動（中ボタンドラッグ）
            // -------------------------------------------------------------
            float factor = m_panSpeed * (m_distance * 0.1f + 1.0f);
            float panX = deltaX * factor;
            float panY = deltaY * factor;

            m_position.x -= right.x * panX - up.x * panY;
            m_position.y -= right.y * panX - up.y * panY;
            m_position.z -= right.z * panX - up.z * panY;

            m_focalPoint.x -= right.x * panX - up.x * panY;
            m_focalPoint.y -= right.y * panX - up.y * panY;
            m_focalPoint.z -= right.z * panX - up.z * panY;
        }
        else if (altPress && leftPress)
        {
            // -------------------------------------------------------------
            // 3. オービット周回（Alt + 左ドラッグ）
            // -------------------------------------------------------------
            m_yaw -= deltaX * m_orbitSensitivity;
            m_pitch += deltaY * m_orbitSensitivity;

            const float maxPitch = DirectX::XM_PIDIV2 * 0.94f;
            m_pitch = std::clamp(m_pitch, -maxPitch, maxPitch);

            DirectX::XMFLOAT3 updatedForward = GetForward();
            m_position = {
                m_focalPoint.x - updatedForward.x * m_distance,
                m_focalPoint.y - updatedForward.y * m_distance,
                m_focalPoint.z - updatedForward.z * m_distance
            };
        }
    }

    // -------------------------------------------------------------
    // 4. マウスホイールによるドリー / ズーム（単体操作）
    // -------------------------------------------------------------
    if (isHovered && !rightPress)
    {
        float wheel = CInputManager::GetInstance().GetMouseWheel();
        if (wheel != 0.0f)
        {
            m_distance -= wheel * m_zoomSensitivity * (m_distance * 0.1f + 0.5f);
            if (m_distance < 0.5f) m_distance = 0.5f;

            DirectX::XMFLOAT3 currentForward = GetForward();
            m_position = {
                m_focalPoint.x - currentForward.x * m_distance,
                m_focalPoint.y - currentForward.y * m_distance,
                m_focalPoint.z - currentForward.z * m_distance
            };
        }
    }

    // -------------------------------------------------------------
    // 5. Fキーで選択中オブジェクトへのフォーカス
    // -------------------------------------------------------------
    if (isHovered && !io.WantTextInput && CInputManager::GetInstance().IsKeyTrigger('F'))
    {
        CObject* selectedObj = nullptr;
        if (CInspectorUI::GetInstance().IsPrefabEditMode())
        {
            selectedObj = CInspectorUI::GetInstance().GetPrefabEditTarget();
        }
        else
        {
            int tagIdx = CInspectorUI::GetInstance().GetSelectedTagIndex();
            int objIdx = CInspectorUI::GetInstance().GetSelectedObjectIndex();
            const auto& list = ObjectManager::GetInstance().GetObjectList();
            if (tagIdx >= 0 && tagIdx < static_cast<int>(list.size()) &&
                objIdx >= 0 && objIdx < static_cast<int>(list[tagIdx].size()))
            {
                selectedObj = list[tagIdx][objIdx].get();
            }
        }

        if (selectedObj)
        {
            CTransform* trans = selectedObj->GetComponent<CTransform>();
            if (trans)
            {
                FocusOn(trans->GetPos(), 6.0f);
            }
        }
    }

    UpdateViewMatrix();
}

void EditorCamera::UpdateViewMatrix()
{
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);
    DirectX::XMFLOAT3 f = GetForward();
    DirectX::XMVECTOR forwardVec = DirectX::XMLoadFloat3(&f);
    DirectX::XMVECTOR target = DirectX::XMVectorAdd(eye, forwardVec);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_viewMatrix = DirectX::XMMatrixLookAtLH(eye, target, up);
}
