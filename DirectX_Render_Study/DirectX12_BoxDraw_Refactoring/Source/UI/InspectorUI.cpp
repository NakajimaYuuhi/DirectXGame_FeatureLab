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
#include "SceneEnums.h"
#include <typeinfo>

bool CInspectorUI::ShouldUpdateGame()
{
#ifndef _DEBUG
    return true;
#endif // !_DEBUG

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

    ImGui::Begin("Scene Inspector");

    auto& objectList = ObjectManager::GetInstance().GetObjectList();

    // 1. Game & Time Controls
    ImGui::Text("Game & Time Controls");
    if (ImGui::Button(m_isPaused ? "  Resume  " : "  Pause  "))
    {
        m_isPaused = !m_isPaused;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step 1 Frame"))
    {
        m_stepNextFrame = true;
    }
    ImGui::SameLine();
    if (ImGui::SliderFloat("Speed", &m_timeScale, 0.0f, 3.0f, "%.2fx"))
    {
        TimeManager::GetInstance().SetTimeScale(m_timeScale);
    }
    ImGui::Checkbox("Show Box Colliders", &m_showColliders);
    ImGui::Separator();

    // 2. Scene Controls
    ImGui::Text("Scene Controls");
    static const char* sceneNames[] = { "Title (TITLE)", "Test/Game (TEST)", "Clear (Clear)", "Failed (Failed)" };
    static const Scenes::ID sceneIDs[] = { Scenes::ID::TITLE, Scenes::ID::TEST, Scenes::ID::Clear, Scenes::ID::Failed };
    static int selectedSceneIndex = 0;

    ImGui::Combo("Scene List", &selectedSceneIndex, sceneNames, IM_ARRAYSIZE(sceneNames));

    if (ImGui::Button("Change Scene (Fade)"))
    {
        SceneManager::GetInstance().ChangeSceneWithFade(sceneIDs[selectedSceneIndex], 0.4f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Additive"))
    {
        SceneManager::GetInstance().LoadSceneAdditive(sceneIDs[selectedSceneIndex], true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Unload Selected"))
    {
        SceneManager::GetInstance().UnloadScene(sceneIDs[selectedSceneIndex]);
    }
    ImGui::Separator();

    // 3. Hierarchy (Object List)
    ImGui::Text("Hierarchy");
    ImGui::Separator();
    
    ImGui::BeginChild("HierarchyList", ImVec2(0, 200), true);
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
                if (!obj) continue;

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

    // 4. Inspector (Selected Object Details)
    ImGui::Spacing();
    ImGui::Text("Inspector");
    ImGui::Separator();

    if (m_selectedTagIndex >= 0 && m_selectedTagIndex < objectList.size())
    {
        const auto& objVec = objectList[m_selectedTagIndex];
        if (m_selectedObjectIndex >= 0 && m_selectedObjectIndex < objVec.size())
        {
            CObject* selectedObj = objVec[m_selectedObjectIndex].get();
            if (selectedObj)
            {
                ImGui::Text("Object (Tag %d, Idx %d)", m_selectedTagIndex, m_selectedObjectIndex);
                
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
                
                if (ImGui::CollapsingHeader("Other Components", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (const auto& comp : selectedObj->GetComponents())
                    {
                        if (comp)
                        {
                            std::string compName = comp->GetName();
                            if (compName != "Transform" && compName != "Model" && compName != "ObjectInfo" && compName != "BoxCollider3D")
                            {
                                if (compName.empty()) {
                                    compName = typeid(*comp).name();
                                    if (compName.find("class ") == 0) compName = compName.substr(6);
                                    if (compName.find("struct ") == 0) compName = compName.substr(7);
                                }
                                ImGui::Text("- %s", compName.c_str());
                            }
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