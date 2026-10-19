#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

enum class BuildState
{
    Idle,
    Running,
    Succeeded,
    Failed
};

class BuildRunner
{
public:
    using OnBuildCompleteCallback = std::function<void(bool success, int exitCode)>;

    static BuildRunner& GetInstance()
    {
        static BuildRunner instance;
        return instance;
    }

    // Search for MSBuild.exe using vswhere.exe or standard installation paths
    std::string FindMSBuildPath() const;

    // Start an asynchronous build command
    bool StartCommandAsync(
        const std::string& commandLine,
        const std::string& workingDir = ".",
        OnBuildCompleteCallback onComplete = nullptr
    );

    // Start an asynchronous MSBuild for a project or solution
    bool StartMSBuildAsync(
        const std::string& targetFile,
        const std::string& configuration = "Release",
        const std::string& platform = "x64",
        const std::string& extraArgs = "/m /v:minimal",
        OnBuildCompleteCallback onComplete = nullptr
    );

    // Append custom log line (thread-safe)
    void AppendLog(const std::string& line);

    // Clear logs (only when not running)
    void ClearLogs();

    // State getters
    bool IsRunning() const { return m_state.load() == BuildState::Running; }
    BuildState GetState() const { return m_state.load(); }
    void SetState(BuildState state) { m_state.store(state); }
    int GetLastExitCode() const { return m_lastExitCode.load(); }
    float GetElapsedSeconds() const;

    // Get copy of current logs (thread-safe)
    std::vector<std::string> GetLogsSnapshot() const;

    // Wait for thread on shutdown
    void Finalize();

private:
    BuildRunner() = default;
    ~BuildRunner();
    BuildRunner(const BuildRunner&) = delete;
    BuildRunner& operator=(const BuildRunner&) = delete;

    void WorkerThreadMain(
        std::string commandLine,
        std::string workingDir,
        OnBuildCompleteCallback onComplete
    );

private:
    mutable std::mutex m_logMutex;
    std::vector<std::string> m_logs;

    std::atomic<BuildState> m_state{ BuildState::Idle };
    std::atomic<int> m_lastExitCode{ 0 };
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_endTime;

    std::thread m_workerThread;
};
