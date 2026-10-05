#include "Profiler.h"
#include <algorithm>

void Profiler::BeginFrame()
{
    m_frameStartTime = std::chrono::high_resolution_clock::now();
}

void Profiler::EndFrame()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> frameDuration = now - m_frameStartTime;
    m_totalFrameTimeMs = frameDuration.count();

    if (m_totalFrameTimeMs > 0.0)
    {
        float instantFPS = static_cast<float>(1000.0 / m_totalFrameTimeMs);
        m_currentFPS = m_currentFPS * 0.9f + instantFPS * 0.1f; // Smoothing
    }

    m_frameTimeHistory.push_back(static_cast<float>(m_totalFrameTimeMs));
    if (m_frameTimeHistory.size() > MAX_HISTORY)
    {
        m_frameTimeHistory.erase(m_frameTimeHistory.begin());
    }
}

void Profiler::BeginSample(const std::string& name)
{
    auto now = std::chrono::high_resolution_clock::now();

    auto it = m_sampleIndexMap.find(name);
    if (it == m_sampleIndexMap.end())
    {
        size_t newIndex = m_samples.size();
        ProfileSample s;
        s.name = name;
        s.durationMs = 0.0;
        s.smoothedMs = 0.0;
        s.startTime = now;
        m_samples.push_back(s);
        m_sampleIndexMap[name] = newIndex;
    }
    else
    {
        m_samples[it->second].startTime = now;
    }
}

void Profiler::EndSample(const std::string& name)
{
    auto now = std::chrono::high_resolution_clock::now();

    auto it = m_sampleIndexMap.find(name);
    if (it != m_sampleIndexMap.end())
    {
        auto& sample = m_samples[it->second];
        std::chrono::duration<double, std::milli> duration = now - sample.startTime;
        sample.durationMs = duration.count();
        // Exponential moving average smoothing
        sample.smoothedMs = sample.smoothedMs * 0.9 + sample.durationMs * 0.1;
    }
}