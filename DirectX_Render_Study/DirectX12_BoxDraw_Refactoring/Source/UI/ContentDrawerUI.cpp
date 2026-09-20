#include "ContentDrawerUI.h"
#include "InspectorUI.h"
#include "imgui.h"
#include "PrefabManager.h"
#include "ObjectManager.h"
#include "Camera.h"
#include "Transform.h"
#include "ObjectInfo.h"
#include "InputManager.h"
#include "Source/External/json.hpp"
#include <fstream>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

void CContentDrawerUI::RefreshPrefabList()
{
    m_prefabItems.clear();
    std::string folderPath = "Assets/Prefabs";

    if (fs::exists(folderPath) && fs::is_directory(folderPath))
    {
        for (const auto& entry : fs::directory_iterator(folderPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                PrefabItem item;
                item.path = entry.path().string();
                // Replace backslashes with forward slashes
                std::replace(item.path.begin(), item.path.end(), '\\', '/');
                item.name = entry.path().stem().string();

                std::ifstream file(item.path);
                if (file.is_open())
                {
                    try
                    {
                        nlohmann::json j;
                        file >> j;
                        item.tag = j.value("Tag", "NONE");
                    }
                    catch (...)
                    {
                        item.tag = "NONE";
                    }
                }

                m_prefabItems.push_back(item);
            }
        }
    }
    m_isInitialized = true;
}

void CContentDrawerUI::Draw()
{
    // Toggle via Ctrl + Space
    if (CInputManager::GetInstance().IsKeyPress(VK_CONTROL) && CInputManager::GetInstance().IsKeyTrigger(VK_SPACE))
    {
        ToggleVisible();
    }

    if (!m_isVisible) return;

    if (!m_isInitialized)
    {
        RefreshPrefabList();
    }

    ImGui::SetNextWindowSize(ImVec2(650, 260), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Content Drawer (Prefabs)", &m_isVisible, ImGuiWindowFlags_NoCollapse))
    {
        if (ImGui::Button("Refresh"))
        {
            RefreshPrefabList();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("Filter", m_searchFilter, sizeof(m_searchFilter));

        ImGui::SameLine();
        ImGui::TextDisabled("Shortcut: Ctrl + Space");

        ImGui::Separator();

        float windowWidth = ImGui::GetContentRegionAvail().x;
        float cardWidth = 180.0f;
        int columns = static_cast<int>(windowWidth / (cardWidth + 12.0f));
        if (columns < 1) columns = 1;

        if (ImGui::BeginTable("PrefabGrid", columns))
        {
            for (size_t i = 0; i < m_prefabItems.size(); ++i)
            {
                const auto& item = m_prefabItems[i];

                if (m_searchFilter[0] != '\0')
                {
                    if (item.name.find(m_searchFilter) == std::string::npos &&
                        item.tag.find(m_searchFilter) == std::string::npos)
                    {
                        continue;
                    }
                }

                ImGui::TableNextColumn();
                ImGui::PushID(static_cast<int>(i));

                ImGui::BeginGroup();
                
                // Prefab Card UI
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "[Prefab] %s", item.name.c_str());
                ImGui::TextDisabled("Tag: %s", item.tag.c_str());

                // Edit Prefab Button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.1f, 1.0f));
                if (ImGui::Button("Edit Prefab"))
                {
                    CInspectorUI::GetInstance().OpenPrefabEditMode(item.path);
                }
                ImGui::PopStyleColor();

                // Spawn at Camera Front
                if (ImGui::Button("Spawn (Cam Front)"))
                {
                    Camera* cam = ObjectManager::GetInstance().GetCamera();
                    DirectX::XMFLOAT3 spawnPos = { 0.0f, 0.0f, 0.0f };
                    if (cam)
                    {
                        CTransform* camTransform = cam->GetComponent<CTransform>();
                        if (camTransform)
                        {
                            DirectX::XMFLOAT3 pos = camTransform->GetPos();
                            DirectX::XMFLOAT3 front = camTransform->GetFront();
                            spawnPos = { pos.x + front.x * 3.0f, pos.y + front.y * 3.0f, pos.z + front.z * 3.0f };
                        }
                    }

                    static int spawnCounter = 0;
                    std::string instanceName = item.name + "_" + std::to_string(++spawnCounter);

                    CObject* newObj = PrefabManager::GetInstance().InstantiateFromJSON(item.path, instanceName);
                    if (newObj)
                    {
                        CTransform* t = newObj->GetComponent<CTransform>();
                        if (t) t->SetPos(spawnPos);

                        CObjectInfo* info = newObj->GetComponent<CObjectInfo>();
                        ObjectTag tag = info ? info->GetObjectTag() : ObjectTag::NONE;

                        ObjectManager::GetInstance().AddObject(tag, newObj);
                    }
                }

                // Spawn at Origin
                if (ImGui::Button("Spawn (Origin)"))
                {
                    static int spawnCounter = 0;
                    std::string instanceName = item.name + "_Origin_" + std::to_string(++spawnCounter);

                    CObject* newObj = PrefabManager::GetInstance().InstantiateFromJSON(item.path, instanceName);
                    if (newObj)
                    {
                        CObjectInfo* info = newObj->GetComponent<CObjectInfo>();
                        ObjectTag tag = info ? info->GetObjectTag() : ObjectTag::NONE;

                        ObjectManager::GetInstance().AddObject(tag, newObj);
                    }
                }

                ImGui::EndGroup();
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}
