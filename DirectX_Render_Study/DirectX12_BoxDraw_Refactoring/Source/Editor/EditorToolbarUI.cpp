#include "EditorToolbarUI.h"
#include "InspectorUI.h"
#include "UndoManager.h"
#include "ContentDrawerUI.h"
#include "imgui.h"
#include "ObjectManager.h"
#include "TimeManager.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"

// -----------------------------------------------------------------
// 繝・・繝ｫ繝舌・UI縺ｮ謠冗判譖ｴ譁ｰ蜃ｦ逅・ｼ医ョ繝舌ャ繧ｰ繝薙Ν繝画凾縺ｮ縺ｿ譛牙柑
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
        // 1. 繝励Ξ繧､繝｢繝ｼ繝・/ 邱ｨ髮・Δ繝ｼ繝・/ 繝励Ξ繝上ヶ邱ｨ髮・Δ繝ｼ繝峨・蛻ｶ蠕｡繝懊ち繝ｳ
        // ---------------------------------------------------------
        bool isPrefabMode = CInspectorUI::GetInstance().IsPrefabEditMode();
        if (isPrefabMode)
        {
            // 繝励Ξ繝上ヶ邱ｨ髮・せ繝・・繧ｸ荳ｭ縺ｮ謫堺ｽ懊・繧ｿ繝ｳ
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
            // 騾壼ｸｸ繧ｷ繝ｼ繝ｳ縺ｧ縺ｮ邱ｨ髮・・蜀咲函繝ｻ荳譎ょ●豁｢蛻・崛
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
        // 2. 繧ｮ繧ｺ繝｢謫堺ｽ懊Δ繝ｼ繝牙・繧頑崛縺茨ｼ育ｧｻ蜍・ W / 蝗櫁ｻ｢: E / 諡｡螟ｧ邵ｮ蟆・ R
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
        // 3. 繧ｷ繝ｼ繝ｳ縺ｮ繧ｷ繝ｪ繧｢繝ｩ繧､繧ｺ謫堺ｽ懶ｼ井ｿ晏ｭ・/ 蜀崎ｪｭ縺ｿ霎ｼ縺ｿ・・        // ---------------------------------------------------------
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
        // 4. UI繧ｵ繝悶え繧｣繝ｳ繝峨え縺ｮ髢矩哩繝医げ繝ｫ・医さ繝ｳ繝・Φ繝・ラ繝ｭ繝ｯ繝ｼ・・        // ---------------------------------------------------------
        if (ImGui::Button("Content Drawer (Ctrl+Space)"))
        {
            CContentDrawerUI::GetInstance().ToggleVisible();
        }
    }
    ImGui::End();
}
