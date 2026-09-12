#include "InspectorUI.h"
#include "imgui.h"
#include "ObjectManager.h"
#include "Transform.h"
#include "Model.h"
#include "ObjectInfo.h"
#include "BoxCollider3D.h"
#include "Camera.h"
#include "TimeManager.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"
#include "SceneEnums.h"
#include "Player.h"
#include "Enemy.h"
#include "Box.h"
#include <typeinfo>

bool CInspectorUI::ShouldUpdateGame()
{
#ifndef _DEBUG
    return true;
#endif // !_DEBUG

    if (m_isEditMode)
    {
        return false;
    }

    if (!m_isPaused)
    {
        return true;
    }

    if (m_stepNextFrame)
    {
        m_stepNextFrame = false;
        return true;
    }

    return false;
}

void CInspectorUI::Draw()
{
#ifndef _DEBUG
    return;
#endif // !_DEBUG

    ImGui::Begin("Level Editor & Inspector");

    auto& objectList = ObjectManager::GetInstance().GetObjectList();
    Scenes::ID currentSceneID = SceneManager::GetInstance().GetActiveSceneID();

    // 1. Mode Controls
    ImGui::Text("Mode & Simulation");
    if (m_isEditMode)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("  [EDIT MODE] Click to Play  "))
        {
            m_isEditMode = false;
            // Play Mode 開始時に Awake と Start を呼ぶ
            for (auto& vec : objectList)
            {
                for (auto& obj : vec)
                {
                    if (obj && !obj->GetIsDestroyed())
                    {
                        if (!obj->GetHasAwoken()) obj->Awake();
                        if (!obj->GetHasStarted()) obj->Start();
                    }
                }
            }
        }
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        if (ImGui::Button("  [PLAY MODE] Click to Edit  "))
        {
            m_isEditMode = true;
            // Edit Mode 復帰時に JSON から復允E            SceneSerializer::LoadScene(m_sceneJsonPath, currentSceneID);
        }
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    if (ImGui::Button(m_isPaused ? " Resume " : " Pause "))
    {
        m_isPaused = !m_isPaused;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step 1 Frame"))
    {
        m_stepNextFrame = true;
    }

    if (ImGui::SliderFloat("Speed", &m_timeScale, 0.0f, 3.0f, "%.2fx"))
    {
        TimeManager::GetInstance().SetTimeScale(m_timeScale);
    }
    ImGui::Checkbox("Show Box Colliders", &m_showColliders);
    ImGui::Separator();

    // 2. Prefab Palette (Object Spawner)
    ImGui::Text("Prefab Spawner (Add Objects)");
    if (ImGui::Button("+ Player"))
    {
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::PLAYER, "Player");
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ Enemy"))
    {
        static int enemyCounter = 0;
        std::string name = "Enemy_" + std::to_string(enemyCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::ENEMY, name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ Box Field"))
    {
        static int boxCounter = 0;
        std::string name = "Box_" + std::to_string(boxCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::FIELD, name);
        if (newObj) newObj->Awake();
    }
    ImGui::Separator();

    // 3. JSON Scene Serialization
    ImGui::Text("Scene Serialization (JSON)");
    ImGui::InputText("File Path", m_sceneJsonPath, sizeof(m_sceneJsonPath));

    if (ImGui::Button("Save Scene (.json)"))
    {
        SceneSerializer::SaveScene(m_sceneJsonPath, currentSceneID);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Scene (.json)"))
    {
        SceneSerializer::LoadScene(m_sceneJsonPath, currentSceneID);
    }
    ImGui::Separator();

    // Player Status
    Player* player = ObjectManager::GetInstance().GetPlayer();
    if (player)
    {
        ImGui::Text("Player Status");
        ImGui::Text("HP: %d / %d", player->GetHP(), player->GetMaxHP());
        float hpFraction = (float)player->GetHP() / (float)player->GetMaxHP();
        ImGui::ProgressBar(hpFraction, ImVec2(-1.0f, 0.0f));
        ImGui::Separator();
    }

    // 4. Hierarchy (Object List)
    ImGui::Text("Hierarchy");
    ImGui::BeginChild("HierarchyList", ImVec2(0, 180), true);
    int objectCounter = 0;
    
    for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
    {
        const auto& objVec = objectList[tagIdx];
        if (objVec.empty()) continue;

        if (ImGui::CollapsingHeader(("Tag: " + std::to_string(tagIdx)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (size_t i = 0; i < objVec.size(); ++i)
            {
                CObject* obj = objVec[i].get();
                if (!obj || obj->GetIsDestroyed()) continue;

                CObjectInfo* objInfo = obj->GetComponent<CObjectInfo>();
                std::string objName = objInfo ? objInfo->GetObjectName() : "Object " + std::to_string(objectCounter);
                std::string label = objName + "##" + std::to_string(tagIdx) + "_" + std::to_string(i);

                bool isSelected = (m_selectedTagIndex == tagIdx && m_selectedObjectIndex == i);
                if (ImGui::Selectable(label.c_str(), isSelected))
                {
                    m_selectedTagIndex = tagIdx;
                    m_selectedObjectIndex = i;
                }
                objectCounter++;
            }
        }
    }
    ImGui::EndChild();

    // 5. Inspector (Selected Object Details)
    ImGui::Spacing();
    ImGui::Text("Inspector");
    ImGui::Separator();

    if (m_selectedTagIndex >= 0 && m_selectedTagIndex < objectList.size())
    {
        const auto& objVec = objectList[m_selectedTagIndex];
        if (m_selectedObjectIndex >= 0 && m_selectedObjectIndex < objVec.size())
        {
            CObject* selectedObj = objVec[m_selectedObjectIndex].get();
            if (selectedObj && !selectedObj->GetIsDestroyed())
            {
                CObjectInfo* objInfo = selectedObj->GetComponent<CObjectInfo>();
                std::string name = objInfo ? objInfo->GetObjectName() : "Object";
                ImGui::Text("Selected: %s (Tag %d, Idx %d)", name.c_str(), m_selectedTagIndex, m_selectedObjectIndex);
                
                // Duplicate & Delete buttons
                if (ImGui::Button("Duplicate Object"))
                {
                    CObjectInfo* selectedInfo = selectedObj->GetComponent<CObjectInfo>();
                    CTransform* selectedTrans = selectedObj->GetComponent<CTransform>();

                    if (selectedInfo && selectedTrans)
                    {
                        CObject* clonedObj = ObjectManager::GetInstance().Instantiate(currentSceneID, selectedInfo->GetObjectTag(), selectedInfo->GetObjectName() + "_Copy");
                        if (clonedObj)
                        {
                            CTransform* clonedTrans = clonedObj->GetComponent<CTransform>();
                            if (clonedTrans)
                            {
                                DirectX::XMFLOAT3 pos = selectedTrans->GetPos();
                                pos.x += 0.5f; // offset slightly
                                clonedTrans->SetPos(pos);
                                clonedTrans->SetRotation(selectedTrans->GetRotation());
                                clonedTrans->SetScale(selectedTrans->GetScale());
                            }
                            clonedObj->Awake();
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Object"))
                {
                    selectedObj->SetIsDestroyed(true);
                    m_selectedObjectIndex = -1;
                    m_selectedTagIndex = -1;
                }

                CTransform* transform = selectedObj->GetComponent<CTransform>();
                if (transform)
                {
                    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        DirectX::XMFLOAT3 pos = transform->GetPos();
                        if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                        {
                            transform->SetPos(pos);
                        }

                        DirectX::XMFLOAT3 rot = transform->GetRotation();
                        if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f))
                        {
                            transform->SetRotation(rot);
                        }

                        DirectX::XMFLOAT3 scale = transform->GetScale();
                        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                        {
                            transform->SetScale(scale);
                        }
                    }
                }

                CModel* model = selectedObj->GetComponent<CModel>();
                if (model)
                {
                    if (ImGui::CollapsingHeader("Model & Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::InputText("Shader Path", m_shaderPathInput, sizeof(m_shaderPathInput));
                        if (ImGui::Button("Apply Shader"))
                        {
                            std::string pathStr(m_shaderPathInput);
                            std::wstring pathW(pathStr.begin(), pathStr.end());
                            model->SetShaderAll(pathW);
                        }
                    }
                }

                BoxCollider3D* boxCollider = selectedObj->GetComponent<BoxCollider3D>();
                if (boxCollider)
                {
                    if (ImGui::CollapsingHeader("BoxCollider3D", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        DirectX::XMFLOAT3 size = boxCollider->GetSize();
                        if (ImGui::DragFloat3("Size", &size.x, 0.1f))
                        {
                            boxCollider->SetSize(size);
                        }

                        DirectX::XMFLOAT3 offset = boxCollider->GetOffset();
                        if (ImGui::DragFloat3("Offset", &offset.x, 0.1f))
                        {
                            boxCollider->SetOffset(offset);
                        }
                    }
                }
            }
        }
    }
    else
    {
        ImGui::TextDisabled("Select an object from Hierarchy.");
    }

    ImGui::End();

    if (m_showColliders)
    {
        Camera* camera = ObjectManager::GetInstance().GetCamera();
        if (camera)
        {
            for (const auto& objVec : objectList)
            {
                for (const auto& obj : objVec)
                {
                    if (!obj || obj->GetIsDestroyed()) continue;
                    BoxCollider3D* boxCol = obj->GetComponent<BoxCollider3D>();
                    if (boxCol)
                    {
                        boxCol->DrawDebug(camera);
                    }
                }
            }
        }
    }
}