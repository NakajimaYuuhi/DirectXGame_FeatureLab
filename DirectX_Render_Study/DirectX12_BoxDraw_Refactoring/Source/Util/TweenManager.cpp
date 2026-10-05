#include "TweenManager.h"

void TweenManager::Update(float deltaTime)
{
    // 1. Flush pending additions to active list
    if (!m_pendingAddTweens.empty())
    {
        for (auto& pending : m_pendingAddTweens)
        {
            if (pending && !pending->IsKilled())
            {
                m_activeTweens.push_back(std::move(pending));
            }
        }
        m_pendingAddTweens.clear();
    }

    // 2. Update all active tweens
    for (size_t i = 0; i < m_activeTweens.size(); ++i)
    {
        auto& tween = m_activeTweens[i];
        if (tween && !tween->IsKilled() && !tween->IsComplete())
        {
            tween->Update(deltaTime);
        }
    }

    // 3. Clean up completed or killed tweens
    m_activeTweens.erase(
        std::remove_if(m_activeTweens.begin(), m_activeTweens.end(),
            [](const std::unique_ptr<CTween>& t) {
                return !t || t->IsKilled() || t->IsComplete();
            }),
        m_activeTweens.end()
    );
}

void TweenManager::KillTweensOf(void* target)
{
    if (!target) return;

    for (auto& t : m_activeTweens)
    {
        if (t && t->GetTarget() == target)
        {
            t->Kill();
        }
    }

    for (auto& t : m_pendingAddTweens)
    {
        if (t && t->GetTarget() == target)
        {
            t->Kill();
        }
    }
}

void TweenManager::KillAll()
{
    for (auto& t : m_activeTweens)
    {
        if (t) t->Kill();
    }
    m_activeTweens.clear();

    for (auto& t : m_pendingAddTweens)
    {
        if (t) t->Kill();
    }
    m_pendingAddTweens.clear();
}

void TweenManager::PauseAll()
{
    for (auto& t : m_activeTweens)
    {
        if (t) t->Pause();
    }
}

void TweenManager::ResumeAll()
{
    for (auto& t : m_activeTweens)
    {
        if (t) t->Resume();
    }
}