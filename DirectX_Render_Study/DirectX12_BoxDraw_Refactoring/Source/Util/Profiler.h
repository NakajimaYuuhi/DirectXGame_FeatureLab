#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>

struct ProfileSample
{
    std::string name;
    double durationMs = 0.0;
    double smoothedMs = 0.0;
    std::chrono::high_resolution_clock::time_point startTime;
};

class Profiler
{
public:
    static Profiler& GetInstance()
    {
        static Profiler instance;
        return instance;
    }

    void BeginFrame();
    void EndFrame();

    void BeginSample(const std::string& name);
    void EndSample(const std::string& name);

    // Get profiling results
    const std::vector<ProfileSample>& GetSamples() const { return m_samples; }
    double GetTotalFrameTimeMs() const { return m_totalFrameTimeMs; }
    float GetCurrentFPS() const { return m_currentFPS; }
    const std::vector<float>& GetFrameTimeHistory() const { return m_frameTimeHistory; }

private:
    Profiler() = default;

    std::vector<ProfileSample> m_samples;
    std::unordered_map<std::string, size_t> m_sampleIndexMap;

    std::chrono::high_resolution_clock::time_point m_frameStartTime;
    double m_totalFrameTimeMs = 0.0;
    float m_currentFPS = 0.0f;

    std::vector<float> m_frameTimeHistory;
    static constexpr size_t MAX_HISTORY = 120;
};

// RAII Scope Profiler Helper
class ProfileScope
{
public:
    ProfileScope(const std::string& name) : m_name(name)
    {
        Profiler::GetInstance().BeginSample(m_name);
    }
    ~ProfileScope()
    {
        Profiler::GetInstance().EndSample(m_name);
    }

private:
    std::string m_name;
};

#define PROFILE_SCOPE(name) ProfileScope profileScope##__LINE__(name)