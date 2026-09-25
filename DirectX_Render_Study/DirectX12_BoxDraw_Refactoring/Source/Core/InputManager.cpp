#include "InputManager.h"
#include <algorithm>
#pragma comment(lib, "xinput.lib")
//------------------------------------------------------------------------------
// ?C???X?^???X?èÔ?i?B???C???X?^???X?????j
//------------------------------------------------------------------------------
CInputManager& CInputManager::GetInstance()
{
    static CInputManager instance;
    return instance;
}

//------------------------------------------------------------------------------
// ?R???X?g???N?^
//------------------------------------------------------------------------------
CInputManager::CInputManager()
{
    // ?L?[????z????????
    ZeroMemory(m_keyTable, sizeof(m_keyTable));
    ZeroMemory(m_oldKeyTable, sizeof(m_oldKeyTable));

    // ?Q?[???p?b?h????????????
    ZeroMemory(&m_state, sizeof(m_state));
    ZeroMemory(&m_oldstate, sizeof(m_oldstate));

    // ?U??????????i??~???j
    ZeroMemory(&m_vibration, sizeof(m_vibration));
}

//------------------------------------------------------------------------------
// ???t???[?????X?V????
// ?L?[?{?[?h??Q?[???p?b?h?????èÔ??????
//------------------------------------------------------------------------------
void CInputManager::Update()
{
    //--- ?L?[?{?[?h?X?V ---
    for (int i = 0; i < 256; ++i)
    {
        // ?O?t???[?????????
        m_oldKeyTable[i] = m_keyTable[i];

        // ?????L?[????èÔ?i????????????1?A??????????????0?j
        m_keyTable[i] = (GetAsyncKeyState(i) & 0x8000) ? 1 : 0;
    }

    //????t???[???????????L?[????
    m_oldstate = m_state;

    //--- ?Q?[???p?b?h?X?V ---

    ZeroMemory(&m_state, sizeof(XINPUT_STATE));
    DWORD dwResult = XInputGetState(0, &m_state); // ?v???C???[1?????????
    if (dwResult != ERROR_SUCCESS)
    {
        // ??????????????0
        ZeroMemory(&m_state.Gamepad, sizeof(XINPUT_GAMEPAD));
    }

  

    // ?A?i???O?X?e?B?b?N??f?b?h?]?[??????
    if ((m_state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
        m_state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
        (m_state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
            m_state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
    {
        // ??????????0????????
        m_state.Gamepad.sThumbLX = 0;
        m_state.Gamepad.sThumbLY = 0;
    }

    //--- ?U?????Z?b?g ---
    ZeroMemory(&m_vibration, sizeof(XINPUT_VIBRATION));
}

//------------------------------------------------------------------------------
// ?L?[?{?[?h??????
//------------------------------------------------------------------------------
bool CInputManager::IsKeyPress(int key) const
{
    return m_keyTable[key]; // ??????????
}

bool CInputManager::IsKeyTrigger(int key) const
{
    return m_keyTable[key] && !m_oldKeyTable[key]; // ???????u??
}

bool CInputManager::IsKeyRelease(int key) const
{
    return !m_keyTable[key] && m_oldKeyTable[key]; // ???????u??
}

//------------------------------------------------------------------------------
// ?Q?[???p?b?h????
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

// ?A?i???O?X?e?B?b?N (-1.0f ~ 1.0f)
float CInputManager::GetThumbLX() const
{
    return m_state.Gamepad.sThumbLX / 32767.0f;
}

float CInputManager::GetThumbLY() const
{
    return m_state.Gamepad.sThumbLY / 32767.0f;
}

// ?g???K?[???? (0~255)
BYTE CInputManager::GetLeftTrigger() const
{
    return m_state.Gamepad.bLeftTrigger;
}

BYTE CInputManager::GetRightTrigger() const
{
    return m_state.Gamepad.bRightTrigger;
}

//------------------------------------------------------------------------------
// ?Q?[???p?b?h?U?????
// leftMotor, rightMotor = 0~65535
//------------------------------------------------------------------------------
void CInputManager::SetVibration(WORD leftMotor, WORD rightMotor)
{
    m_vibration.wLeftMotorSpeed = leftMotor;
    m_vibration.wRightMotorSpeed = rightMotor;
    XInputSetState(0, &m_vibration);
}
