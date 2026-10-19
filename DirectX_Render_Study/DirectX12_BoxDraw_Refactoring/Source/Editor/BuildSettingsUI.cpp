#include "BuildSettingsUI.h"
#include "BuildRunner.h"
#include "Source/Core/HotReloadManager.h"
#include "imgui.h"
#include <cstring>

void CBuildSettingsUI::Draw()
{
#ifndef _DEBUG
    return;
#endif

    // Global keyboard shortcuts (active even when window is closed)
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantTextInput)
    {
        // F5: Reload Shaders & Prefabs
        if (ImGui::IsKeyPressed(ImGuiKey_F5, false))
        {
            HotReloadManager::GetInstance().ReloadShadersAndPrefabs();
        }
        // Ctrl + Alt + F11: Compile & Hot-Reload C++ GameLogic DLL
        if (io.KeyCtrl && io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_F11, false))
        {
            HotReloadManager::GetInstance().CompileAndReloadGameLogicAsync();
            m_isVisible = true;
        }
    }

    if (!m_isVisible) return;

    ImGui::SetNextWindowSize(ImVec2(760, 580), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Build & Hot-Reload Center", &m_isVisible))
    {
        BuildRunner& runner = BuildRunner::GetInstance();
        HotReloadManager& hr = HotReloadManager::GetInstance();
        bool isBusy = runner.IsRunning();

        if (ImGui::BeginTabBar("BuildHotReloadTabs"))
        {
            // =================================================================
            // Tab 1: Game Package Builder (Standalone Build)
            // =================================================================
            if (ImGui::BeginTabItem("Game Package Build"))
            {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "Standalone Game Package Exporter");
                ImGui::Separator();
                ImGui::Spacing();

                static char outDirBuf[256] = "Build/ReleasePackage";
                static char appNameBuf[128] = "DirectX12_Game.exe";
                static bool bufInitialized = false;
                if (!bufInitialized)
                {
                    strncpy_s(outDirBuf, m_packageConfig.outputDir.c_str(), sizeof(outDirBuf) - 1);
                    strncpy_s(appNameBuf, m_packageConfig.appName.c_str(), sizeof(appNameBuf) - 1);
                    bufInitialized = true;
                }

                if (ImGui::InputText("Output Directory", outDirBuf, sizeof(outDirBuf)))
                {
                    m_packageConfig.outputDir = outDirBuf;
                }
                if (ImGui::InputText("Executable Name", appNameBuf, sizeof(appNameBuf)))
                {
                    m_packageConfig.appName = appNameBuf;
                }

                const char* configs[] = { "Release", "Debug" };
                if (ImGui::Combo("Build Configuration", &m_configIndex, configs, IM_ARRAYSIZE(configs)))
                {
                    m_packageConfig.configuration = configs[m_configIndex];
                }

                ImGui::Spacing();
                ImGui::Checkbox("Encrypt 3D Models (.glb -> .dat via XOR Cipher)", &m_packageConfig.encryptModels);
                if (m_packageConfig.encryptModels)
                {
                    ImGui::Indent();
                    ImGui::Checkbox("Exclude Plaintext .glb Files from Package", &m_packageConfig.excludePlaintextModels);
                    ImGui::Unindent();
                }
                ImGui::Checkbox("Clean Output Directory Before Packaging", &m_packageConfig.cleanOutputBeforeBuild);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (isBusy) ImGui::BeginDisabled();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.55f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.68f, 0.32f, 1.0f));
                if (ImGui::Button("Build & Package Game", ImVec2(220, 36)))
                {
                    m_packageConfig.outputDir = outDirBuf;
                    m_packageConfig.appName = appNameBuf;
                    m_packageConfig.configuration = configs[m_configIndex];
                    GamePackageBuilder::GetInstance().StartPackageBuild(m_packageConfig);
                }
                ImGui::PopStyleColor(2);
                if (isBusy) ImGui::EndDisabled();

                ImGui::SameLine();
                if (ImGui::Button("Open Output Folder", ImVec2(170, 36)))
                {
                    GamePackageBuilder::GetInstance().OpenOutputFolder(outDirBuf);
                }

                ImGui::SameLine();
                if (isBusy) ImGui::BeginDisabled();
                if (ImGui::Button("Launch Built Game", ImVec2(170, 36)))
                {
                    m_packageConfig.outputDir = outDirBuf;
                    m_packageConfig.appName = appNameBuf;
                    GamePackageBuilder::GetInstance().LaunchBuiltGame(m_packageConfig);
                }
                if (isBusy) ImGui::EndDisabled();

                ImGui::EndTabItem();
            }

            // =================================================================
            // Tab 2: Live Hot-Reload (C++ GameLogic DLL & Shaders/Assets)
            // =================================================================
            if (ImGui::BeginTabItem("Hot-Reload & Live Coding"))
            {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "1. C++ GameLogic DLL Hot-Reload");
                ImGui::Separator();

                ImGui::Text("Active Logic Module : %s", hr.GetCurrentLoadedDllName().c_str());
                ImGui::SameLine(360.0f);
                ImGui::Text("Reload Version : v%d", hr.GetReloadVersion());

                ImGui::Spacing();
                if (isBusy) ImGui::BeginDisabled();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.48f, 0.78f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.58f, 0.90f, 1.0f));
                if (ImGui::Button("Compile & Hot-Reload C++ (Ctrl+Alt+F11)", ImVec2(310, 34)))
                {
                    hr.CompileAndReloadGameLogicAsync();
                }
                ImGui::PopStyleColor(2);
                if (isBusy) ImGui::EndDisabled();

                ImGui::SameLine();
                bool autoCpp = hr.IsAutoWatchCppEnabled();
                if (ImGui::Checkbox("Auto-Reload C++ on Save (Source/Game/*)", &autoCpp))
                {
                    hr.SetAutoWatchCppEnabled(autoCpp);
                }

                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.6f, 1.0f), "2. Shaders (.hlsl) & Prefabs (.json) Hot-Reload");
                ImGui::Separator();

                if (ImGui::Button("Reload Shaders & Prefabs Now (F5)", ImVec2(310, 32)))
                {
                    hr.ReloadShadersAndPrefabs();
                }
                ImGui::SameLine();
                bool autoShader = hr.IsAutoWatchShadersEnabled();
                if (ImGui::Checkbox("Auto-Reload Shaders on Save (Assets/Shader/*)", &autoShader))
                {
                    hr.SetAutoWatchShadersEnabled(autoShader);
                }

                ImGui::Spacing();
                ImVec4 statusCol = hr.IsLastStatusError()
                    ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f)
                    : ImVec4(0.35f, 0.95f, 0.45f, 1.0f);
                ImGui::TextColored(statusCol, "[Status] %s", hr.GetLastStatusMessage().c_str());

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        // =====================================================================
        // Shared Bottom Section: Real-Time Build & Hot-Reload Console Output
        // =====================================================================
        ImGui::Spacing();
        ImGui::Separator();

        BuildState state = runner.GetState();
        if (state == BuildState::Running)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "[RUNNING] Building... (%.1f sec)", runner.GetElapsedSeconds());
        }
        else if (state == BuildState::Succeeded)
        {
            ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.35f, 1.0f), "[SUCCEEDED] Completed in %.1f sec", runner.GetElapsedSeconds());
        }
        else if (state == BuildState::Failed)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[FAILED] Exit Code %d (%.1f sec)", runner.GetLastExitCode(), runner.GetElapsedSeconds());
        }
        else
        {
            ImGui::TextDisabled("[IDLE] Build Console Output");
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 210.0f);
        ImGui::Checkbox("Auto-Scroll", &m_autoScrollLog);
        ImGui::SameLine();
        if (!isBusy && ImGui::SmallButton("Clear"))
        {
            runner.ClearLogs();
        }

        ImGui::BeginChild("BuildConsoleLog", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        auto logs = runner.GetLogsSnapshot();
        for (const auto& line : logs)
        {
            if (line.find("error ") != std::string::npos || line.find("[Error]") != std::string::npos || line.find("Error]") != std::string::npos || line.find("FAILED") != std::string::npos)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", line.c_str());
            }
            else if (line.find("warning ") != std::string::npos || line.find("[Warning]") != std::string::npos)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "%s", line.c_str());
            }
            else if (line.find("succeeded") != std::string::npos || line.find("successfully") != std::string::npos || line.find("====") != std::string::npos)
            {
                ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.45f, 1.0f), "%s", line.c_str());
            }
            else
            {
                ImGui::TextUnformatted(line.c_str());
            }
        }
        if (m_autoScrollLog && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.0f)
        {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
