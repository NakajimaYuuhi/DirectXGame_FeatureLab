#include "EditorToolbarUI.h"
#include "InspectorUI.h"
#include "UndoManager.h"
#include "ContentDrawerUI.h"
#include "ProfilerUI.h"
#include "AssetSecurityUI.h"
#include "BuildSettingsUI.h"
#include "ViewportUI.h"
#include "imgui.h"
#include "ObjectManager.h"
#include "TimeManager.h"
#include "Source/Core/HotReloadManager.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"

// -----------------------------------------------------------------
// ツールバーUIの描画更新処理（デバッグビルド時のみ有効）
// -----------------------------------------------------------------
void CEditorToolbarUI::Draw()
{
#ifndef _DEBUG
    return;
#endif // !_DEBUG

    if (!m_isVisible) return;

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float toolbarH = 48.0f;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenW, toolbarH), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
    if (ImGui::Begin("Editor Toolbar", &m_isVisible, flags))
    {
        // ---------------------------------------------------------
        // 1. Prefab Stage Mode Controls
        // ---------------------------------------------------------
        bool isPrefabMode = CInspectorUI::GetInstance().IsPrefabEditMode();
        if (isPrefabMode)
        {
            std::string path = CInspectorUI::GetInstance().GetEditingPrefabPath();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[PREFAB STAGE MODE] %s", path.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Save Prefab"))
            {
                CInspectorUI::GetInstance().SaveCurrentPrefab();
            }
            ImGui::SameLine();
            if (ImGui::Button("Exit Stage"))
            {
                CInspectorUI::GetInstance().ClosePrefabEditMode();
            }
            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();
        }

        // ---------------------------------------------------------
        // 2. Play / Pause / Stop Controls (In-Memory Scene Snapshot)
        // ---------------------------------------------------------
        HotReloadManager& hr = HotReloadManager::GetInstance();
        EditorPlayState playState = hr.GetPlayState();

        if (playState == EditorPlayState::Playing)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.65f, 0.28f, 1.0f));
        }
        if (ImGui::Button("Play"))
        {
            hr.Play();
        }
        if (playState == EditorPlayState::Playing)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        if (playState == EditorPlayState::Paused)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.55f, 0.15f, 1.0f));
        }
        if (ImGui::Button("Pause"))
        {
            hr.Pause();
        }
        if (playState == EditorPlayState::Paused)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        if (playState == EditorPlayState::Stopped)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.22f, 0.22f, 1.0f));
        }
        if (ImGui::Button("Stop"))
        {
            hr.StopAndRestoreSnapshot();
        }
        if (playState == EditorPlayState::Stopped)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        // ---------------------------------------------------------
        // 3. ギズモ操作モード切り替え（移動: W / 回転: E / 拡大縮小: R）
        // ---------------------------------------------------------
        GizmoMode currentGizmoMode = CInspectorUI::GetInstance().GetGizmoMode();
        if (currentGizmoMode == GizmoMode::Translate) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        if (ImGui::Button("Move (W)")) CInspectorUI::GetInstance().SetGizmoMode(GizmoMode::Translate);
        if (currentGizmoMode == GizmoMode::Translate) ImGui::PopStyleColor();

        ImGui::SameLine();
        if (currentGizmoMode == GizmoMode::Rotate) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        if (ImGui::Button("Rotate (E)")) CInspectorUI::GetInstance().SetGizmoMode(GizmoMode::Rotate);
        if (currentGizmoMode == GizmoMode::Rotate) ImGui::PopStyleColor();

        ImGui::SameLine();
        if (currentGizmoMode == GizmoMode::Scale) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        if (ImGui::Button("Scale (R)")) CInspectorUI::GetInstance().SetGizmoMode(GizmoMode::Scale);
        if (currentGizmoMode == GizmoMode::Scale) ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        // ---------------------------------------------------------
        // 4. Undo / Redo Controls
        // ---------------------------------------------------------
        bool canUndo = UndoManager::GetInstance().CanUndo();
        if (!canUndo) ImGui::BeginDisabled();
        if (ImGui::Button("Undo"))
        {
            UndoManager::GetInstance().Undo();
        }
        if (!canUndo) ImGui::EndDisabled();
        if (canUndo && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Undo (Ctrl+Z): %s", UndoManager::GetInstance().GetUndoName().c_str());
        }

        ImGui::SameLine();

        bool canRedo = UndoManager::GetInstance().CanRedo();
        if (!canRedo) ImGui::BeginDisabled();
        if (ImGui::Button("Redo"))
        {
            UndoManager::GetInstance().Redo();
        }
        if (!canRedo) ImGui::EndDisabled();
        if (canRedo && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Redo (Ctrl+Y): %s", UndoManager::GetInstance().GetRedoName().c_str());
        }

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        // ---------------------------------------------------------
        // 5. シーンのシリアライズ操作（保存 / 再読み込み）
        // ---------------------------------------------------------
        static char scenePathBuf[256] = "Assets/Scene/SceneTest.json";
        ImGui::SetNextItemWidth(140.0f);
        ImGui::InputText("##ScenePath", scenePathBuf, sizeof(scenePathBuf));

        ImGui::SameLine();
        if (ImGui::Button("Save Scene"))
        {
            Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();
            std::string path = scenePathBuf;
            if (SceneSerializer::SaveScene(path, activeScene))
            {
                hr.CaptureSceneSnapshot();
                CInspectorUI::GetInstance().SetStatusMessage("Scene saved: " + path, 3.0f);
            }
            else
            {
                CInspectorUI::GetInstance().SetStatusMessage("Failed to save scene: " + path, 3.0f);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reload Scene"))
        {
            Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();
            std::string path = scenePathBuf;
            if (SceneSerializer::LoadScene(path, activeScene))
            {
                hr.CaptureSceneSnapshot();
                CInspectorUI::GetInstance().SetStatusMessage("Scene loaded: " + path, 3.0f);
            }
            else
            {
                CInspectorUI::GetInstance().SetStatusMessage("Failed to load scene: " + path, 3.0f);
            }
        }

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        // ---------------------------------------------------------
        // 6. Hot-Reload & Build Center + UIサブウィンドウの開閉トグル
        // ---------------------------------------------------------
        bool isCompiling = hr.IsCompilingGameLogic();
        if (isCompiling) ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.48f, 0.78f, 1.0f));
        if (ImGui::Button("Hot Reload C++"))
        {
            hr.CompileAndReloadGameLogicAsync();
            CBuildSettingsUI::GetInstance().SetVisible(true);
        }
        ImGui::PopStyleColor();
        if (isCompiling) ImGui::EndDisabled();
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Compile Source/Game/* into GameLogic DLL & hot-reload in-place (Ctrl+Alt+F11)");
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.55f, 0.25f, 1.0f));
        if (ImGui::Button("Build / Reload"))
        {
            CBuildSettingsUI::GetInstance().ToggleVisible();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Viewport"))
        {
            CViewportUI::GetInstance().ToggleVisible();
        }
        ImGui::SameLine();
        if (ImGui::Button("Content Drawer"))
        {
            CContentDrawerUI::GetInstance().ToggleVisible();
        }
        ImGui::SameLine();
        if (ImGui::Button("Profiler"))
        {
            CProfilerUI::GetInstance().ToggleVisible();
        }
        ImGui::SameLine();
        if (ImGui::Button("Asset Encryptor"))
        {
            CAssetSecurityUI::GetInstance().ToggleVisible();
        }
    }
    ImGui::End();
}
