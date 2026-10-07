#pragma once
#include <windows.h>
#include <Xinput.h>
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
    static CInputManager& GetInstance();

    // ウィンドウハンドルの初期化
    void Initialize(HWND hwnd);

    // 毎フレームの更新処理
    void Update();

    //--------------------------------------
    // キーボード入力
    //--------------------------------------
    bool IsKeyPress(int key) const;   // 押されているか
    bool IsKeyTrigger(int key) const; // 押された瞬間
    bool IsKeyRelease(int key) const; // 離された瞬間

    //--------------------------------------
    // マウス入力
    //--------------------------------------
    bool IsMousePress(MouseButton button) const;   // ボタンが押されているか
    bool IsMouseTrigger(MouseButton button) const; // ボタンが押された瞬間
    bool IsMouseRelease(MouseButton button) const; // ボタンが離された瞬間

    // マウス位置（クライアント領域座標）
    float GetMouseX() const;
    float GetMouseY() const;

    // 前フレームからの相対移動量
    float GetMouseDeltaX() const;
    float GetMouseDeltaY() const;

    // マウスホイール回転量（正: 前方/ズームイン, 負: 後方/ズームアウト）
    float GetMouseWheel() const;

    // カーソルの表示/非表示およびロック制御
    void SetCursorVisible(bool visible);
    bool IsCursorVisible() const { return m_isCursorVisible; }

    void SetCursorLocked(bool locked);
    bool IsCursorLocked() const { return m_isCursorLocked; }

    // WindowProcからのホイール通知
    void OnMouseWheel(short delta);

    //--------------------------------------
    // ゲームパッド入力
    //--------------------------------------
    bool IsPadPress(WORD button) const;
    bool IsPadTrigger(WORD button) const;
    bool IsPadRelease(WORD button) const;

    // アナログスティック (-1.0f ~ 1.0f)
    float GetThumbLX() const;
    float GetThumbLY() const;

    // トリガー (0 ~ 255)
    BYTE GetLeftTrigger() const;
    BYTE GetRightTrigger() const;

    // バイブレーション
    void SetVibration(WORD leftMotor, WORD rightMotor);

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
