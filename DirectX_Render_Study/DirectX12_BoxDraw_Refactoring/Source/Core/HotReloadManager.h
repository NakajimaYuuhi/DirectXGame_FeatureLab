#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <Windows.h>

enum class EditorPlayState
{
    Playing,
    Paused,
    Stopped // Edit Mode (Game Update paused, Scene restored to snapshot)
};

class HotReloadManager
{
public:
    static HotReloadManager& GetInstance()
    {
        static HotReloadManager instance;
        return instance;
    }

    // Initialize file timestamps for watching
    void Initialize();

    // Clean up loaded DLL on shutdown (call after SceneManager::Uninit)
    void Finalize();

    // Called once per frame on the main thread (outside Update/Draw)
    void ProcessMainThreadTasks(float deltaTime);

    // -----------------------------------------------------------------
    // 1. C++ GameLogic DLL Hot Reload
    // -----------------------------------------------------------------
    bool CompileAndReloadGameLogicAsync();
    bool IsCompilingGameLogic() const;
    int GetReloadVersion() const { return m_reloadVersion.load(); }
    const std::string& GetCurrentLoadedDllName() const { return m_loadedDllName; }

    // -----------------------------------------------------------------
    // 2. Shader (.hlsl) & Prefab (.json) Hot Reload
    // -----------------------------------------------------------------
    bool ReloadShadersAndPrefabs(std::string* outMessage = nullptr);

    // -----------------------------------------------------------------
    // 3. Auto File Watcher Settings
    // -----------------------------------------------------------------
    bool IsAutoWatchShadersEnabled() const { return m_autoWatchShaders; }
    void SetAutoWatchShadersEnabled(bool enabled) { m_autoWatchShaders = enabled; }

    bool IsAutoWatchCppEnabled() const { return m_autoWatchCpp; }
    void SetAutoWatchCppEnabled(bool enabled) { m_autoWatchCpp = enabled; }

    // -----------------------------------------------------------------
    // 4. Editor Play / Pause / Stop Mode (In-Memory Scene Snapshot)
    // -----------------------------------------------------------------
    EditorPlayState GetPlayState() const { return m_playState; }
    bool ShouldUpdateGameLogic() const { return m_playState == EditorPlayState::Playing; }

    void Play();
    void Pause();
    void StopAndRestoreSnapshot();
    bool CaptureSceneSnapshot();
    bool HasSceneSnapshot() const { return !m_playModeSnapshotJson.empty(); }

    const std::string& GetLastStatusMessage() const { return m_lastStatusMessage; }
    bool IsLastStatusError() const { return m_lastStatusIsError; }

private:
    HotReloadManager() = default;
    ~HotReloadManager();
    HotReloadManager(const HotReloadManager&) = delete;
    HotReloadManager& operator=(const HotReloadManager&) = delete;

    std::string FindVcVars64Path() const;
    bool PerformDllSwapOnMainThread(const std::string& newDllPath);
    void ScanWatchedFiles(bool triggerReloadOnChange);

private:
    HMODULE m_hLoadedDll = nullptr;
    std::string m_loadedDllName = "(Built-in EXE)";
    std::atomic<int> m_reloadVersion{ 0 };

    std::mutex m_pendingMutex;
    std::string m_pendingDllPath;
    std::atomic<bool> m_hasPendingDllSwap{ false };
    std::atomic<bool> m_hasPendingShaderReload{ false };

    // File watcher state
    bool m_isInitialized = false;
    bool m_autoWatchShaders = true;
    bool m_autoWatchCpp = false;
    float m_watchTimer = 0.0f;
    const float WATCH_INTERVAL = 0.5f;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_shaderTimestamps;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_cppTimestamps;

    // Editor Play / Stop state & in-memory scene snapshot
    EditorPlayState m_playState = EditorPlayState::Playing;
    std::string m_playModeSnapshotJson;

    std::string m_lastStatusMessage = "Ready.";
    bool m_lastStatusIsError = false;
};
