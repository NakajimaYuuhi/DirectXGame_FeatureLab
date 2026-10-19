#include "BuildRunner.h"
#include <Windows.h>
#include <filesystem>
#include <sstream>

BuildRunner::~BuildRunner()
{
    Finalize();
}

void BuildRunner::Finalize()
{
    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }
}

std::string BuildRunner::FindMSBuildPath() const
{
    // 1. vswhere.exe を用いた自動検出
    char* progFilesX86 = nullptr;
    size_t len = 0;
    _dupenv_s(&progFilesX86, &len, "ProgramFiles(x86)");
    std::string vswherePath = progFilesX86
        ? (std::string(progFilesX86) + "\\Microsoft Visual Studio\\Installer\\vswhere.exe")
        : "C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe";
    if (progFilesX86) free(progFilesX86);

    if (std::filesystem::exists(vswherePath))
    {
        SECURITY_ATTRIBUTES sa = {};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;

        HANDLE hRead = NULL, hWrite = NULL;
        if (CreatePipe(&hRead, &hWrite, &sa, 0))
        {
            SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

            STARTUPINFOA si = {};
            si.cb = sizeof(STARTUPINFOA);
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.hStdOutput = hWrite;
            si.hStdError = hWrite;
            si.wShowWindow = SW_HIDE;

            PROCESS_INFORMATION pi = {};
            std::string cmd = "\"" + vswherePath + "\" -latest -requires Microsoft.Component.MSBuild -find \"MSBuild\\**\\Bin\\MSBuild.exe\"";
            std::vector<char> cmdBuf(cmd.begin(), cmd.end());
            cmdBuf.push_back('\0');

            if (CreateProcessA(NULL, cmdBuf.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                CloseHandle(hWrite);
                hWrite = NULL;

                std::string output;
                char buffer[512];
                DWORD bytesRead = 0;
                while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    output += buffer;
                }

                WaitForSingleObject(pi.hProcess, 3000);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                std::istringstream iss(output);
                std::string firstLine;
                if (std::getline(iss, firstLine))
                {
                    while (!firstLine.empty() && (firstLine.back() == '\r' || firstLine.back() == '\n' || firstLine.back() == ' '))
                    {
                        firstLine.pop_back();
                    }
                    if (!firstLine.empty() && std::filesystem::exists(firstLine))
                    {
                        CloseHandle(hRead);
                        return firstLine;
                    }
                }
            }
            if (hWrite) CloseHandle(hWrite);
            if (hRead) CloseHandle(hRead);
        }
    }

    // 2. フォールバック検索
    const std::vector<std::string> fallbackPaths = {
        "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"
    };

    for (const auto& p : fallbackPaths)
    {
        if (std::filesystem::exists(p))
        {
            return p;
        }
    }

    return "MSBuild.exe";
}

void BuildRunner::AppendLog(const std::string& line)
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logs.push_back(line);
}

void BuildRunner::ClearLogs()
{
    if (IsRunning()) return;
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logs.clear();
}

std::vector<std::string> BuildRunner::GetLogsSnapshot() const
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    return m_logs;
}

float BuildRunner::GetElapsedSeconds() const
{
    auto state = m_state.load();
    if (state == BuildState::Idle) return 0.0f;

    auto end = (state == BuildState::Running) ? std::chrono::steady_clock::now() : m_endTime;
    return std::chrono::duration<float>(end - m_startTime).count();
}

bool BuildRunner::StartMSBuildAsync(
    const std::string& targetFile,
    const std::string& configuration,
    const std::string& platform,
    const std::string& extraArgs,
    OnBuildCompleteCallback onComplete)
{
    std::string msbuild = FindMSBuildPath();
    std::string cmd = "\"" + msbuild + "\" \"" + targetFile + "\""
        + " /p:Configuration=" + configuration
        + " /p:Platform=" + platform
        + " " + extraArgs;

    return StartCommandAsync(cmd, ".", onComplete);
}

bool BuildRunner::StartCommandAsync(
    const std::string& commandLine,
    const std::string& workingDir,
    OnBuildCompleteCallback onComplete)
{
    if (IsRunning())
    {
        return false;
    }

    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }

    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_logs.clear();
        m_logs.push_back("[BuildRunner] Starting command:");
        m_logs.push_back("  > " + commandLine);
    }

    m_state.store(BuildState::Running);
    m_lastExitCode.store(0);
    m_startTime = std::chrono::steady_clock::now();

    m_workerThread = std::thread(&BuildRunner::WorkerThreadMain, this, commandLine, workingDir, onComplete);
    return true;
}

void BuildRunner::WorkerThreadMain(
    std::string commandLine,
    std::string workingDir,
    OnBuildCompleteCallback onComplete)
{
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hReadPipe = NULL;
    HANDLE hWritePipe = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
    {
        AppendLog("[Error] Failed to create output pipe.");
        m_lastExitCode.store(-1);
        m_endTime = std::chrono::steady_clock::now();
        m_state.store(BuildState::Failed);
        if (onComplete) onComplete(false, -1);
        return;
    }

    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {};
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    std::vector<char> cmdBuf(commandLine.begin(), commandLine.end());
    cmdBuf.push_back('\0');

    const char* workDirPtr = workingDir.empty() ? NULL : workingDir.c_str();

    BOOL created = CreateProcessA(
        NULL,
        cmdBuf.data(),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        workDirPtr,
        &si,
        &pi
    );

    // 親プロセス側では書き込みハンドルをすぐ閉じる（子プロセス終了時にReadFileをEOFにするため）
    CloseHandle(hWritePipe);

    if (!created)
    {
        CloseHandle(hReadPipe);
        AppendLog("[Error] Failed to launch build process. Check MSBuild path.");
        m_lastExitCode.store(-2);
        m_endTime = std::chrono::steady_clock::now();
        m_state.store(BuildState::Failed);
        if (onComplete) onComplete(false, -2);
        return;
    }

    std::string lineBuffer;
    char readBuf[1024];
    DWORD bytesRead = 0;

    while (ReadFile(hReadPipe, readBuf, sizeof(readBuf) - 1, &bytesRead, NULL) && bytesRead > 0)
    {
        readBuf[bytesRead] = '\0';
        for (DWORD i = 0; i < bytesRead; ++i)
        {
            char c = readBuf[i];
            if (c == '\r') continue;
            if (c == '\n')
            {
                AppendLog(lineBuffer);
                lineBuffer.clear();
            }
            else
            {
                lineBuffer.push_back(c);
            }
        }
    }

    if (!lineBuffer.empty())
    {
        AppendLog(lineBuffer);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hReadPipe);

    m_lastExitCode.store(static_cast<int>(exitCode));
    bool success = (exitCode == 0);

    if (onComplete)
    {
        onComplete(success, static_cast<int>(exitCode));
    }

    m_endTime = std::chrono::steady_clock::now();
    if (m_state.load() == BuildState::Running)
    {
        m_state.store(success ? BuildState::Succeeded : BuildState::Failed);
    }
}
