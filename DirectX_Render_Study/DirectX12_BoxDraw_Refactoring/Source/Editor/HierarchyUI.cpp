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

    ImGui::SetNextWindowSize(ImVec2(320, 500), ImGuiCond_FirstUseEver);
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

        ImGui::SameLine();
        ImGui::TextDisabled("(Drag to Parent)");

        ImGui::Separator();

        // 2. 逆引きマップ（CObject* -> { tagIdx, objIdx }）の構築
        const auto& objectList = ObjectManager::GetInstance().GetObjectList();
        std::unordered_map<CObject*, std::pair<int, int>> objIndexMap;
        for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
        {
            const auto& objVec = objectList[tagIdx];
            for (size_t objIdx = 0; objIdx < objVec.size(); ++objIdx)
            {
                if (objVec[objIdx] && !objVec[objIdx]->GetIsDestroyed())
                {
                    objIndexMap[objVec[objIdx].get()] = { static_cast<int>(tagIdx), static_cast<int>(objIdx) };
                }
            }
        }

        // 3. Scene Object Tree (タグごとに描画)
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

                    // 親が存在するオブジェクトは親ノード内で再帰的に描画されるため、トップレベルではスキップ
                    CTransform* trans = obj->GetComponent<CTransform>();
                    if (trans && trans->GetParent() != nullptr)
                    {
                        continue;
                    }

                    // ルートオブジェクトの描画
                    DrawObjectNode(obj.get(), static_cast<int>(tagIdx), static_cast<int>(objIdx), objIndexMap);
                }
            }
        }

        ImGui::Separator();

        // 4. ルート解除用ドロップゾーン
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.4f));
        ImGui::Button("[ Drop Here to Detach to Root ]", ImVec2(-1.0f, 26.0f));
        ImGui::PopStyleColor();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_OBJECT_PTR"))
            {
                CObject* droppedObj = *(CObject**)payload->Data;
                if (droppedObj)
                {
                    CTransform* droppedTrans = droppedObj->GetComponent<CTransform>();
                    if (droppedTrans)
                    {
                        droppedTrans->SetParent(nullptr, true); // ワールド姿勢を維持してルートに戻す
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}

void CHierarchyUI::DrawObjectNode(
    CObject* obj,
    int tagIdx,
    int objIdx,
    const std::unordered_map<CObject*, std::pair<int, int>>& objIndexMap)
{
    if (!obj || obj->GetIsDestroyed()) return;

    CObjectInfo* info = obj->GetComponent<CObjectInfo>();
    std::string objName = info ? info->GetObjectName() : "CObject";
    std::string pName = info ? info->GetPrefabName() : "";
    CTransform* trans = obj->GetComponent<CTransform>();

    int curSelTag = CInspectorUI::GetInstance().GetSelectedTagIndex();
    int curSelObj = CInspectorUI::GetInstance().GetSelectedObjectIndex();
    bool isSelected = (curSelTag == tagIdx && curSelObj == objIdx);

    // 子オブジェクトのリストを確認
    const std::vector<CTransform*>* children = trans ? &trans->GetChildren() : nullptr;
    bool hasChildren = children && !children->empty();

    // TreeNode フラグ
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (isSelected)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!hasChildren)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    // 表示名
    std::string labelText = objName;
    if (!pName.empty())
    {
        labelText += " [P]";
    }

    // ノードID（ポインタアドレスで一意化）
    std::string nodeID = labelText + "###Node_" + std::to_string(reinterpret_cast<uintptr_t>(obj));

    bool nodeOpen = ImGui::TreeNodeEx(nodeID.c_str(), nodeFlags);

    // クリックでインスペクター選択
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        CInspectorUI::GetInstance().SetSelectedObject(tagIdx, objIdx);
    }

    // --- ドラッグ元（Drag Source） ---
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload("HIERARCHY_OBJECT_PTR", &obj, sizeof(CObject*));
        ImGui::Text("Move: %s", objName.c_str());
        ImGui::EndDragDropSource();
    }

    // --- ドロップ先（Drop Target: このオブジェクトの子にする） ---
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_OBJECT_PTR"))
        {
            CObject* droppedObj = *(CObject**)payload->Data;
            if (droppedObj && droppedObj != obj && trans)
            {
                CTransform* droppedTrans = droppedObj->GetComponent<CTransform>();
                if (droppedTrans)
                {
                    // ワールド姿勢を維持して親子付け（循環参照は内部で安全に無視される）
                    droppedTrans->SetParent(trans, true);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    // --- 右クリック コンテキストメニュー ---
    if (ImGui::BeginPopupContextItem())
    {
        CInspectorUI::GetInstance().SetSelectedObject(tagIdx, objIdx);

        if (trans && trans->GetParent() != nullptr)
        {
            if (ImGui::MenuItem("Detach from Parent"))
            {
                trans->SetParent(nullptr, true);
            }
        }

        if (ImGui::MenuItem("Delete Object"))
        {
            obj->SetIsDestroyed(true);
        }
        ImGui::EndPopup();
    }

    // --- 子ノードの再帰展開 ---
    if (hasChildren && nodeOpen)
    {
        for (auto* childTrans : *children)
        {
            if (!childTrans) continue;
            CObject* childObj = childTrans->GetOwner();
            if (!childObj || childObj->GetIsDestroyed()) continue;

            auto it = objIndexMap.find(childObj);
            if (it != objIndexMap.end())
            {
                DrawObjectNode(childObj, it->second.first, it->second.second, objIndexMap);
            }
        }
        ImGui::TreePop();
    }
}