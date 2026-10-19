#pragma once
#include <windows.h>
#include <Xinput.h>
#include "EngineAPI.h"
#pragma comment(lib, "xinput.lib")

// マウスボタン列挙型
enum class MouseButton
{
    Left = 0,
    Right = 1,
    Middle = 2,
    Count = 3
};

//------------------------------------------------------------------------------
// CInputManager
// キーボード・ゲームパッド・マウス入力を管理するシングルトンクラス
//------------------------------------------------------------------------------
class CInputManager
{
public:
    ENGINE_API static CInputManager& GetInstance();

    // ウィンドウハンドルの初期化
    ENGINE_API void Initialize(HWND hwnd);

    // 毎フレームの更新処理
    ENGINE_API void Update();

    //--------------------------------------
    // キーボード入力
    //--------------------------------------
    ENGINE_API bool IsKeyPress(int key) const;   // 押されているか
    ENGINE_API bool IsKeyTrigger(int key) const; // 押された瞬間
    ENGINE_API bool IsKeyRelease(int key) const; // 離された瞬間

    //--------------------------------------
    // マウス入力
    //--------------------------------------
    ENGINE_API bool IsMousePress(MouseButton button) const;   // ボタンが押されているか
    ENGINE_API bool IsMouseTrigger(MouseButton button) const; // ボタンが押された瞬間
    ENGINE_API bool IsMouseRelease(MouseButton button) const; // ボタンが離された瞬間

    // マウス位置（クライアント領域座標）
    ENGINE_API float GetMouseX() const;
    ENGINE_API float GetMouseY() const;

    // 前フレームからの相対移動量
    ENGINE_API float GetMouseDeltaX() const;
    ENGINE_API float GetMouseDeltaY() const;

    // マウスホイール回転量（正: 前方/ズームイン, 負: 後方/ズームアウト）
    ENGINE_API float GetMouseWheel() const;

    // カーソルの表示/非表示およびロック制御
    ENGINE_API void SetCursorVisible(bool visible);
    bool IsCursorVisible() const { return m_isCursorVisible; }

    ENGINE_API void SetCursorLocked(bool locked);
    bool IsCursorLocked() const { return m_isCursorLocked; }

    // WindowProcからのホイール通知
    ENGINE_API void OnMouseWheel(short delta);

    //--------------------------------------
    // ゲームパッド入力
    //--------------------------------------
    ENGINE_API bool IsPadPress(WORD button) const;
    ENGINE_API bool IsPadTrigger(WORD button) const;
    ENGINE_API bool IsPadRelease(WORD button) const;

    // アナログスティック (-1.0f ~ 1.0f)
    ENGINE_API float GetThumbLX() const;
    ENGINE_API float GetThumbLY() const;

    // トリガー (0 ~ 255)
    ENGINE_API BYTE GetLeftTrigger() const;
    ENGINE_API BYTE GetRightTrigger() const;

    // バイブレーション
    ENGINE_API void SetVibration(WORD leftMotor, WORD rightMotor);

private:
    CInputManager();
    ~CInputManager() = default;

    CInputManager(const CInputManager&) = delete;
    CInputManager& operator=(const CInputManager&) = delete;

    HWND m_hwnd = nullptr;

    // キーボード
    BYTE m_keyTable[256];
    BYTE m_oldKeyTable[256];

    // マウス
    bool m_mouseButtons[static_cast<int>(MouseButton::Count)] = { false, false, false };
    bool m_oldMouseButtons[static_cast<int>(MouseButton::Count)] = { false, false, false };

    POINT m_clientMousePos = { 0, 0 };
    POINT m_prevClientMousePos = { 0, 0 };
    float m_mouseDeltaX = 0.0f;
    float m_mouseDeltaY = 0.0f;
    float m_mouseWheelDelta = 0.0f;
    float m_accumulatedWheelDelta = 0.0f;

    bool m_isCursorVisible = true;
    bool m_isCursorLocked = false;
    bool m_hasInitialMousePos = false;

    // ゲームパッド
    XINPUT_STATE m_state;
    XINPUT_STATE m_oldstate;
    XINPUT_VIBRATION m_vibration;
};
