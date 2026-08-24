#include "InspectorUI.h"
#include "imgui.h"
#include "ObjectManager.h"
#include "Transform.h"
#include "Model.h"

void CInspectorUI::Draw()
{
    ImGui::Begin("Scene Inspector");

    auto& objectList = ObjectManager::GetInstance().GetObjectList();

    // 1. Hierarchy (Object List)
    ImGui::Text("Hierarchy");
    ImGui::Separator();
    
    ImGui::BeginChild("HierarchyList", ImVec2(0, 200), true);
    int objectCounter = 0;
    
    for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
    {
        const auto& objVec = objectList[tagIdx];
        if (objVec.empty()) continue;

        // タグごとに折りたたみヘッダを表示
        if (ImGui::CollapsingHeader(("Tag: " + std::to_string(tagIdx)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (size_t i = 0; i < objVec.size(); ++i)
            {
                CObject* obj = objVec[i].get();
                if (!obj) continue;

                // 選択可能アイテムとして表示
                std::string label = "Object " + std::to_string(objectCounter) + "##" + std::to_string(tagIdx) + "_" + std::to_string(i);

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

    // 2. Inspector (Selected Object Details)
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
                
                // Transform Component
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
                        // 表示は度数法にするかラジアンにするかだが、一旦そのまま
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

                // Model Component (Shader swapping)
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
            }
        }
    }
    else
    {
        ImGui::TextDisabled("Select an object from Hierarchy.");
    }

    ImGui::End();
}
