#include "EditorToolbarUI.h"
#include "InspectorUI.h"
#include "ContentDrawerUI.h"
#include "imgui.h"
#include "ObjectManager.h"
#include "TimeManager.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"

void CEditorToolbarUI::Draw()
{
#ifndef _DEBUG
    return;
#endif // !_DEBUG

    if (!m_isVisible) return;

    ImGui::SetNextWindowSize(ImVec2(800, 55), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Editor Toolbar", &m_isVisible, ImGuiWindowFlags_NoScrollbar))
    {
        // 1. Play / Edit Mode Controls
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
        }
        else
        {
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

        // 2. Gizmo Mode Selection
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

        // 3. Scene Operations
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

        // 4. UI Window Toggles
        if (ImGui::Button("Content Drawer (Ctrl+Space)"))
        {
            CContentDrawerUI::GetInstance().ToggleVisible();
        }
    }
    ImGui::End();
}
