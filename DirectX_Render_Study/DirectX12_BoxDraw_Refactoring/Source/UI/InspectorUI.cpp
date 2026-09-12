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
#include "UIObject.h"
#include "CUIButton.h"
#include "TextObject.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "ButtonEventManager.h"
#include "ButtonAction.h"
#include <typeinfo>
#include <windows.h>
#include <vector>

static std::string WStringToString(const std::wstring& wstr)
{
	if (wstr.empty()) return "";
	int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string str(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size, NULL, NULL);
	return str;
}

static std::wstring StringToWString(const std::string& str)
{
	if (str.empty()) return L"";
	int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size);
	return wstr;
}

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
            // Play Mode Start
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
            // Edit Mode Reload
            SceneSerializer::LoadScene(m_sceneJsonPath, currentSceneID);
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

    // 2. Scene Controls
    ImGui::Text("Scene Controls");
    static const char* sceneNames[] = { "Title (TITLE)", "Test/Game (TEST)", "Clear (Clear)", "Failed (Failed)" };
    static const Scenes::ID sceneIDs[] = { Scenes::ID::TITLE, Scenes::ID::TEST, Scenes::ID::Clear, Scenes::ID::Failed };
    static const char* sceneJsonPaths[] = { "Assets/Scene/SceneTitle.json", "Assets/Scene/SceneTest.json", "Assets/Scene/SceneClear.json", "Assets/Scene/SceneFailed.json" };
    static int selectedSceneIndex = 1; // Default to TEST

    if (ImGui::Combo("Scene List", &selectedSceneIndex, sceneNames, IM_ARRAYSIZE(sceneNames)))
    {
        strncpy_s(m_sceneJsonPath, sizeof(m_sceneJsonPath), sceneJsonPaths[selectedSceneIndex], _TRUNCATE);
    }

    if (ImGui::Button("Change Scene"))
    {
        if (m_isEditMode)
        {
            SceneManager::GetInstance().ChangeSceneInstant(sceneIDs[selectedSceneIndex]);
        }
        else
        {
            SceneManager::GetInstance().ChangeSceneWithFade(sceneIDs[selectedSceneIndex], 0.4f);
        }
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

    // 2.5 UI Event System (First Selected Button)
    ImGui::Text("UI Event System");
    std::vector<std::string> uiButtonNames;
    int selectedFirstIdx = 0;
    std::string currentFirstSel = ButtonEventManager::GetInstance().GetFirstSelectedName();

    int btnCounter = 0;
    for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
    {
        for (const auto& obj : objectList[tagIdx])
        {
            if (!obj || obj->GetIsDestroyed()) continue;
            if (dynamic_cast<CUIButton*>(obj.get()))
            {
                CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                std::string bName = info ? info->GetObjectName() : "Button";
                uiButtonNames.push_back(bName);
                if (bName == currentFirstSel)
                {
                    selectedFirstIdx = btnCounter;
                }
                btnCounter++;
            }
        }
    }

    if (!uiButtonNames.empty())
    {
        std::vector<const char*> btnPtrs;
        for (const auto& name : uiButtonNames) btnPtrs.push_back(name.c_str());

        if (ImGui::Combo("First Selected Button", &selectedFirstIdx, btnPtrs.data(), (int)btnPtrs.size()))
        {
            ButtonEventManager::GetInstance().SetFirstSelectedName(uiButtonNames[selectedFirstIdx]);
        }
    }
    else
    {
        ImGui::TextDisabled("No UI Buttons in scene");
    }
    ImGui::Separator();

    // 3. Prefab Palette (Object Spawner)
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
    ImGui::SameLine();
    if (ImGui::Button("+ UI Image"))
    {
        static int uiCounter = 0;
        std::string name = "UIImage_" + std::to_string(uiCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::UI, name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ UI Button"))
    {
        static int btnCounter = 0;
        std::string name = "UIButton_" + std::to_string(btnCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::UI, name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ UI Text"))
    {
        static int txtCounter = 0;
        std::string name = "Text_" + std::to_string(txtCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::TEXT, name);
        if (newObj) newObj->Awake();
    }
    ImGui::Separator();

    // 4. JSON Scene Serialization
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

    // 5. Hierarchy (Object List)
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

                bool isSelected = (m_selectedTagIndex == (int)tagIdx && m_selectedObjectIndex == (int)i);
                if (ImGui::Selectable(label.c_str(), isSelected))
                {
                    m_selectedTagIndex = (int)tagIdx;
                    m_selectedObjectIndex = (int)i;
                }
                objectCounter++;
            }
        }
    }
    ImGui::EndChild();

    // 6. Inspector (Selected Object Details)
    ImGui::Spacing();
    ImGui::Text("Inspector");
    ImGui::Separator();

    if (m_selectedTagIndex >= 0 && m_selectedTagIndex < (int)objectList.size())
    {
        const auto& objVec = objectList[m_selectedTagIndex];
        if (m_selectedObjectIndex >= 0 && m_selectedObjectIndex < (int)objVec.size())
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
                                pos.x += 0.5f;
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

                CSpriteRenderer* sprite = selectedObj->GetComponent<CSpriteRenderer>();
                if (sprite)
                {
                    if (ImGui::CollapsingHeader("SpriteRenderer", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        std::string texPath = WStringToString(sprite->GetTexturePath());
                        char texBuf[256];
                        strncpy_s(texBuf, sizeof(texBuf), texPath.c_str(), _TRUNCATE);
                        if (ImGui::InputText("Texture Path", texBuf, sizeof(texBuf)))
                        {
                            sprite->SetTexture(StringToWString(std::string(texBuf)));
                        }

                        DirectX::XMFLOAT2 sz = sprite->GetSize();
                        float sizeArr[2] = { sz.x, sz.y };
                        if (ImGui::DragFloat2("Size (W, H)", sizeArr, 1.0f))
                        {
                            sprite->SetSize(sizeArr[0], sizeArr[1]);
                        }

                        DirectX::XMFLOAT4 col = sprite->GetColor();
                        float colorArr[4] = { col.x, col.y, col.z, col.w };
                        if (ImGui::ColorEdit4("Color", colorArr))
                        {
                            sprite->SetColor({ colorArr[0], colorArr[1], colorArr[2], colorArr[3] });
                        }
                    }
                }

                CTextRenderer* textComp = selectedObj->GetComponent<CTextRenderer>();
                if (textComp)
                {
                    if (ImGui::CollapsingHeader("TextRenderer", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        std::string contentStr = WStringToString(textComp->GetText());
                        char textBuf[512];
                        strncpy_s(textBuf, sizeof(textBuf), contentStr.c_str(), _TRUNCATE);
                        if (ImGui::InputText("Text Content", textBuf, sizeof(textBuf)))
                        {
                            textComp->SetText(StringToWString(std::string(textBuf)));
                        }

                        CTransform* trans = selectedObj->GetComponent<CTransform>();
                        if (trans)
                        {
                            DirectX::XMFLOAT3 tPos = trans->GetPos();
                            float posArr[2] = { tPos.x, tPos.y };
                            if (ImGui::DragFloat2("Text Pos (X, Y)", posArr, 1.0f))
                            {
                                trans->SetPos({ posArr[0], posArr[1], 0.0f });
                            }
                        }

                        float fsz = textComp->GetFontSize();
                        if (ImGui::DragFloat("Font Size", &fsz, 1.0f, 8.0f, 120.0f))
                        {
                            textComp->SetFontSize(fsz);
                        }

                        D2D1::ColorF c = textComp->GetColor();
                        float colorArr[4] = { c.r, c.g, c.b, c.a };
                        if (ImGui::ColorEdit4("Text Color", colorArr))
                        {
                            textComp->SetColor(D2D1::ColorF(colorArr[0], colorArr[1], colorArr[2], colorArr[3]));
                        }
                    }
                }

                CUIButton* btn = dynamic_cast<CUIButton*>(selectedObj);
                if (btn)
                {
                    if (ImGui::CollapsingHeader("CUIButton Settings", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        static const char* actionNames[] = {
                            "None",
                            "ChangeScene_Test",
                            "ChangeScene_Title",
                            "ChangeScene_Clear",
                            "ChangeScene_Failed",
                            "ExitGame"
                        };
                        static const ButtonAction actionEnums[] = {
                            ButtonAction::None,
                            ButtonAction::ChangeScene_Test,
                            ButtonAction::ChangeScene_Title,
                            ButtonAction::ChangeScene_Clear,
                            ButtonAction::ChangeScene_Failed,
                            ButtonAction::ExitGame
                        };

                        int currentActionIdx = 0;
                        ButtonAction curAction = btn->GetAction();
                        for (int a = 0; a < IM_ARRAYSIZE(actionEnums); ++a)
                        {
                            if (actionEnums[a] == curAction)
                            {
                                currentActionIdx = a;
                                break;
                            }
                        }

                        if (ImGui::Combo("OnClick Action", &currentActionIdx, actionNames, IM_ARRAYSIZE(actionNames)))
                        {
                            btn->SetAction(actionEnums[currentActionIdx]);
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
            // draw colliders
        }
    }
}

