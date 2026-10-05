#pragma once
#include "Tween.h"
#include <vector>
#include <memory>
#include <algorithm>

class TweenManager
{
public:
    static TweenManager& GetInstance()
    {
        static TweenManager instance;
        return instance;
    }

    void Update(float deltaTime);

    // Generic Tween creator
    template <typename T>
    CTween* To(std::function<T()> getter, std::function<void(const T&)> setter, T endValue, float duration, void* target = nullptr)
    {
        auto tween = std::make_unique<CTweenProperty<T>>(getter, setter, endValue, duration);
        if (target)
        {
            // Kill existing tweens on the same target if needed, or allow concurrent
            tween->SetTarget(target);
        }
        CTween* rawPtr = tween.get();
        m_pendingAddTweens.push_back(std::move(tween));
        return rawPtr;
    }

    // Kill all active/pending tweens associated with a target pointer
    void KillTweensOf(void* target);

    // Kill all tweens
    void KillAll();

    // Pause / Resume all
    void PauseAll();
    void ResumeAll();

    size_t GetActiveTweenCount() const { return m_activeTweens.size() + m_pendingAddTweens.size(); }

private:
    TweenManager() = default;
    ~TweenManager() = default;
    TweenManager(const TweenManager&) = delete;
    TweenManager& operator=(const TweenManager&) = delete;

    std::vector<std::unique_ptr<CTween>> m_activeTweens;
    std::vector<std::unique_ptr<CTween>> m_pendingAddTweens;
};

// Global Convenience Namespace
namespace Tween
{
    template <typename T>
    inline CTween* To(std::function<T()> getter, std::function<void(const T&)> setter, T endValue, float duration, void* target = nullptr)
    {
        return TweenManager::GetInstance().To<T>(getter, setter, endValue, duration, target);
    }

    inline void Kill(void* target)
    {
        TweenManager::GetInstance().KillTweensOf(target);
    }

    inline void KillAll()
    {
        TweenManager::GetInstance().KillAll();
    }
}