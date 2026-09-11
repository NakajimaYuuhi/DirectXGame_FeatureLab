#pragma once

#include <memory>
#include "StateBase.h"

// 汎用ステートマシン管理クラス
template <typename T>
class StateMachine {
private:
    T* m_owner = nullptr;
    std::shared_ptr<StateBase<T>> m_currentState = nullptr;

public:
    // 所有者を渡して初期化
    StateMachine(T* owner = nullptr) : m_owner(owner) {}
    ~StateMachine() = default;

    // 所有者を後からセットする場合
    void SetOwner(T* owner) {
        m_owner = owner;
    }

    // 状態の切り替え
    void ChangeState(std::shared_ptr<StateBase<T>> newState) 
    {
        // 終了処理
        if (m_currentState) 
        {
            m_currentState->OnExit();
        }

        // Stateの変更
        m_currentState = newState;

        // 初期化処理
        if (m_currentState) {
            m_currentState->Setup(m_owner);
            m_currentState->OnEnter();
        }
    }

    // 毎フレームの更新
    void OnUpdate(float deltaTime) {
        if (m_currentState) {
            m_currentState->OnUpdate(deltaTime);
        }
    }

    // 現在のステート取得
    std::shared_ptr<StateBase<T>> GetCurrentState() const {
        return m_currentState;
    }
};