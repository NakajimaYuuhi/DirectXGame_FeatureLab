#include "InputManager.h"
#include <algorithm>
#pragma comment(lib, "xinput.lib")

//------------------------------------------------------------------------------
// インスタンス取得（シングルトン）
//------------------------------------------------------------------------------
CInputManager& CInputManager::GetInstance()
{
    static CInputManager instance;
    return instance;
}

//------------------------------------------------------------------------------
// コンストラクタ
//------------------------------------------------------------------------------
CInputManager::CInputManager()
{
    ZeroMemory(m_keyTable, sizeof(m_keyTable));
    ZeroMemory(m_oldKeyTable, sizeof(m_oldKeyTable));

    ZeroMemory(m_mouseButtons, sizeof(m_mouseButtons));
    ZeroMemory(m_oldMouseButtons, sizeof(m_oldMouseButtons));

    ZeroMemory(&m_state, sizeof(m_state));
    ZeroMemory(&m_oldstate, sizeof(m_oldstate));
    ZeroMemory(&m_vibration, sizeof(m_vibration));
}

//------------------------------------------------------------------------------
// ウィンドウハンドル設定
//------------------------------------------------------------------------------
void CInputManager::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;
}

//------------------------------------------------------------------------------
// 毎フレームの更新処理
//------------------------------------------------------------------------------
void CInputManager::Update()
{
    //--- キーボード更新 ---
    for (int i = 0; i < 256; ++i)
    {
        m_oldKeyTable[i] = m_keyTable[i];
        m_keyTable[i] = (GetAsyncKeyState(i) & 0x8000) ? 1 : 0;
    }

    //--- マウスボタン更新 ---
    for (int i = 0; i < static_cast<int>(MouseButton::Count); ++i)
    {
        m_oldMouseButtons[i] = m_mouseButtons[i];
    }
    m_mouseButtons[static_cast<int>(MouseButton::Left)]   = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    m_mouseButtons[static_cast<int>(MouseButton::Right)]  = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    m_mouseButtons[static_cast<int>(MouseButton::Middle)] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

    //--- マウス座標 & デルタ更新 ---
    POINT curScreenPos = { 0, 0 };
    GetCursorPos(&curScreenPos);

    bool isWindowActive = (m_hwnd == nullptr) || (GetForegroundWindow() == m_hwnd);

    if (m_hwnd && isWindowActive)
    {
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);
        POINT centerClient = {
            (clientRect.right - clientRect.left) / 2,
            (clientRect.bottom - clientRect.top) / 2
        };

        if (m_isCursorLocked)
        {
            POINT centerScreen = centerClient;
            ClientToScreen(m_hwnd, &centerScreen);

            m_mouseDeltaX = static_cast<float>(curScreenPos.x - centerScreen.x);
            m_mouseDeltaY = static_cast<float>(curScreenPos.y - centerScreen.y);

            SetCursorPos(centerScreen.x, centerScreen.y);
            m_clientMousePos = centerClient;
            m_prevClientMousePos = centerClient;
            m_hasInitialMousePos = true;
        }
        else
        {
            POINT curClientPos = curScreenPos;
            ScreenToClient(m_hwnd, &curClientPos);

            if (!m_hasInitialMousePos)
            {
                m_prevClientMousePos = curClientPos;
                m_hasInitialMousePos = true;
                m_mouseDeltaX = 0.0f;
                m_mouseDeltaY = 0.0f;
            }
            else
            {
                m_mouseDeltaX = static_cast<float>(curClientPos.x - m_prevClientMousePos.x);
                m_mouseDeltaY = static_cast<float>(curClientPos.y - m_prevClientMousePos.y);
                m_prevClientMousePos = curClientPos;
            }
            m_clientMousePos = curClientPos;
        }
    }
    else
    {
        m_mouseDeltaX = 0.0f;
        m_mouseDeltaY = 0.0f;
    }

    //--- ホイール回転量更新 ---
    m_mouseWheelDelta = m_accumulatedWheelDelta;
    m_accumulatedWheelDelta = 0.0f;

    //--- ゲームパッド更新 ---
    m_oldstate = m_state;
    ZeroMemory(&m_state, sizeof(XINPUT_STATE));
    DWORD dwResult = XInputGetState(0, &m_state);
    if (dwResult != ERROR_SUCCESS)
    {
        ZeroMemory(&m_state.Gamepad, sizeof(XINPUT_GAMEPAD));
    }

    // アナログスティック デッドゾーン処理
    if ((m_state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         m_state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
        (m_state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         m_state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
    {
        m_state.Gamepad.sThumbLX = 0;
        m_state.Gamepad.sThumbLY = 0;
    }

    // バイブレーションリセット
    ZeroMemory(&m_vibration, sizeof(XINPUT_VIBRATION));
}

//------------------------------------------------------------------------------
// キーボード入力判定
//------------------------------------------------------------------------------
bool CInputManager::IsKeyPress(int key) const
{
    return m_keyTable[key] != 0;
}

bool CInputManager::IsKeyTrigger(int key) const
{
    return m_keyTable[key] && !m_oldKeyTable[key];
}

bool CInputManager::IsKeyRelease(int key) const
{
    return !m_keyTable[key] && m_oldKeyTable[key];
}

//------------------------------------------------------------------------------
// マウスボタン入力判定
//------------------------------------------------------------------------------
bool CInputManager::IsMousePress(MouseButton button) const
{
    return m_mouseButtons[static_cast<int>(button)];
}

bool CInputManager::IsMouseTrigger(MouseButton button) const
{
    int idx = static_cast<int>(button);
    return m_mouseButtons[idx] && !m_oldMouseButtons[idx];
}

bool CInputManager::IsMouseRelease(MouseButton button) const
{
    int idx = static_cast<int>(button);
    return !m_mouseButtons[idx] && m_oldMouseButtons[idx];
}

float CInputManager::GetMouseX() const
{
    return static_cast<float>(m_clientMousePos.x);
}

float CInputManager::GetMouseY() const
{
    return static_cast<float>(m_clientMousePos.y);
}

float CInputManager::GetMouseDeltaX() const
{
    return m_mouseDeltaX;
}

float CInputManager::GetMouseDeltaY() const
{
    return m_mouseDeltaY;
}

float CInputManager::GetMouseWheel() const
{
    return m_mouseWheelDelta;
}

void CInputManager::SetCursorVisible(bool visible)
{
    if (m_isCursorVisible == visible) return;
    m_isCursorVisible = visible;
    ShowCursor(visible ? TRUE : FALSE);
}

void CInputManager::SetCursorLocked(bool locked)
{
    if (m_isCursorLocked == locked) return;
    m_isCursorLocked = locked;

    if (m_isCursorLocked && m_hwnd)
    {
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);
        POINT centerScreen = {
            (clientRect.right - clientRect.left) / 2,
            (clientRect.bottom - clientRect.top) / 2
        };
        ClientToScreen(m_hwnd, &centerScreen);
        SetCursorPos(centerScreen.x, centerScreen.y);
    }
}

void CInputManager::OnMouseWheel(short delta)
{
    // 120刻みを 1.0f 単位に正規化して累積
    m_accumulatedWheelDelta += static_cast<float>(delta) / 120.0f;
}

//------------------------------------------------------------------------------
// ゲームパッド入力判定
//------------------------------------------------------------------------------
bool CInputManager::IsPadPress(WORD button) const
{
    return (m_state.Gamepad.wButtons & button) != 0;
}

bool CInputManager::IsPadTrigger(WORD button) const
{
    return (m_state.Gamepad.wButtons & button) && !(m_oldstate.Gamepad.wButtons & button);
}

bool CInputManager::IsPadRelease(WORD button) const
{
    return !(m_state.Gamepad.wButtons & button) && (m_oldstate.Gamepad.wButtons & button);
}

float CInputManager::GetThumbLX() const
{
    return m_state.Gamepad.sThumbLX / 32767.0f;
}

float CInputManager::GetThumbLY() const
{
    return m_state.Gamepad.sThumbLY / 32767.0f;
}

BYTE CInputManager::GetLeftTrigger() const
{
    return m_state.Gamepad.bLeftTrigger;
}

BYTE CInputManager::GetRightTrigger() const
{
    return m_state.Gamepad.bRightTrigger;
}

void CInputManager::SetVibration(WORD leftMotor, WORD rightMotor)
{
    m_vibration.wLeftMotorSpeed = leftMotor;
    m_vibration.wRightMotorSpeed = rightMotor;
    XInputSetState(0, &m_vibration);
}
