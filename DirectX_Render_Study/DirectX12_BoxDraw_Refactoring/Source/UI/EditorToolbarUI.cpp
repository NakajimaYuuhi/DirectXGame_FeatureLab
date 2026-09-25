#include "EditorToolbarUI.h"
#include "InspectorUI.h"
#include "Source/Core/UndoManager.h"
#include "ContentDrawerUI.h"
#include "imgui.h"
#include "ObjectManager.h"
#include "TimeManager.h"
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

    ImGui::SetNextWindowSize(ImVec2(800, 55), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Editor Toolbar", &m_isVisible, ImGuiWindowFlags_NoScrollbar))
    {
        // ---------------------------------------------------------
        // 1. プレイモード / 編集モード / プレハブ編集モードの制御ボタン
        // ---------------------------------------------------------
        bool isPrefabMode = CInspectorUI::GetInstance().IsPrefabEditMode();
        if (isPrefabMode)
        {
            // プレハブ編集ステージ中の操作ボタン
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
        }
        else
        {
            // 通常シーンでの編集・再生・一時停止切替
            bool isEditMode = CInspectorUI::GetInstance().IsEditMode();
            bool isPaused = CInspectorUI::GetInstance().IsPaused();

            if (isEditMode)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button(" [Edit Mode] Click to Play "))
                {
                    CInspectorUI::GetInstance().SetEditMode(false);
                }
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
                if (ImGui::Button(" [PLAYING] Click to Edit "))
                {
                    CInspectorUI::GetInstance().SetEditMode(true);
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if (ImGui::Button(isPaused ? " Resume " : " Pause "))
                {
                    CInspectorUI::GetInstance().SetPaused(!isPaused);
                }
            }
        }

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        // ---------------------------------------------------------
        // 2. ギズモ操作モード切り替え（移動: W / 回転: E / 拡大縮小: R）
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

        // 3. Undo / Redo Controls
        bool canUndo = UndoManager::GetInstance().CanUndo();
        if (!canUndo) ImGui::BeginDisabled();
        if (ImGui::Button("Undo (Ctrl+Z)"))
        {
            UndoManager::GetInstance().Undo();
        }
        if (!canUndo) ImGui::EndDisabled();
        if (canUndo && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Undo: %s", UndoManager::GetInstance().GetUndoName().c_str());
        }

        ImGui::SameLine();

        bool canRedo = UndoManager::GetInstance().CanRedo();
        if (!canRedo) ImGui::BeginDisabled();
        if (ImGui::Button("Redo (Ctrl+Y)"))
        {
            UndoManager::GetInstance().Redo();
        }
        if (!canRedo) ImGui::EndDisabled();
        if (canRedo && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Redo: %s", UndoManager::GetInstance().GetRedoName().c_str());
        }

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        // 4. Save / Reload Scene

        // ---------------------------------------------------------
        // 3. シーンのシリアライズ操作（保存 / 再読み込み）
        // ---------------------------------------------------------
        if (ImGui::Button("Save Scene"))
        {
            Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();
            std::string path = "Assets/Scene/SceneTest.json";
            SceneSerializer::SaveScene(path, activeScene);
        }

        ImGui::SameLine();
        if (ImGui::Button("Reload Scene"))
        {
            Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();
            std::string path = "Assets/Scene/SceneTest.json";
            SceneSerializer::LoadScene(path, activeScene);
        }

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        // ---------------------------------------------------------
        // 4. UIサブウィンドウの開閉トグル（コンテンツドロワー）
        // ---------------------------------------------------------
        if (ImGui::Button("Content Drawer (Ctrl+Space)"))
        {
            CContentDrawerUI::GetInstance().ToggleVisible();
        }
    }
    ImGui::End();
}
