#include "HierarchyUI.h"
#include "InspectorUI.h"
#include "imgui.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "Transform.h"

void CHierarchyUI::Draw()
{
#ifndef _DEBUG
    return;
#endif // !_DEBUG

    if (!m_isVisible) return;

    ImGui::SetNextWindowSize(ImVec2(280, 450), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Hierarchy", &m_isVisible))
    {
        // 1. Create Empty Object Action
        if (ImGui::Button("+ Create Empty CObject"))
        {
            static int emptyCounter = 0;
            std::string name = "GameObject_" + std::to_string(++emptyCounter);
            CObject* newObj = new CObject(name);
            ObjectManager::GetInstance().AddObject(ObjectTag::NONE, newObj);
        }

        ImGui::Separator();

        // 2. Scene Object List
        const auto& objectList = ObjectManager::GetInstance().GetObjectList();
        int curSelTag = CInspectorUI::GetInstance().GetSelectedTagIndex();
        int curSelObj = CInspectorUI::GetInstance().GetSelectedObjectIndex();

        for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
        {
            const auto& objVec = objectList[tagIdx];
            if (objVec.empty()) continue;

            std::string tagHeaderName = "Tag: " + std::to_string(tagIdx);
            if (ImGui::CollapsingHeader(tagHeaderName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (size_t objIdx = 0; objIdx < objVec.size(); ++objIdx)
                {
                    const auto& obj = objVec[objIdx];
                    if (!obj || obj->GetIsDestroyed()) continue;

                    CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                    std::string objName = info ? info->GetObjectName() : "CObject";

                    bool isSelected = (curSelTag == static_cast<int>(tagIdx) && curSelObj == static_cast<int>(objIdx));

                    std::string label = objName + "##" + std::to_string(tagIdx) + "_" + std::to_string(objIdx);

                    if (ImGui::Selectable(label.c_str(), isSelected))
                    {
                        CInspectorUI::GetInstance().SetSelectedObject(static_cast<int>(tagIdx), static_cast<int>(objIdx));
                    }
                }
            }
        }
    }
    ImGui::End();
}
