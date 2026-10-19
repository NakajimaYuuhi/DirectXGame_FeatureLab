#include "HotReloadManager.h"
#include "Source/Editor/BuildRunner.h"
#include "Source/Editor/InspectorUI.h"
#include "Source/Editor/UndoManager.h"
#include "Source/Engine/Component/ComponentFactory.h"
#include "Source/Objects/ObjectManager.h"
#include "Source/Core/PrefabManager.h"
#include "Source/Core/DX12Manager.h"
#include "Source/Rendering/PSOManager.h"
#include "Source/Rendering/ShaderManager.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"
#include <sstream>

namespace fs = std::filesystem;

using RegisterGameLogicComponentsFn = void(*)(ComponentFactory*);

HotReloadManager::~HotReloadManager()
{
    Finalize();
}

void HotReloadManager::Initialize()
{
    if (m_isInitialized) return;
    ScanWatchedFiles(false);
    CaptureSceneSnapshot();
    m_isInitialized = true;
}

void HotReloadManager::Finalize()
{
    if (m_hLoadedDll)
    {
        FreeLibrary(m_hLoadedDll);
        m_hLoadedDll = nullptr;
    }
}

void HotReloadManager::ProcessMainThreadTasks(float deltaTime)
{
    if (!m_isInitialized)
    {
        Initialize();
    }

    // 1. Check if a background C++ DLL compilation finished and is ready to swap
    if (m_hasPendingDllSwap.exchange(false))
    {
        std::string dllPath;
        {
            std::lock_guard<std::mutex> lock(m_pendingMutex);
            dllPath = m_pendingDllPath;
        }
        if (!dllPath.empty())
        {
            PerformDllSwapOnMainThread(dllPath);
        }
    }

    // 2. Check if a shader reload was queued
    if (m_hasPendingShaderReload.exchange(false))
    {
        std::string msg;
        ReloadShadersAndPrefabs(&msg);
    }

    // 3. Periodic file watcher check
    m_watchTimer += deltaTime;
    if (m_watchTimer >= WATCH_INTERVAL)
    {
        m_watchTimer = 0.0f;
        ScanWatchedFiles(true);
    }
}

std::string HotReloadManager::FindVcVars64Path() const
{
    const std::vector<std::string> candidates = {
        "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvars64.bat",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvars64.bat",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat"
    };

    for (const auto& p : candidates)
    {
        if (fs::exists(p)) return p;
    }

    // Fallback: derive from MSBuild path
    std::string msbuild = BuildRunner::GetInstance().FindMSBuildPath();
    auto pos = msbuild.find("\\MSBuild\\");
    if (pos != std::string::npos)
    {
        std::string vsRoot = msbuild.substr(0, pos);
        std::string vcvars = vsRoot + "\\VC\\Auxiliary\\Build\\vcvars64.bat";
        if (fs::exists(vcvars)) return vcvars;
    }

    return "vcvars64.bat";
}

bool HotReloadManager::IsCompilingGameLogic() const
{
    return BuildRunner::GetInstance().IsRunning();
}

bool HotReloadManager::CompileAndReloadGameLogicAsync()
{
    BuildRunner& runner = BuildRunner::GetInstance();
    if (runner.IsRunning())
    {
        return false;
    }

#ifdef _DEBUG
    std::string config = "Debug";
    std::string crtAndOptFlags = "/MTd /Zi /Od /D_DEBUG";
#else
    std::string config = "Release";
    std::string crtAndOptFlags = "/MT /O2 /DNDEBUG";
#endif

    fs::path engineLibPath = fs::path("x64") / config / "DirectX12_BoxDraw_Refactoring.lib";
    if (!fs::exists(engineLibPath))
    {
        m_lastStatusMessage = "Engine import library not found: " + engineLibPath.string();
        m_lastStatusIsError = true;
        runner.AppendLog("[HotReload Error] " + m_lastStatusMessage);
        return false;
    }

    fs::path outDir("Build/HotReload");
    fs::create_directories(outDir);

    int nextVer = m_reloadVersion.load() + 1;
    std::string verStr = std::to_string(nextVer);

    fs::path outDll = outDir / ("GameLogic_" + verStr + ".dll");
    fs::path outPdb = outDir / ("GameLogic_" + verStr + ".pdb");
    fs::path outImpLib = outDir / ("GameLogic_" + verStr + ".lib");
    fs::path objDir = outDir / ("Obj_" + verStr);
    fs::create_directories(objDir);

    std::string vcvars = FindVcVars64Path();

    // Include directories matching DirectX12_BoxDraw_Refactoring.vcxproj
    std::string includes =
        "/I\".\" "
        "/I\"DirectXTex\" "
        "/I\"Source\\External\" "
        "/I\"Source\\Core\" "
        "/I\"Source\\Core\\Scenes\\Base\" "
        "/I\"Source\\Core\\Scenes\\Manager\" "
        "/I\"Source\\Core\\Scenes\\Serializer\" "
        "/I\"Source\\Rendering\" "
        "/I\"Source\\Resources\" "
        "/I\"Source\\Engine\\Component\" "
        "/I\"Source\\Engine\" "
        "/I\"Source\\Game\" "
        "/I\"Source\\Game\\Entities\" "
        "/I\"Source\\Game\\Entities\\Characters\\Player\" "
        "/I\"Source\\Game\\Entities\\Characters\\Enemy\" "
        "/I\"Source\\Game\\Entities\\Bullet\\PlayerBullet\" "
        "/I\"Source\\Game\\Scenes\" "
        "/I\"Source\\Game\\Features\\Status\" "
        "/I\"Source\\Objects\" "
        "/I\"Source\\Editor\" "
        "/I\"Source\\UI\" "
        "/I\"Source\\Util\" "
        "/I\"Source\\Util\\Logic\\StateMachine\"";

    std::string sources =
        "\"Source\\Game\\GameLogicDll.cpp\" "
        "\"Source\\Game\\Entities\\Characters\\Player\\PlayerControllerComponent.cpp\" "
        "\"Source\\Game\\Entities\\Characters\\Enemy\\EnemyAIComponent.cpp\" "
        "\"Source\\Game\\Entities\\Bullet\\PlayerBullet\\BulletComponent.cpp\" "
        "\"Source\\Game\\Features\\Status\\HealthComponent.cpp\"";

    std::string clCmd =
        "cmd.exe /C \"call \"" + vcvars + "\" >nul && "
        "cl.exe /nologo /std:c++20 /EHsc /MP " + crtAndOptFlags + " "
        "/DGAMELOGIC_DLL_BUILD /D_CONSOLE " + includes + " "
        "/Fo\"" + objDir.string() + "\\\\\" "
        "/Fd\"" + (objDir / "vc.pdb").string() + "\" "
        + sources + " /LD "
        "/Fe\"" + outDll.string() + "\" "
        "/link /NOLOGO "
        "/PDB:\"" + outPdb.string() + "\" "
        "/IMPLIB:\"" + outImpLib.string() + "\" "
        "\"" + engineLibPath.string() + "\" "
        "d3d12.lib dxgi.lib dxguid.lib winmm.lib xinput.lib user32.lib\"";

    m_lastStatusMessage = "Compiling GameLogic DLL (v" + verStr + ")...";
    m_lastStatusIsError = false;

    std::string capturedDllPath = outDll.string();

    return runner.StartCommandAsync(
        clCmd,
        ".",
        [this, capturedDllPath, nextVer](bool success, int exitCode)
        {
            BuildRunner& br = BuildRunner::GetInstance();
            if (!success)
            {
                m_lastStatusMessage = "C++ Hot-Reload compile failed (Exit code " + std::to_string(exitCode) + "). Keeping existing game logic.";
                m_lastStatusIsError = true;
                br.AppendLog("[HotReload] Compilation failed. Active scene and existing components are untouched.");
                return;
            }

            br.AppendLog("[HotReload] Compiled " + capturedDllPath + " successfully! Queuing main-thread state swap...");
            {
                std::lock_guard<std::mutex> lock(m_pendingMutex);
                m_pendingDllPath = capturedDllPath;
            }
            m_reloadVersion.store(nextVer);
            m_hasPendingDllSwap.store(true);
        }
    );
}

bool HotReloadManager::PerformDllSwapOnMainThread(const std::string& newDllPath)
{
    BuildRunner& br = BuildRunner::GetInstance();
    Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();

    // 1. Preserve selected object indices in Inspector and clear editor selection before destroying objects
    int prevTagIdx = CInspectorUI::GetInstance().GetSelectedTagIndex();
    int prevObjIdx = CInspectorUI::GetInstance().GetSelectedObjectIndex();
    CInspectorUI::GetInstance().ClearSelection();
    UndoManager::GetInstance().Clear();

    // 2. Snapshot current scene state in memory (JSON)
    std::string sceneSnapshot;
    bool serialized = SceneSerializer::SerializeToString(sceneSnapshot, activeScene);
    if (!serialized)
    {
        br.AppendLog("[HotReload Warning] Failed to serialize current scene state.");
    }

    // 3. Destroy all scene objects while the OLD DLL is still in memory so virtual destructors are valid
    DX12Manager::GetInstance().WaitForPendingOperations();
    ObjectManager::GetInstance().Uninit();

    // 4. Unload previous hot-reload DLL if one was loaded
    if (m_hLoadedDll)
    {
        FreeLibrary(m_hLoadedDll);
        m_hLoadedDll = nullptr;
        br.AppendLog("[HotReload] Unloaded previous GameLogic DLL.");
    }

    // 5. Load newly compiled GameLogic DLL
    HMODULE hNewDll = LoadLibraryA(newDllPath.c_str());
    if (!hNewDll)
    {
        m_lastStatusMessage = "Failed to LoadLibrary: " + newDllPath;
        m_lastStatusIsError = true;
        br.AppendLog("[HotReload Error] " + m_lastStatusMessage);

        // Fallback to built-in EXE component creators and restore scene
        ComponentFactory::GetInstance().InitDefaultComponents();
        PrefabManager::GetInstance().InitDefaultPrefabs();
        if (serialized)
        {
            SceneSerializer::DeserializeFromString(sceneSnapshot, activeScene);
        }
        return false;
    }

    auto regFn = reinterpret_cast<RegisterGameLogicComponentsFn>(GetProcAddress(hNewDll, "RegisterGameLogicComponents"));
    if (!regFn)
    {
        FreeLibrary(hNewDll);
        m_lastStatusMessage = "RegisterGameLogicComponents export not found in " + newDllPath;
        m_lastStatusIsError = true;
        br.AppendLog("[HotReload Error] " + m_lastStatusMessage);

        ComponentFactory::GetInstance().InitDefaultComponents();
        PrefabManager::GetInstance().InitDefaultPrefabs();
        if (serialized)
        {
            SceneSerializer::DeserializeFromString(sceneSnapshot, activeScene);
        }
        return false;
    }

    // 6. Register new DLL component creators into ComponentFactory & refresh Prefabs
    m_hLoadedDll = hNewDll;
    m_loadedDllName = fs::path(newDllPath).filename().string();
    regFn(&ComponentFactory::GetInstance());
    PrefabManager::GetInstance().InitDefaultPrefabs();

    // 7. Restore scene state from in-memory JSON snapshot using the new DLL components!
    if (serialized && !sceneSnapshot.empty())
    {
        SceneSerializer::DeserializeFromString(sceneSnapshot, activeScene);
    }

    // 8. Re-select previously selected object in Inspector
    if (prevTagIdx >= 0 && prevObjIdx >= 0)
    {
        CInspectorUI::GetInstance().SetSelectedObject(prevTagIdx, prevObjIdx);
    }

    m_lastStatusMessage = "Hot-Reloaded " + m_loadedDllName + " & restored scene!";
    m_lastStatusIsError = false;
    br.AppendLog("[HotReload] Successfully swapped to " + m_loadedDllName + " and restored scene state!");
    CInspectorUI::GetInstance().SetStatusMessage(m_lastStatusMessage, 4.0f);
    return true;
}

bool HotReloadManager::ReloadShadersAndPrefabs(std::string* outMessage)
{
    BuildRunner& br = BuildRunner::GetInstance();
    DX12Manager::GetInstance().WaitForPendingOperations();

    std::string errLog;
    bool ok = PSOManager::GetInstance().RebuildPSOs(DX12Manager::GetInstance().GetDevice(), &errLog);

    // Also refresh JSON prefabs in Assets/Prefabs
    PrefabManager::GetInstance().RegisterPrefabsInDirectory("Assets/Prefabs");

    if (ok)
    {
        m_lastStatusMessage = "Shaders (.hlsl) & Prefabs (.json) hot-reloaded successfully!";
        m_lastStatusIsError = false;
        br.AppendLog("[ShaderHotReload] " + m_lastStatusMessage);
        CInspectorUI::GetInstance().SetStatusMessage(m_lastStatusMessage, 3.0f);
        if (outMessage) *outMessage = m_lastStatusMessage;
        return true;
    }
    else
    {
        m_lastStatusMessage = "Shader Hot-Reload failed (kept previous working shaders):\n" + errLog;
        m_lastStatusIsError = true;
        br.AppendLog("[ShaderHotReload Error] " + m_lastStatusMessage);
        CInspectorUI::GetInstance().SetStatusMessage("Shader compile error! Check Build/HotReload window.", 4.0f);
        if (outMessage) *outMessage = m_lastStatusMessage;
        return false;
    }
}

void HotReloadManager::ScanWatchedFiles(bool triggerReloadOnChange)
{
    bool shaderChanged = false;
    bool cppChanged = false;

    std::error_code ec;
    if (fs::exists("Assets/Shader", ec))
    {
        for (const auto& entry : fs::recursive_directory_iterator("Assets/Shader", ec))
        {
            if (!entry.is_regular_file(ec)) continue;
            std::string ext = entry.path().extension().string();
            if (ext != ".hlsl" && ext != ".hlsli") continue;

            std::string pathStr = entry.path().string();
            auto ftime = entry.last_write_time(ec);
            auto it = m_shaderTimestamps.find(pathStr);
            if (it == m_shaderTimestamps.end())
            {
                m_shaderTimestamps[pathStr] = ftime;
            }
            else if (it->second != ftime)
            {
                it->second = ftime;
                shaderChanged = true;
            }
        }
    }

    if (fs::exists("Source/Game", ec))
    {
        for (const auto& entry : fs::recursive_directory_iterator("Source/Game", ec))
        {
            if (!entry.is_regular_file(ec)) continue;
            std::string ext = entry.path().extension().string();
            if (ext != ".cpp" && ext != ".h") continue;

            std::string pathStr = entry.path().string();
            auto ftime = entry.last_write_time(ec);
            auto it = m_cppTimestamps.find(pathStr);
            if (it == m_cppTimestamps.end())
            {
                m_cppTimestamps[pathStr] = ftime;
            }
            else if (it->second != ftime)
            {
                it->second = ftime;
                cppChanged = true;
            }
        }
    }

    if (!triggerReloadOnChange) return;

    if (shaderChanged && m_autoWatchShaders)
    {
        ReloadShadersAndPrefabs();
    }

    if (cppChanged && m_autoWatchCpp && !IsCompilingGameLogic())
    {
        CompileAndReloadGameLogicAsync();
    }
}

bool HotReloadManager::CaptureSceneSnapshot()
{
    Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();
    if (SceneSerializer::SerializeToString(m_playModeSnapshotJson, activeScene))
    {
        m_lastStatusMessage = "Captured in-memory scene snapshot.";
        m_lastStatusIsError = false;
        return true;
    }
    return false;
}

void HotReloadManager::Play()
{
    if (m_playState == EditorPlayState::Stopped)
    {
        CaptureSceneSnapshot();
    }
    m_playState = EditorPlayState::Playing;
    m_lastStatusMessage = "Editor Mode: Playing";
    m_lastStatusIsError = false;
}

void HotReloadManager::Pause()
{
    if (m_playState == EditorPlayState::Playing)
    {
        m_playState = EditorPlayState::Paused;
        m_lastStatusMessage = "Editor Mode: Paused";
        m_lastStatusIsError = false;
    }
    else if (m_playState == EditorPlayState::Paused)
    {
        m_playState = EditorPlayState::Playing;
        m_lastStatusMessage = "Editor Mode: Resumed";
        m_lastStatusIsError = false;
    }
}

void HotReloadManager::StopAndRestoreSnapshot()
{
    m_playState = EditorPlayState::Stopped;
    if (!m_playModeSnapshotJson.empty())
    {
        int prevTagIdx = CInspectorUI::GetInstance().GetSelectedTagIndex();
        int prevObjIdx = CInspectorUI::GetInstance().GetSelectedObjectIndex();
        CInspectorUI::GetInstance().ClearSelection();

        Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();
        SceneSerializer::DeserializeFromString(m_playModeSnapshotJson, activeScene);

        if (prevTagIdx >= 0 && prevObjIdx >= 0)
        {
            CInspectorUI::GetInstance().SetSelectedObject(prevTagIdx, prevObjIdx);
        }
        m_lastStatusMessage = "Stopped & restored scene from in-memory snapshot.";
        m_lastStatusIsError = false;
        CInspectorUI::GetInstance().SetStatusMessage(m_lastStatusMessage, 3.0f);
    }
}
