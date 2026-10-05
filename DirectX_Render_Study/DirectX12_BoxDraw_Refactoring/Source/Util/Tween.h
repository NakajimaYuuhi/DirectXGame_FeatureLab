#pragma once
#include "Ease.h"
#include <functional>
#include <memory>
#include <DirectXMath.h>

enum class LoopType
{
    Restart,
    Yoyo
};

// Base Tween Class
class CTween
{
public:
    virtual ~CTween() = default;

    virtual void Update(float deltaTime) = 0;
    virtual bool IsComplete() const = 0;
    virtual bool IsKilled() const = 0;
    virtual void Kill() = 0;

    // Builder Methods (Method Chaining)
    CTween* SetEase(Ease ease) { m_ease = ease; return this; }
    CTween* SetDelay(float delay) { m_delay = delay; return this; }
    CTween* SetLoops(int loops, LoopType loopType = LoopType::Restart)
    {
        m_loops = loops;
        m_loopType = loopType;
        return this;
    }
    CTween* OnUpdate(std::function<void()> callback) { m_onUpdate = callback; return this; }
    CTween* OnComplete(std::function<void()> callback) { m_onComplete = callback; return this; }
    CTween* SetTarget(void* target) { m_target = target; return this; }
    void* GetTarget() const { return m_target; }

    void Pause() { m_isPaused = true; }
    void Resume() { m_isPaused = false; }
    bool IsPaused() const { return m_isPaused; }

protected:
    Ease m_ease = Ease::OutQuad;
    float m_delay = 0.0f;
    float m_delayTimer = 0.0f;
    float m_duration = 1.0f;
    float m_elapsedTime = 0.0f;

    int m_loops = 1; // 1 = once, -1 = infinite
    int m_completedLoops = 0;
    LoopType m_loopType = LoopType::Restart;
    bool m_isBackwards = false;

    bool m_isPaused = false;
    bool m_isKilled = false;
    bool m_isComplete = false;

    void* m_target = nullptr;
    std::function<void()> m_onUpdate;
    std::function<void()> m_onComplete;
};

// Template Lerp Helper
namespace TweenMath
{
    inline float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    inline DirectX::XMFLOAT2 Lerp(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b, float t)
    {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
    }

    inline DirectX::XMFLOAT3 Lerp(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t)
    {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
    }

    inline DirectX::XMFLOAT4 Lerp(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b, float t)
    {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t };
    }
}

// Property Tween Implementation
template <typename T>
class CTweenProperty : public CTween
{
public:
    using GetterFunc = std::function<T()>;
    using SetterFunc = std::function<void(const T&)>;

    CTweenProperty(GetterFunc getter, SetterFunc setter, T endValue, float duration)
        : m_getter(getter), m_setter(setter), m_endValue(endValue)
    {
        m_duration = (duration > 0.0f) ? duration : 0.0001f;
        if (m_getter)
        {
            m_startValue = m_getter();
        }
    }

    virtual void Update(float deltaTime) override
    {
        if (m_isPaused || m_isKilled || m_isComplete) return;

        // 1. Handle Delay
        if (m_delayTimer < m_delay)
        {
            m_delayTimer += deltaTime;
            if (m_delayTimer < m_delay) return;
            // Delay finished, capture start value fresh
            if (m_getter) m_startValue = m_getter();
        }

        // 2. Advance time
        m_elapsedTime += deltaTime;
        float progress = m_elapsedTime / m_duration;
        bool loopFinished = false;

        if (progress >= 1.0f)
        {
            progress = 1.0f;
            loopFinished = true;
        }

        // 3. Evaluate ease
        float easeT = EaseUtility::Evaluate(m_ease, progress);
        if (m_isBackwards)
        {
            easeT = 1.0f - easeT;
        }

        // 4. Apply interpolated value
        T currentVal = TweenMath::Lerp(m_startValue, m_endValue, easeT);
        if (m_setter)
        {
            m_setter(currentVal);
        }

        if (m_onUpdate)
        {
            m_onUpdate();
        }

        // 5. Handle loop / completion
        if (loopFinished)
        {
            m_completedLoops++;
            if (m_loops != -1 && m_completedLoops >= m_loops)
            {
                m_isComplete = true;
                if (m_onComplete)
                {
                    m_onComplete();
                }
            }
            else
            {
                // Loop restart or yoyo
                m_elapsedTime = 0.0f;
                if (m_loopType == LoopType::Yoyo)
                {
                    m_isBackwards = !m_isBackwards;
                }
            }
        }
    }

    virtual bool IsComplete() const override { return m_isComplete; }
    virtual bool IsKilled() const override { return m_isKilled; }
    virtual void Kill() override { m_isKilled = true; }

private:
    GetterFunc m_getter;
    SetterFunc m_setter;
    T m_startValue{};
    T m_endValue{};
};