#pragma once

// 各ステートの基底クラス
template <typename T>
class StateBase {
protected:
    T* owner = nullptr;

public:
    virtual ~StateBase() = default;

    // 所有者（コンポーネント等）の参照を保持
    void Setup(T* ownerInstance) {
        owner = ownerInstance;
    }

    // ステート開始時（アニメーション再生や初期化）
    virtual void OnEnter() {}

    // 毎フレーム更新（移動や遷移条件チェック）
    virtual void OnUpdate(float deltaTime) {}

    // ステート終了時（後片付けやフラグ解除）
    virtual void OnExit() {}
};