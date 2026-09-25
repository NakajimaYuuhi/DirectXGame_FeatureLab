#pragma once

#include <memory>
#include "StateBase.h"

// ??p?X?e?[?g?}?V??????N???X
template <typename T>
class StateMachine {
private:
    T* m_owner = nullptr;
    std::shared_ptr<StateBase<T>> m_currentState = nullptr;

public:
    // ???L???n?????????
    StateMachine(T* owner = nullptr) : m_owner(owner) {}
    ~StateMachine() = default;

    // ???L???ÉH??Z?b?g?????
    void SetOwner(T* owner) {
        m_owner = owner;
    }

    // ????????
    void ChangeState(std::shared_ptr<StateBase<T>> newState) 
    {
        // ?I??????
        if (m_currentState) 
        {
            m_currentState->OnExit();
        }

        // State???X
        m_currentState = newState;

        // ??????????
        if (m_currentState) {
            m_currentState->Setup(m_owner);
            m_currentState->OnEnter();
        }
    }

    // ???t???[????X?V
    void OnUpdate(float deltaTime) {
        if (m_currentState) {
            m_currentState->OnUpdate(deltaTime);
        }
    }

    // ?????X?e?[?g?èÔ
    std::shared_ptr<StateBase<T>> GetCurrentState() const {
        return m_currentState;
    }
};