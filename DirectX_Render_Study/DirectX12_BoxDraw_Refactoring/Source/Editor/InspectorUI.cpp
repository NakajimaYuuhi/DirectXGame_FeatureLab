#include "InspectorUI.h"
#include "EditorRaycast.h"
#include "UndoManager.h"
#include <d3d12.h>
#include "ContentDrawerUI.h"
#include "ModelManager.h"
#include "PrefabManager.h"
#include "PrefabSerializer.h"
#include "audio.h"
#include <filesystem>
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
#include "Box.h"
#include "UIObject.h"
#include "CUIButton.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "ButtonEventManager.h"
#include "ButtonAction.h"
#include "GravityComponent.h"
#include "HealthComponent.h"
#include "CharacterMovementComponent.h"
#include "PlayerControllerComponent.h"
#include "EnemyAIComponent.h"
#include "BulletComponent.h"
#include "CameraComponent.h"
#include "BillboardComponent.h"
#include "ParticleComponent.h"
#include "UVAnimationComponent.h"
#include "ParticleEmitterComponent.h"
#include "ButtonComponent.h"
#include "EnemyCounterComponent.h"
#include "CollisionLayers.h"
#include "RenderLayer.h"
#include <typeinfo>
#include <windows.h>
#include <vector>

static std::string GetCleanComponentName(CComponent* comp)
{
	if (!comp) return "Null";
	std::string name = comp->GetName();
	if (!name.empty()) return name;

	std::string rawName = typeid(*comp).name();
	const std::string classPrefix = "class ";
	const std::string structPrefix = "struct ";
	if (rawName.rfind(classPrefix, 0) == 0) {
		rawName = rawName.substr(classPrefix.length());
	} else if (rawName.rfind(structPrefix, 0) == 0) {
		rawName = rawName.substr(structPrefix.length());
	}
	return rawName;
}

static const char* GetUpdatePhaseName(UpdatePhase phase)
{
	switch (phase)
	{
	case UpdatePhase::Input:       return "Input";
	case UpdatePhase::AI:          return "AI";
	case UpdatePhase::Movement:    return "Movement";
	case UpdatePhase::Physics:     return "Physics";
	case UpdatePhase::Animation:   return "Animation";
	case UpdatePhase::PostPhysics: return "PostPhysics";
	default:                       return "Unknown";
	}
}

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

    ImGuiIO& io = ImGui::GetIO();
    Camera* camera = ObjectManager::GetInstance().GetCamera();

    // Keyboard Shortcuts (Undo: Ctrl+Z, Redo: Ctrl+Y / Ctrl+Shift+Z, Gizmo Mode: W/E/R)
    if (!io.WantCaptureKeyboard && !io.WantTextInput)
    {
        if (io.KeyCtrl)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Z))
            {
                if (io.KeyShift)
                    UndoManager::GetInstance().Redo();
                else
                    UndoManager::GetInstance().Undo();
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Y))
            {
                UndoManager::GetInstance().Redo();
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_S))
            {
                Scenes::ID activeScene = SceneManager::GetInstance().GetActiveSceneID();
                std::string path = "Assets/Scene/SceneTest.json";
                if (SceneSerializer::SaveScene(path, activeScene))
                {
                    SetStatusMessage("Scene saved (Ctrl+S): " + path, 3.0f);
                }
            }
        }
        else
        {
            if (ImGui::IsKeyPressed(ImGuiKey_W)) m_gizmoMode = GizmoMode::Translate;
            if (ImGui::IsKeyPressed(ImGuiKey_E)) m_gizmoMode = GizmoMode::Rotate;
            if (ImGui::IsKeyPressed(ImGuiKey_R)) m_gizmoMode = GizmoMode::Scale;
        }
    }

    // 3D Viewport Mouse Picking (Raycasting)
    if (!m_isPrefabEditMode && !io.WantCaptureMouse && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        float mouseX = io.MousePos.x;
        float mouseY = io.MousePos.y;
        float screenW = io.DisplaySize.x;
        float screenH = io.DisplaySize.y;

        int hitTag = -1, hitObj = -1;
        const auto& objectListForPick = ObjectManager::GetInstance().GetObjectList();
        CObject* hit = EditorRaycast::PickObject(mouseX, mouseY, screenW, screenH, camera, objectListForPick, hitTag, hitObj);
        if (hit && hitTag >= 0 && hitObj >= 0)
        {
            m_selectedTagIndex = hitTag;
            m_selectedObjectIndex = hitObj;
        }
    }

    if (m_isPrefabEditMode)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "[PREFAB STAGE] Editing: %s", m_editingPrefabPath.c_str());
        if (ImGui::Button("Save Prefab (Update JSON)"))
        {
            SaveCurrentPrefab();
        }
        ImGui::SameLine();
        if (ImGui::Button("<- Exit Prefab Stage"))
        {
            ClosePrefabEditMode();
            ImGui::End();
            return;
        }
        ImGui::Separator();
    }

    auto& objectList = ObjectManager::GetInstance().GetObjectList();
    Scenes::ID currentSceneID = SceneManager::GetInstance().GetActiveSceneID();

    // 1. Mode Controls
    ImGui::Text("Mode & Simulation");
    ImGui::SameLine();
    if (ImGui::Button("Content Drawer (Ctrl+Space)"))
    {
        CContentDrawerUI::GetInstance().ToggleVisible();
    }
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
            if (dynamic_cast<CUIButton*>(obj.get()) || obj->GetComponent<ButtonComponent>())
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
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::PLAYER, "Player", "Player");
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ Enemy"))
    {
        static int enemyCounter = 0;
        std::string name = "Enemy_" + std::to_string(enemyCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::ENEMY, "Enemy", name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ Box Field"))
    {
        static int boxCounter = 0;
        std::string name = "Box_" + std::to_string(boxCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::FIELD, "3DObject", name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ UI Image"))
    {
        static int uiCounter = 0;
        std::string name = "UIImage_" + std::to_string(uiCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::UI, "CUIObject", name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ UI Button"))
    {
        static int btnCounter = 0;
        std::string name = "UIButton_" + std::to_string(btnCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::UI, "CUIButton", name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ UI Text"))
    {
        static int txtCounter = 0;
        std::string name = "Text_" + std::to_string(txtCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::TEXT, "Text", name);
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
    CObject* player = ObjectManager::GetInstance().GetPlayer();
    if (player)
    {
        HealthComponent* health = player->GetComponent<HealthComponent>();
        if (health)
        {
            ImGui::Text("Player Status");
            ImGui::Text("HP: %d / %d", health->GetHP(), health->GetMaxHP());
            float hpFraction = (float)health->GetHP() / (float)health->GetMaxHP();
            ImGui::ProgressBar(hpFraction, ImVec2(-1.0f, 0.0f));
            ImGui::Separator();
        }
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

    float dt = TimeManager::GetInstance().GetDeltaTime();
    if (m_statusTimer > 0.0f)
    {
        m_statusTimer -= dt;
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "[STATUS] %s", m_statusMessage.c_str());
        ImGui::Separator();
    }

    CObject* selectedObj = nullptr;
    if (m_isPrefabEditMode && m_prefabEditTarget)
    {
        selectedObj = m_prefabEditTarget.get();
    }
    else if (m_selectedTagIndex >= 0 && m_selectedTagIndex < (int)objectList.size())
    {
        const auto& objVec = objectList[m_selectedTagIndex];
        if (m_selectedObjectIndex >= 0 && m_selectedObjectIndex < (int)objVec.size())
        {
            selectedObj = objVec[m_selectedObjectIndex].get();
        }
    }

    if (selectedObj && !selectedObj->GetIsDestroyed())
    {
        CObjectInfo* objInfo = selectedObj->GetComponent<CObjectInfo>();
        std::string name = objInfo ? objInfo->GetObjectName() : "Object";
        if (m_isPrefabEditMode)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "PREFAB TARGET: %s", name.c_str());
        }
        else
        {
            ImGui::Text("Selected: %s (Tag %d, Idx %d)", name.c_str(), m_selectedTagIndex, m_selectedObjectIndex);
            if (objInfo && objInfo->IsPrefab())
            {
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "[Prefab Instance: %s.json]", objInfo->GetPrefabName().c_str());
            }
            else
            {
                ImGui::TextDisabled("[Standalone Scene Object]");
            }
        }
                if (objInfo)
                {
                    char nameBuf[128];
                    strncpy_s(nameBuf, sizeof(nameBuf), name.c_str(), _TRUNCATE);
                    if (ImGui::InputText("Object Name", nameBuf, sizeof(nameBuf)))
                    {
                        objInfo->SetObjectName(std::string(nameBuf));
                    }

                    static const char* tagNames[] = {
                        "NONE", "BACKGROUND", "PLAYER", "PLAYER_BULLET", "ENEMY", "ENEMY_BULLET",
                        "FIELD", "TRIANGLE", "BILLBOARD", "EFFECT", "UI", "TEXT", "CAMERA", "FADE", "MANAGER"
                    };
                    static const ObjectTag tagValues[] = {
                        ObjectTag::NONE, ObjectTag::BACKGROUND, ObjectTag::PLAYER, ObjectTag::PLAYER_BULLET,
                        ObjectTag::ENEMY, ObjectTag::ENEMY_BULLET, ObjectTag::FIELD, ObjectTag::TRIANGLE,
                        ObjectTag::BILLBOARD, ObjectTag::EFFECT, ObjectTag::UI, ObjectTag::TEXT,
                        ObjectTag::CAMERA, ObjectTag::FADE, ObjectTag::MANAGER
                    };
                    int currentTagIdx = 0;
                    ObjectTag curTag = objInfo->GetObjectTag();
                    for (int t = 0; t < IM_ARRAYSIZE(tagValues); ++t)
                    {
                        if (tagValues[t] == curTag) { currentTagIdx = t; break; }
                    }
                    if (ImGui::Combo("Tag", &currentTagIdx, tagNames, IM_ARRAYSIZE(tagNames)))
                    {
                        objInfo->SetObjectTag(tagValues[currentTagIdx]);
                    }
                }
                
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
                ImGui::SameLine();
                if (ImGui::Button("Save as Prefab"))
                {
                    ImGui::OpenPopup("Save Object as Prefab");
                }

                if (ImGui::BeginPopupModal("Save Object as Prefab", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    static char prefabPathBuf[256] = "";
                    if (prefabPathBuf[0] == '\0')
                    {
                        CObjectInfo* info = selectedObj->GetComponent<CObjectInfo>();
                        std::string objName = info ? info->GetObjectName() : "PrefabObject";
                        std::string defaultPath = "Assets/Prefabs/" + objName + ".json";
                        strncpy_s(prefabPathBuf, sizeof(prefabPathBuf), defaultPath.c_str(), _TRUNCATE);
                    }

                    ImGui::Text("Save selected object as a JSON Prefab:");
                    ImGui::InputText("Save Path", prefabPathBuf, sizeof(prefabPathBuf));

                    if (ImGui::Button("Save", ImVec2(120, 0)))
                    {
                        std::string savePath = prefabPathBuf;
                        if (PrefabSerializer::SavePrefab(savePath, selectedObj))
                        {
                            std::string stemName = std::filesystem::path(savePath).stem().string();
                            PrefabManager::GetInstance().RegisterPrefabJSON(stemName, savePath);
                            PrefabManager::GetInstance().RegisterPrefabJSON(stemName + "JSON", savePath);
                            CContentDrawerUI::GetInstance().RefreshPrefabList();
                            m_statusMessage = "Prefab saved & registered: " + savePath;
                            m_statusTimer = 3.0f;
                        }
                        else
                        {
                            m_statusMessage = "Failed to save prefab: " + savePath;
                            m_statusTimer = 3.0f;
                        }
                        prefabPathBuf[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    {
                        prefabPathBuf[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                // -------------------------------------------------------------
                // Attached Components Overview Table
                // -------------------------------------------------------------
                const auto& compList = selectedObj->GetComponents();
                if (ImGui::CollapsingHeader("Attached Components", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text("Total Components: %d", (int)compList.size());
                    if (ImGui::BeginTable("AttachedCompTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
                    {
                        ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableSetupColumn("Component Name", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Phase", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 70.0f);
                        ImGui::TableHeadersRow();

                        for (size_t i = 0; i < compList.size(); ++i)
                        {
                            const auto& comp = compList[i];
                            if (!comp) continue;

                            ImGui::TableNextRow();
                            ImGui::PushID((int)i);

                            // Active column
                            ImGui::TableSetColumnIndex(0);
                            bool isValid = comp->GetIsValid();
                            bool isLocked = (dynamic_cast<CObjectInfo*>(comp.get()) != nullptr);
                            if (isLocked)
                            {
                                ImGui::BeginDisabled();
                                ImGui::Checkbox("##active", &isValid);
                                ImGui::EndDisabled();
                            }
                            else
                            {
                                if (ImGui::Checkbox("##active", &isValid))
                                {
                                    comp->SetIsValid(isValid);
                                }
                            }

                            // Component Name column
                            ImGui::TableSetColumnIndex(1);
                            std::string cName = GetCleanComponentName(comp.get());
                            ImGui::Text("%s", cName.c_str());

                            // Phase column
                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextDisabled("%s", GetUpdatePhaseName(comp->GetUpdatePhase()));

                            // Action column
                            ImGui::TableSetColumnIndex(3);
                            bool isNonRemovable = (dynamic_cast<CObjectInfo*>(comp.get()) != nullptr || dynamic_cast<CTransform*>(comp.get()) != nullptr);
                            if (!isNonRemovable)
                            {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                                if (ImGui::Button("Remove"))
                                {
                                    selectedObj->RemoveComponent(comp.get());
                                    ImGui::PopStyleColor();
                                    ImGui::PopID();
                                    break;
                                }
                                ImGui::PopStyleColor();
                            }
                            else
                            {
                                ImGui::TextDisabled("Locked");
                            }

                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }
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

                        static const char* rLayerNames[] = { "Opaque", "Transparent", "UI" };
                        int curLayer = (int)sprite->GetRenderLayer();
                        if (ImGui::Combo("Render Layer", &curLayer, rLayerNames, IM_ARRAYSIZE(rLayerNames)))
                        {
                            sprite->SetRenderLayer((RenderLayer)curLayer);
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

                        static const char* rLayerNames[] = { "Opaque", "Transparent", "UI" };
                        int curTextLayer = (int)textComp->GetRenderLayer();
                        if (ImGui::Combo("Render Layer", &curTextLayer, rLayerNames, IM_ARRAYSIZE(rLayerNames)))
                        {
                            textComp->SetRenderLayer((RenderLayer)curTextLayer);
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

                        // Navigation Target Selection
                        std::vector<std::string> buttonNames;
                        buttonNames.push_back("(None)");
                        std::vector<CUIButton*> buttonPtrs;
                        buttonPtrs.push_back(nullptr);

                        for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
                        {
                            for (const auto& obj : objectList[tagIdx])
                            {
                                if (!obj || obj->GetIsDestroyed()) continue;
                                if (dynamic_cast<CUIButton*>(obj.get()) || obj->GetComponent<ButtonComponent>())
                                {
                                    CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                                    std::string bName = info ? info->GetObjectName() : "Button";
                                    buttonNames.push_back(bName);
                                    buttonPtrs.push_back(dynamic_cast<CUIButton*>(obj.get()));
                                }
                            }
                        }

                        std::vector<const char*> btnComboLabels;
                        for (const auto& bName : buttonNames) btnComboLabels.push_back(bName.c_str());

                        auto FindComboIndex = [&](const std::string& targetName, CUIButton* legacyPtr) -> int {
                            if (!targetName.empty())
                            {
                                for (size_t idx = 1; idx < buttonNames.size(); ++idx)
                                {
                                    if (buttonNames[idx] == targetName) return (int)idx;
                                }
                            }
                            if (legacyPtr)
                            {
                                for (size_t idx = 1; idx < buttonPtrs.size(); ++idx)
                                {
                                    if (buttonPtrs[idx] == legacyPtr) return (int)idx;
                                }
                            }
                            return 0;
                        };

                        int upIdx = FindComboIndex(btn->GetUpName(), btn->GetSelectOnUp());
                        int downIdx = FindComboIndex(btn->GetDownName(), btn->GetSelectOnDown());
                        int leftIdx = FindComboIndex(btn->GetLeftName(), btn->GetSelectOnLeft());
                        int rightIdx = FindComboIndex(btn->GetRightName(), btn->GetSelectOnRight());

                        bool navChanged = false;
                        if (ImGui::Combo("Select On Up", &upIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;
                        if (ImGui::Combo("Select On Down", &downIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;
                        if (ImGui::Combo("Select On Left", &leftIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;
                        if (ImGui::Combo("Select On Right", &rightIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;

                        if (navChanged)
                        {
                            auto GetNameOrEmpty = [&](int idx) -> std::string {
                                return (idx > 0 && idx < (int)buttonNames.size()) ? buttonNames[idx] : "";
                            };
                            btn->SetNavigationNames(GetNameOrEmpty(upIdx), GetNameOrEmpty(downIdx), GetNameOrEmpty(leftIdx), GetNameOrEmpty(rightIdx));
                            btn->SetNavigation(buttonPtrs[upIdx], buttonPtrs[downIdx], buttonPtrs[leftIdx], buttonPtrs[rightIdx]);
                        }
                    }
                }

                CModel* model = selectedObj->GetComponent<CModel>();
                if (model)
                {
                    if (ImGui::CollapsingHeader("Model & Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        static char modelPathInput[256] = "";
                        std::string curPath = model->GetModelPath();
                        if (!curPath.empty())
                        {
                            strncpy_s(modelPathInput, sizeof(modelPathInput), curPath.c_str(), _TRUNCATE);
                        }
                        else if (modelPathInput[0] == '\0')
                        {
                            strncpy_s(modelPathInput, sizeof(modelPathInput), "Assets/Model/Wizard.glb", _TRUNCATE);
                        }

                        ImGui::InputText("Model Path", modelPathInput, sizeof(modelPathInput));
                        if (ImGui::Button("Load / Copy Model"))
                        {
                            auto sharedModel = ModelManager::GetInstance().GetModel(modelPathInput);
                            if (sharedModel)
                            {
                                model->CopyFrom(sharedModel);
                                model->SetModelPath(modelPathInput);
                                model->PlayAnimation("Idle");
                            }
                        }

                        static const char* rLayerNames[] = { "Opaque", "Transparent", "UI" };
                        int curModelLayer = (int)model->GetRenderLayer();
                        if (ImGui::Combo("Render Layer", &curModelLayer, rLayerNames, IM_ARRAYSIZE(rLayerNames)))
                        {
                            model->SetRenderLayer((RenderLayer)curModelLayer);
                        }

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

                        bool isTrigger = boxCollider->GetIsTrigger();
                        if (ImGui::Checkbox("Is Trigger (Pass through)", &isTrigger))
                        {
                            boxCollider->SetIsTrigger(isTrigger);
                        }

                        static const char* layerNames[] = {
                            "Default", "Player", "Enemy", "PlayerBullet", "EnemyBullet", "Terrain", "Obstacle", "Trigger"
                        };
                        static const uint32_t layerValues[] = {
                            CollisionLayer::Default, CollisionLayer::Player, CollisionLayer::Enemy,
                            CollisionLayer::PlayerBullet, CollisionLayer::EnemyBullet,
                            CollisionLayer::Terrain, CollisionLayer::Obstacle, CollisionLayer::Trigger
                        };
                        int curLayerIdx = 0;
                        uint32_t curLayer = boxCollider->GetLayer();
                        for (int l = 0; l < IM_ARRAYSIZE(layerValues); ++l)
                        {
                            if (layerValues[l] == curLayer) { curLayerIdx = l; break; }
                        }
                        if (ImGui::Combo("Collision Layer", &curLayerIdx, layerNames, IM_ARRAYSIZE(layerNames)))
                        {
                            boxCollider->SetLayer(layerValues[curLayerIdx]);
                        }
                    }
                }

                GravityComponent* gravityComp = selectedObj->GetComponent<GravityComponent>();
                if (gravityComp)
                {
                    if (ImGui::CollapsingHeader("GravityComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = gravityComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##GravityComp", &enabled)) gravityComp->SetEnabled(enabled);
                        float grav = gravityComp->GetGravity();
                        if (ImGui::DragFloat("Gravity Accel", &grav, 0.5f, -100.0f, 0.0f, "%.1f"))
                        {
                            gravityComp->SetGravity(grav);
                        }

                        float jPower = gravityComp->GetJumpPower();
                        if (ImGui::DragFloat("Jump Power", &jPower, 0.5f, 0.0f, 50.0f, "%.1f"))
                        {
                            gravityComp->SetJumpPower(jPower);
                        }

                        float vSpeed = gravityComp->GetVerticalVelocity();
                        ImGui::Text("Vertical Velocity: %.2f", vSpeed);

                        bool grounded = gravityComp->IsGrounded();
                        if (grounded)
                        {
                            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Grounded: YES");
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Grounded: NO (In Air)");
                        }

                        if (ImGui::Button("Test Jump"))
                        {
                            gravityComp->Jump();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Snap To Ground"))
                        {
                            gravityComp->SnapToGround();
                        }
                    }
                }

                HealthComponent* healthComp = selectedObj->GetComponent<HealthComponent>();
                if (healthComp)
                {
                    if (ImGui::CollapsingHeader("HealthComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = healthComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##HealthComp", &enabled)) healthComp->SetEnabled(enabled);
                        int curHp = healthComp->GetHP();
                        int maxHp = healthComp->GetMaxHP();

                        float fraction = maxHp > 0 ? (float)curHp / (float)maxHp : 0.0f;
                        char overlay[32];
                        sprintf_s(overlay, "%d / %d", curHp, maxHp);
                        ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);

                        if (ImGui::SliderInt("HP", &curHp, 0, maxHp))
                        {
                            healthComp->SetHP(curHp);
                        }

                        if (ImGui::DragInt("Max HP", &maxHp, 1, 1, 999))
                        {
                            healthComp->SetMaxHP(maxHp);
                        }

                        bool isDead = healthComp->IsDead();
                        bool isInvincible = healthComp->IsInvincible();

                        if (isDead)
                        {
                            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Status: DEAD");
                        }
                        else if (isInvincible)
                        {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Status: INVINCIBLE (Flashing)");
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: ALIVE");
                        }

                        if (ImGui::Button("Take 1 Damage"))
                        {
                            healthComp->TakeDamage(1);
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Heal 1 HP"))
                        {
                            healthComp->SetHP(min(maxHp, curHp + 1));
                        }
                    }
                }

                CharacterMovementComponent* movementComp = selectedObj->GetComponent<CharacterMovementComponent>();
                if (movementComp)
                {
                    if (ImGui::CollapsingHeader("CharacterMovementComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = movementComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##MovementComp", &enabled)) movementComp->SetEnabled(enabled);
                        float spd = movementComp->GetSpeed();
                        if (ImGui::DragFloat("Move Speed", &spd, 0.005f, 0.0f, 2.0f, "%.3f"))
                        {
                            movementComp->SetSpeed(spd);
                        }

                        float climbNy = movementComp->GetMinClimbNormalY();
                        if (ImGui::SliderFloat("Slope Limit (Normal Y)", &climbNy, 0.0f, 1.0f, "%.2f"))
                        {
                            movementComp->SetMinClimbNormalY(climbNy);
                        }

                        bool autoRot = movementComp->GetAutoRotate();
                        if (ImGui::Checkbox("Auto Rotate to Movement", &autoRot))
                        {
                            movementComp->SetAutoRotate(autoRot);
                        }

                        DirectX::XMFLOAT3 lastMove = movementComp->GetLastMovement();
                        ImGui::Text("Last Move: (%.3f, %.3f, %.3f)", lastMove.x, lastMove.y, lastMove.z);
                    }
                }

                PlayerControllerComponent* playerCtrl = selectedObj->GetComponent<PlayerControllerComponent>();
                if (playerCtrl)
                {
                    if (ImGui::CollapsingHeader("PlayerControllerComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = playerCtrl->IsEnabled();
                        if (ImGui::Checkbox("Enabled##PlayerCtrlComp", &enabled)) playerCtrl->SetEnabled(enabled);
                        static const char* stateNames[] = { "Idle", "Move", "Attack", "Hurt", "Dead" };
                        int curState = (int)playerCtrl->GetCurrentState();
                        const char* curStateName = (curState >= 0 && curState < 5) ? stateNames[curState] : "Unknown";
                        ImGui::Text("State: %s", curStateName);
                        ImGui::Text("Input Active: %s", playerCtrl->HasMoveInput() ? "YES" : "NO");
                    }
                }

                EnemyAIComponent* enemyAI = selectedObj->GetComponent<EnemyAIComponent>();
                if (enemyAI)
                {
                    if (ImGui::CollapsingHeader("EnemyAIComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = enemyAI->IsEnabled();
                        if (ImGui::Checkbox("Enabled##EnemyAIComp", &enabled)) enemyAI->SetEnabled(enabled);
                        static const char* aiStateNames[] = { "Idle", "Chase", "Attack", "Hurt", "Dead" };
                        int curState = (int)enemyAI->GetCurrentState();
                        const char* curStateName = (curState >= 0 && curState < 5) ? aiStateNames[curState] : "Unknown";
                        ImGui::Text("AI State: %s", curStateName);

                        char dPrefabBuf[128];
                        strncpy_s(dPrefabBuf, sizeof(dPrefabBuf), enemyAI->GetDeathEffectPrefab().c_str(), _TRUNCATE);
                        if (ImGui::InputText("Death Effect Prefab", dPrefabBuf, sizeof(dPrefabBuf)))
                        {
                            enemyAI->SetDeathEffectPrefab(std::string(dPrefabBuf));
                        }

                        char dmgPrefabBuf[128];
                        strncpy_s(dmgPrefabBuf, sizeof(dmgPrefabBuf), enemyAI->GetDamagedEffectPrefab().c_str(), _TRUNCATE);
                        if (ImGui::InputText("Damaged Effect Prefab", dmgPrefabBuf, sizeof(dmgPrefabBuf)))
                        {
                            enemyAI->SetDamagedEffectPrefab(std::string(dmgPrefabBuf));
                        }
                    }
                }

                BulletComponent* bulletComp = selectedObj->GetComponent<BulletComponent>();
                if (bulletComp)
                {
                    if (ImGui::CollapsingHeader("BulletComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = bulletComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##BulletComp", &enabled)) bulletComp->SetEnabled(enabled);
                        DirectX::XMFLOAT3 dir = bulletComp->GetDirection();
                        if (ImGui::DragFloat3("Direction", &dir.x, 0.01f))
                        {
                            bulletComp->SetDirection(dir);
                        }

                        float speed = bulletComp->GetSpeed();
                        if (ImGui::DragFloat("Speed", &speed, 0.005f, 0.0f, 10.0f))
                        {
                            bulletComp->SetSpeed(speed);
                        }

                        float lifeTime = bulletComp->GetLifeTime();
                        if (ImGui::DragFloat("LifeTime", &lifeTime, 0.1f, 0.0f, 60.0f))
                        {
                            bulletComp->SetLifeTime(lifeTime);
                        }

                        int damage = bulletComp->GetDamage();
                        if (ImGui::DragInt("Damage", &damage, 1, 0, 100))
                        {
                            bulletComp->SetDamage(damage);
                        }

                        char hitPrefabBuf[128];
                        strncpy_s(hitPrefabBuf, sizeof(hitPrefabBuf), bulletComp->GetHitEffectPrefab().c_str(), _TRUNCATE);
                        if (ImGui::InputText("Hit Effect Prefab", hitPrefabBuf, sizeof(hitPrefabBuf)))
                        {
                            bulletComp->SetHitEffectPrefab(std::string(hitPrefabBuf));
                        }
                    }
                }

                CameraComponent* cameraComp = selectedObj->GetComponent<CameraComponent>();
                if (cameraComp)
                {
                    if (ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = cameraComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##CameraComp", &enabled)) cameraComp->SetEnabled(enabled);
                        CObject* target = cameraComp->GetTarget();
                        CObjectInfo* targetInfo = target ? target->GetComponent<CObjectInfo>() : nullptr;
                        std::string targetName = targetInfo ? targetInfo->GetObjectName() : (target ? "Target" : "(Auto Player)");
                        ImGui::Text("Follow Target: %s", targetName.c_str());

                        float dist = cameraComp->GetDistance();
                        if (ImGui::DragFloat("Distance", &dist, 0.1f, 0.5f, 50.0f))
                        {
                            cameraComp->SetDistance(dist);
                        }

                        float height = cameraComp->GetHeight();
                        if (ImGui::DragFloat("Height", &height, 0.1f, -10.0f, 30.0f))
                        {
                            cameraComp->SetHeight(height);
                        }

                        float angleY = cameraComp->GetAngleY();
                        if (ImGui::DragFloat("Angle Y (Yaw)", &angleY, 0.02f, -6.28f, 6.28f))
                        {
                            cameraComp->SetAngleY(angleY);
                        }

                        float angleX = cameraComp->GetAngleX();
                        if (ImGui::DragFloat("Angle X (Pitch)", &angleX, 0.02f, -1.5f, 1.5f))
                        {
                            cameraComp->SetAngleX(angleX);
                        }

                        float fovDeg = cameraComp->GetFov() * (180.0f / 3.14159265f);
                        if (ImGui::DragFloat("FOV (Degrees)", &fovDeg, 0.5f, 10.0f, 120.0f))
                        {
                            cameraComp->SetFov(fovDeg * (3.14159265f / 180.0f));
                        }

                        float followSpeed = cameraComp->GetFollowSpeed();
                        if (ImGui::DragFloat("Follow Speed", &followSpeed, 0.5f, 0.0f, 50.0f))
                        {
                            cameraComp->SetFollowSpeed(followSpeed);
                        }

                        DirectX::XMFLOAT3 tOffset = cameraComp->GetTargetOffset();
                        if (ImGui::DragFloat3("Target Offset", &tOffset.x, 0.1f))
                        {
                            cameraComp->SetTargetOffset(tOffset);
                        }

                        ImGui::Spacing();
                        if (ImGui::Button("Shake Camera Test"))
                        {
                            cameraComp->Shake(0.3f, 0.5f);
                        }
                    }
                }

                BillboardComponent* bbComp = selectedObj->GetComponent<BillboardComponent>();
                if (bbComp)
                {
                    if (ImGui::CollapsingHeader("BillboardComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = bbComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##BillboardComp", &enabled)) bbComp->SetEnabled(enabled);
                        bool lockY = bbComp->GetLockYAxis();
                        if (ImGui::Checkbox("Lock Y Axis", &lockY)) bbComp->SetLockYAxis(lockY);
                    }
                }

                ParticleComponent* ptComp = selectedObj->GetComponent<ParticleComponent>();
                if (ptComp)
                {
                    if (ImGui::CollapsingHeader("ParticleComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = ptComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##ParticleComp", &enabled)) ptComp->SetEnabled(enabled);
                        float spd = ptComp->GetSpeed();
                        if (ImGui::DragFloat("Speed", &spd, 0.005f, 0.0f, 10.0f)) ptComp->SetSpeed(spd);
                        float life = ptComp->GetLifeTime();
                        if (ImGui::DragFloat("Life Time", &life, 0.1f, 0.0f, 60.0f)) ptComp->SetLifeTime(life);
                        DirectX::XMFLOAT3 dir = ptComp->GetDirection();
                        if (ImGui::DragFloat3("Direction", &dir.x, 0.01f)) ptComp->SetDirection(dir);
                        ImGui::Text("Random Direction: %s", ptComp->IsRandomDirection() ? "YES" : "NO");
                    }
                }

                UVAnimationComponent* uvComp = selectedObj->GetComponent<UVAnimationComponent>();
                if (uvComp)
                {
                    if (ImGui::CollapsingHeader("UVAnimationComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = uvComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##UVAnimComp", &enabled)) uvComp->SetEnabled(enabled);
                        int grid[2] = { uvComp->GetRows(), uvComp->GetCols() };
                        if (ImGui::DragInt2("Grid (Rows, Cols)", grid, 1, 1, 32)) uvComp->SetGrid(grid[0], grid[1]);
                        int totalFrames = uvComp->GetTotalFrames();
                        if (ImGui::DragInt("Total Frames", &totalFrames, 1, 1, 256)) uvComp->SetTotalFrames(totalFrames);
                        float frameDur = uvComp->GetFrameDuration();
                        if (ImGui::DragFloat("Frame Duration", &frameDur, 0.005f, 0.001f, 1.0f)) uvComp->SetFrameDuration(frameDur);
                        bool loop = uvComp->IsLoop();
                        if (ImGui::Checkbox("Loop Animation", &loop)) uvComp->SetLoop(loop);
                        bool destroyComplete = uvComp->IsDestroyOnComplete();
                        if (ImGui::Checkbox("Destroy On Complete", &destroyComplete)) uvComp->SetDestroyOnComplete(destroyComplete);
                        ImGui::Text("Current Frame: %d", uvComp->GetCurrentFrame());
                    }
                }

                ParticleEmitterComponent* peComp = selectedObj->GetComponent<ParticleEmitterComponent>();
                if (peComp)
                {
                    if (ImGui::CollapsingHeader("ParticleEmitterComponent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool enabled = peComp->IsEnabled();
                        if (ImGui::Checkbox("Enabled##ParticleEmitterComp", &enabled)) peComp->SetEnabled(enabled);

                        char prefabBuf[128];
                        strncpy_s(prefabBuf, sizeof(prefabBuf), peComp->GetParticlePrefab().c_str(), _TRUNCATE);
                        if (ImGui::InputText("Particle Prefab", prefabBuf, sizeof(prefabBuf)))
                        {
                            peComp->SetParticlePrefab(std::string(prefabBuf));
                        }

                        int count = peComp->GetBurstCount();
                        if (ImGui::DragInt("Burst Count", &count, 1, 1, 100)) peComp->SetBurstCount(count);

                        float interval = peComp->GetSpawnInterval();
                        if (ImGui::DragFloat("Spawn Interval", &interval, 0.05f, 0.0f, 60.0f)) peComp->SetSpawnInterval(interval);

                        bool burstStart = peComp->GetBurstOnStart();
                        if (ImGui::Checkbox("Burst On Start", &burstStart)) peComp->SetBurstOnStart(burstStart);

                        DirectX::XMFLOAT3 pScale = peComp->GetParticleScale();
                        if (ImGui::DragFloat3("Particle Scale", &pScale.x, 0.01f, 0.01f, 10.0f)) peComp->SetParticleScale(pScale);

                        if (ImGui::Button("Emit Test"))
                        {
                            peComp->Emit();
                        }
                    }
                }

                ButtonComponent* btnComp = selectedObj->GetComponent<ButtonComponent>();
                if (btnComp)
                {
                    if (ImGui::CollapsingHeader("ButtonComponent Settings", ImGuiTreeNodeFlags_DefaultOpen))
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
                        ButtonAction curAction = btnComp->GetAction();
                        for (int a = 0; a < IM_ARRAYSIZE(actionEnums); ++a)
                        {
                            if (actionEnums[a] == curAction)
                            {
                                currentActionIdx = a;
                                break;
                            }
                        }

                        if (ImGui::Combo("OnClick Action##BtnComp", &currentActionIdx, actionNames, IM_ARRAYSIZE(actionNames)))
                        {
                            btnComp->SetAction(actionEnums[currentActionIdx]);
                        }

                        // Navigation Target Selection
                        std::vector<std::string> targetNames;
                        targetNames.push_back("(None)");

                        for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
                        {
                            for (const auto& obj : objectList[tagIdx])
                            {
                                if (!obj || obj->GetIsDestroyed()) continue;
                                if (dynamic_cast<CUIButton*>(obj.get()) || obj->GetComponent<ButtonComponent>())
                                {
                                    CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                                    std::string bName = info ? info->GetObjectName() : "Button";
                                    targetNames.push_back(bName);
                                }
                            }
                        }

                        std::vector<const char*> btnComboLabels;
                        for (const auto& bName : targetNames) btnComboLabels.push_back(bName.c_str());

                        auto FindComboIndex = [&](const std::string& targetName) -> int {
                            if (targetName.empty()) return 0;
                            for (size_t idx = 1; idx < targetNames.size(); ++idx)
                            {
                                if (targetNames[idx] == targetName) return (int)idx;
                            }
                            return 0;
                        };

                        int upIdx = FindComboIndex(btnComp->GetUpName());
                        int downIdx = FindComboIndex(btnComp->GetDownName());
                        int leftIdx = FindComboIndex(btnComp->GetLeftName());
                        int rightIdx = FindComboIndex(btnComp->GetRightName());

                        bool navChanged = false;
                        if (ImGui::Combo("Select On Up##BtnComp", &upIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;
                        if (ImGui::Combo("Select On Down##BtnComp", &downIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;
                        if (ImGui::Combo("Select On Left##BtnComp", &leftIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;
                        if (ImGui::Combo("Select On Right##BtnComp", &rightIdx, btnComboLabels.data(), (int)btnComboLabels.size())) navChanged = true;

                        if (navChanged)
                        {
                            auto GetNameOrEmpty = [&](int idx) -> std::string {
                                return (idx > 0 && idx < (int)targetNames.size()) ? targetNames[idx] : "";
                            };
                            btnComp->SetNavigationNames(GetNameOrEmpty(upIdx), GetNameOrEmpty(downIdx), GetNameOrEmpty(leftIdx), GetNameOrEmpty(rightIdx));
                        }
                    }
                }

                // EnemyCounterComponent
                EnemyCounterComponent* ecComp = selectedObj->GetComponent<EnemyCounterComponent>();
                if (ecComp)
                {
                    if (ImGui::CollapsingHeader("EnemyCounterComponent Settings", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text("Active Enemies: %d", ecComp->GetCount());
                        ImGui::Text("Defeat Count: %d", ecComp->GetDefeatCount());

                        char targetBuf[128] = "";
                        strncpy_s(targetBuf, sizeof(targetBuf), ecComp->GetTargetTextName().c_str(), _TRUNCATE);
                        if (ImGui::InputText("Target Text Object", targetBuf, sizeof(targetBuf)))
                        {
                            ecComp->SetTargetTextName(targetBuf);
                        }
                    }
                }

                // -------------------------------------------------------------
                // Other Components (Components without custom inspector panels)
                // -------------------------------------------------------------
                std::vector<std::string> otherCompNames;
                for (const auto& comp : compList)
                {
                    if (!comp) continue;
                    CComponent* cPtr = comp.get();
                    if (cPtr != transform && cPtr != sprite && cPtr != textComp && cPtr != model &&
                        cPtr != boxCollider && cPtr != gravityComp && cPtr != healthComp &&
                        cPtr != movementComp && cPtr != playerCtrl && cPtr != enemyAI && cPtr != bulletComp && cPtr != cameraComp &&
                        cPtr != bbComp && cPtr != ptComp && cPtr != uvComp && cPtr != peComp && cPtr != btnComp && cPtr != ecComp && cPtr != objInfo)
                    {
                        otherCompNames.push_back(GetCleanComponentName(cPtr));
                    }
                }
                if (!otherCompNames.empty())
                {
                    if (ImGui::CollapsingHeader("Other Components", ImGuiTreeNodeFlags_None))
                    {
                        for (const auto& oName : otherCompNames)
                        {
                            ImGui::BulletText("%s (No configurable parameters)", oName.c_str());
                        }
                    }
                }

                // -------------------------------------------------------------
                // Add Component Dropdown Button & Searchable Popup
                // -------------------------------------------------------------
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                static char compSearchFilter[128] = "";
                if (ImGui::Button("+ Add Component", ImVec2(-1.0f, 28.0f)))
                {
                    compSearchFilter[0] = '\0';
                    ImGui::OpenPopup("AddComponentPopup");
                }

                if (ImGui::BeginPopup("AddComponentPopup"))
                {
                    ImGui::TextDisabled("Search Component:");
                    ImGui::SetNextItemWidth(260.0f);
                    if (ImGui::IsWindowAppearing())
                    {
                        ImGui::SetKeyboardFocusHere();
                    }
                    ImGui::InputTextWithHint("##CompSearch", "Type to search...", compSearchFilter, sizeof(compSearchFilter));
                    ImGui::Separator();

                    std::string filterStr = compSearchFilter;
                    std::transform(filterStr.begin(), filterStr.end(), filterStr.begin(), ::tolower);

                    auto MatchesFilter = [&filterStr](const std::string& name) -> bool {
                        if (filterStr.empty()) return true;
                        std::string lowerName = name;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                        return lowerName.find(filterStr) != std::string::npos;
                    };

                    // --- Visuals & Rendering ---
                    if (MatchesFilter("Model Component (CModel)") && !selectedObj->GetComponent<CModel>())
                    {
                        if (ImGui::Selectable("Model Component (CModel)"))
                        {
                            CModel* newModel = selectedObj->AddComponent<CModel>();
                            if (newModel)
                            {
                                auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/Wizard.glb");
                                if (sharedModel)
                                {
                                    newModel->CopyFrom(sharedModel);
                                    newModel->SetModelPath("Assets/Model/Wizard.glb");
                                    newModel->PlayAnimation("Idle");
                                }
                            }
                        }
                    }
                    if (MatchesFilter("Sprite Renderer Component (CSpriteRenderer)") && !selectedObj->GetComponent<CSpriteRenderer>())
                    {
                        if (ImGui::Selectable("Sprite Renderer Component (CSpriteRenderer)"))
                        {
                            selectedObj->AddComponent<CSpriteRenderer>();
                        }
                    }
                    if (MatchesFilter("Text Renderer Component (CTextRenderer)") && !selectedObj->GetComponent<CTextRenderer>())
                    {
                        if (ImGui::Selectable("Text Renderer Component (CTextRenderer)"))
                        {
                            selectedObj->AddComponent<CTextRenderer>();
                        }
                    }
                    if (MatchesFilter("Billboard Component") && !selectedObj->GetComponent<BillboardComponent>())
                    {
                        if (ImGui::Selectable("Billboard Component"))
                        {
                            selectedObj->AddComponent<BillboardComponent>();
                        }
                    }

                    // --- Physics & Gameplay ---
                    if (MatchesFilter("Box Collider 3D (BoxCollider3D)") && !selectedObj->GetComponent<BoxCollider3D>())
                    {
                        if (ImGui::Selectable("Box Collider 3D (BoxCollider3D)"))
                        {
                            selectedObj->AddComponent<BoxCollider3D>();
                        }
                    }
                    if (MatchesFilter("Gravity Component") && !selectedObj->GetComponent<GravityComponent>())
                    {
                        if (ImGui::Selectable("Gravity Component"))
                        {
                            selectedObj->AddComponent<GravityComponent>();
                        }
                    }
                    if (MatchesFilter("Character Movement Component") && !selectedObj->GetComponent<CharacterMovementComponent>())
                    {
                        if (ImGui::Selectable("Character Movement Component"))
                        {
                            selectedObj->AddComponent<CharacterMovementComponent>();
                        }
                    }
                    if (MatchesFilter("Health Component") && !selectedObj->GetComponent<HealthComponent>())
                    {
                        if (ImGui::Selectable("Health Component"))
                        {
                            selectedObj->AddComponent<HealthComponent>();
                        }
                    }
                    if (MatchesFilter("Player Controller Component") && !selectedObj->GetComponent<PlayerControllerComponent>())
                    {
                        if (ImGui::Selectable("Player Controller Component"))
                        {
                            selectedObj->AddComponent<PlayerControllerComponent>();
                        }
                    }
                    if (MatchesFilter("Enemy AI Component") && !selectedObj->GetComponent<EnemyAIComponent>())
                    {
                        if (ImGui::Selectable("Enemy AI Component"))
                        {
                            selectedObj->AddComponent<EnemyAIComponent>();
                        }
                    }
                    if (MatchesFilter("Bullet Component") && !selectedObj->GetComponent<BulletComponent>())
                    {
                        if (ImGui::Selectable("Bullet Component"))
                        {
                            selectedObj->AddComponent<BulletComponent>();
                        }
                    }
                    if (MatchesFilter("Camera Component") && !selectedObj->GetComponent<CameraComponent>())
                    {
                        if (ImGui::Selectable("Camera Component"))
                        {
                            selectedObj->AddComponent<CameraComponent>();
                        }
                    }

                    // --- UI & Input ---
                    if (MatchesFilter("Button Component") && !selectedObj->GetComponent<ButtonComponent>())
                    {
                        if (ImGui::Selectable("Button Component"))
                        {
                            selectedObj->AddComponent<ButtonComponent>();
                        }
                    }

                    // --- Manager ---
                    if (MatchesFilter("EnemyCounter Component") && !selectedObj->GetComponent<EnemyCounterComponent>())
                    {
                        if (ImGui::Selectable("EnemyCounter Component"))
                        {
                            selectedObj->AddComponent<EnemyCounterComponent>();
                        }
                    }

                    // --- Audio & FX ---
                    if (MatchesFilter("Audio Component (Audio)") && !selectedObj->GetComponent<Audio>())
                    {
                        if (ImGui::Selectable("Audio Component (Audio)"))
                        {
                            selectedObj->AddComponent<Audio>();
                        }
                    }
                    if (MatchesFilter("Particle Component") && !selectedObj->GetComponent<ParticleComponent>())
                    {
                        if (ImGui::Selectable("Particle Component"))
                        {
                            selectedObj->AddComponent<ParticleComponent>();
                        }
                    }
                    if (MatchesFilter("UV Animation Component") && !selectedObj->GetComponent<UVAnimationComponent>())
                    {
                        if (ImGui::Selectable("UV Animation Component"))
                        {
                            selectedObj->AddComponent<UVAnimationComponent>();
                        }
                    }
                    if (MatchesFilter("Particle Emitter Component") && !selectedObj->GetComponent<ParticleEmitterComponent>())
                    {
                        if (ImGui::Selectable("Particle Emitter Component"))
                        {
                            selectedObj->AddComponent<ParticleEmitterComponent>();
                        }
                    }

                    ImGui::EndPopup();
                }
            }
    else
    {
        ImGui::TextDisabled("Select an object from Hierarchy.");
    }

    ImGui::End();

    // 3D Transform Gizmo Rendering & Direct Mouse Manipulation
    if (camera)
    {
        CObject* targetObj = nullptr;
        if (m_isPrefabEditMode && m_prefabEditTarget)
        {
            targetObj = m_prefabEditTarget.get();
        }
        else
        {
            const auto& currentObjList = ObjectManager::GetInstance().GetObjectList();
            if (m_selectedTagIndex >= 0 && m_selectedTagIndex < static_cast<int>(currentObjList.size()))
            {
                const auto& vec = currentObjList[m_selectedTagIndex];
                if (m_selectedObjectIndex >= 0 && m_selectedObjectIndex < static_cast<int>(vec.size()))
                {
                    if (vec[m_selectedObjectIndex] && !vec[m_selectedObjectIndex]->GetIsDestroyed())
                    {
                        targetObj = vec[m_selectedObjectIndex].get();
                    }
                }
            }
        }

        if (targetObj)
        {
            CTransform* transform = targetObj->GetComponent<CTransform>();
            if (transform)
            {
                DirectX::XMFLOAT3 pos = transform->GetPos();
                DirectX::XMMATRIX view = camera->GetView();
                DirectX::XMMATRIX proj = camera->GetProj();

                DirectX::XMVECTOR vOrigin = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
                DirectX::XMVECTOR vAxisX = DirectX::XMVectorSet(pos.x + 1.5f, pos.y, pos.z, 1.0f);
                DirectX::XMVECTOR vAxisY = DirectX::XMVectorSet(pos.x, pos.y + 1.5f, pos.z, 1.0f);
                DirectX::XMVECTOR vAxisZ = DirectX::XMVectorSet(pos.x, pos.y, pos.z + 1.5f, 1.0f);

                D3D12_VIEWPORT viewport{};
                viewport.Width = io.DisplaySize.x;
                viewport.Height = io.DisplaySize.y;
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
                viewport.TopLeftX = 0;
                viewport.TopLeftY = 0;

                DirectX::XMVECTOR pOrigin = DirectX::XMVector3Project(vOrigin, viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());
                DirectX::XMVECTOR pAxisX  = DirectX::XMVector3Project(vAxisX,  viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());
                DirectX::XMVECTOR pAxisY  = DirectX::XMVector3Project(vAxisY,  viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());
                DirectX::XMVECTOR pAxisZ  = DirectX::XMVector3Project(vAxisZ,  viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());

                DirectX::XMFLOAT3 fOrigin, fAxisX, fAxisY, fAxisZ;
                DirectX::XMStoreFloat3(&fOrigin, pOrigin);
                DirectX::XMStoreFloat3(&fAxisX, pAxisX);
                DirectX::XMStoreFloat3(&fAxisY, pAxisY);
                DirectX::XMStoreFloat3(&fAxisZ, pAxisZ);

                bool axisValid[3] = { fOrigin.z > 0.0f && fAxisX.z > 0.0f, fOrigin.z > 0.0f && fAxisY.z > 0.0f, fOrigin.z > 0.0f && fAxisZ.z > 0.0f };
                if (axisValid[0] || axisValid[1] || axisValid[2])
                {
                    ImDrawList* drawList = ImGui::GetForegroundDrawList();
                    ImVec2 originScreen = ImVec2(fOrigin.x, fOrigin.y);
                    ImVec2 axisEndScreen[3] = {
                        ImVec2(fAxisX.x, fAxisX.y),
                        ImVec2(fAxisY.x, fAxisY.y),
                        ImVec2(fAxisZ.x, fAxisZ.y)
                    };

                    ImVec2 mousePos = io.MousePos;
                    int hoverAxis = -1;
                    float minHoverDist = 16.0f; // Threshold in pixels

                    if (!m_isDraggingGizmo)
                    {
                        for (int a = 0; a < 3; ++a)
                        {
                            if (!axisValid[a]) continue;
                            ImVec2 p1 = originScreen;
                            ImVec2 p2 = axisEndScreen[a];

                            float l2 = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
                            if (l2 < 1e-4f) continue;

                            float t = ((mousePos.x - p1.x) * (p2.x - p1.x) + (mousePos.y - p1.y) * (p2.y - p1.y)) / l2;
                            t = (std::max)(0.0f, (std::min)(1.0f, t));
                            ImVec2 projPt = ImVec2(p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y));
                            float dist = sqrtf((mousePos.x - projPt.x) * (mousePos.x - projPt.x) + (mousePos.y - projPt.y) * (mousePos.y - projPt.y));

                            if (dist < minHoverDist)
                            {
                                minHoverDist = dist;
                                hoverAxis = a;
                            }
                        }
                    }

                    // Mouse Dragging Logic
                    if (m_isDraggingGizmo)
                    {
                        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                        {
                            ImVec2 mouseDelta = ImVec2(mousePos.x - m_dragStartMouseX, mousePos.y - m_dragStartMouseY);
                            int a = m_draggedAxis;
                            if (a >= 0 && a < 3 && axisValid[a])
                            {
                                ImVec2 axisDir2D = ImVec2(axisEndScreen[a].x - originScreen.x, axisEndScreen[a].y - originScreen.y);
                                float len2D = sqrtf(axisDir2D.x * axisDir2D.x + axisDir2D.y * axisDir2D.y);
                                if (len2D > 1e-4f)
                                {
                                    axisDir2D.x /= len2D;
                                    axisDir2D.y /= len2D;
                                    float projAmount = mouseDelta.x * axisDir2D.x + mouseDelta.y * axisDir2D.y;

                                    if (m_gizmoMode == GizmoMode::Translate)
                                    {
                                        DirectX::XMFLOAT3 newPos = m_dragStartVal;
                                        float factor = 0.02f;
                                        if (a == 0) newPos.x += projAmount * factor;
                                        if (a == 1) newPos.y += projAmount * factor; // Fixed Y axis direction
                                        if (a == 2) newPos.z += projAmount * factor;
                                        transform->SetPos(newPos);
                                    }
                                    else if (m_gizmoMode == GizmoMode::Rotate)
                                    {
                                        DirectX::XMFLOAT3 newRot = m_dragStartVal;
                                        float factor = 0.01f;
                                        if (a == 0) newRot.x += projAmount * factor;
                                        if (a == 1) newRot.y += projAmount * factor;
                                        if (a == 2) newRot.z += projAmount * factor;
                                        transform->SetRotation(newRot);
                                    }
                                    else if (m_gizmoMode == GizmoMode::Scale)
                                    {
                                        DirectX::XMFLOAT3 newScale = m_dragStartVal;
                                        float factor = 0.02f;
                                        if (a == 0) newScale.x = (std::max)(0.01f, m_dragStartVal.x + projAmount * factor);
                                        if (a == 1) newScale.y = (std::max)(0.01f, m_dragStartVal.y + projAmount * factor); // Fixed Y axis direction
                                        if (a == 2) newScale.z = (std::max)(0.01f, m_dragStartVal.z + projAmount * factor);
                                        transform->SetScale(newScale);
                                    }
                                }
                            }
                        }
                        else
                        {
                            m_isDraggingGizmo = false;
                            m_draggedAxis = -1;

                            DirectX::XMFLOAT3 curPos = transform->GetPos();
                            DirectX::XMFLOAT3 curRot = transform->GetRotation();
                            DirectX::XMFLOAT3 curScale = transform->GetScale();

                            if (curPos.x != m_dragStartPos.x || curPos.y != m_dragStartPos.y || curPos.z != m_dragStartPos.z ||
                                curRot.x != m_dragStartRot.x || curRot.y != m_dragStartRot.y || curRot.z != m_dragStartRot.z ||
                                curScale.x != m_dragStartScale.x || curScale.y != m_dragStartScale.y || curScale.z != m_dragStartScale.z)
                            {
                                UndoManager::GetInstance().RecordCommand(
                                    std::make_unique<TransformUndoCommand>(
                                        targetObj,
                                        m_dragStartPos, curPos,
                                        m_dragStartRot, curRot,
                                        m_dragStartScale, curScale
                                    )
                                );
                            }
                        }
                    }
                    else
                    {
                        if (hoverAxis >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                        {
                            m_isDraggingGizmo = true;
                            m_draggedAxis = hoverAxis;
                            m_dragStartMouseX = mousePos.x;
                            m_dragStartMouseY = mousePos.y;
                            m_dragStartPos = transform->GetPos();
                            m_dragStartRot = transform->GetRotation();
                            m_dragStartScale = transform->GetScale();
                            if (m_gizmoMode == GizmoMode::Translate) m_dragStartVal = m_dragStartPos;
                            else if (m_gizmoMode == GizmoMode::Rotate) m_dragStartVal = m_dragStartRot;
                            else if (m_gizmoMode == GizmoMode::Scale) m_dragStartVal = m_dragStartScale;
                        }
                    }

                    // Render Thicker Axes with Dark Contrast Outline
                    ImU32 colors[3] = {
                        (m_isDraggingGizmo && m_draggedAxis == 0) ? IM_COL32(255, 255, 0, 255) : (hoverAxis == 0 ? IM_COL32(255, 220, 0, 255) : IM_COL32(255, 60, 60, 255)),
                        (m_isDraggingGizmo && m_draggedAxis == 1) ? IM_COL32(255, 255, 0, 255) : (hoverAxis == 1 ? IM_COL32(255, 220, 0, 255) : IM_COL32(40, 255, 40, 255)),
                        (m_isDraggingGizmo && m_draggedAxis == 2) ? IM_COL32(255, 255, 0, 255) : (hoverAxis == 2 ? IM_COL32(255, 220, 0, 255) : IM_COL32(40, 160, 255, 255))
                    };

                    const char* labels[3] = { "X", "Y", "Z" };

                    for (int a = 0; a < 3; ++a)
                    {
                        if (axisValid[a])
                        {
                            bool isCurrent = (hoverAxis == a || (m_isDraggingGizmo && m_draggedAxis == a));
                            float thickness = isCurrent ? 10.0f : 6.0f;
                            
                            // Black outline for background contrast
                            drawList->AddLine(originScreen, axisEndScreen[a], IM_COL32(0, 0, 0, 220), thickness + 4.0f);
                            // Main colored axis line
                            drawList->AddLine(originScreen, axisEndScreen[a], colors[a], thickness);
                            
                            // Label with dark background text shadow
                            ImVec2 labelPos = ImVec2(axisEndScreen[a].x + 4.0f, axisEndScreen[a].y - 6.0f);
                            drawList->AddText(ImVec2(labelPos.x + 1.0f, labelPos.y + 1.0f), IM_COL32(0, 0, 0, 255), labels[a]);
                            drawList->AddText(labelPos, colors[a], labels[a]);
                        }
                    }

                    // Selection Indicator Ring & Center Dot with Dark Outline
                    drawList->AddCircleFilled(originScreen, 9.0f, IM_COL32(0, 0, 0, 200));
                    drawList->AddCircleFilled(originScreen, 6.0f, IM_COL32(255, 255, 255, 255));
                    drawList->AddCircle(originScreen, 12.0f, IM_COL32(255, 200, 0, 255), 0, 3.0f);
                }
            }
        }
    }

    if (m_showColliders)
    {
        Camera* camera = ObjectManager::GetInstance().GetCamera();
        if (camera)
        {
            const auto& objectList = ObjectManager::GetInstance().GetObjectList();
            for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
            {
                for (const auto& obj : objectList[tagIdx])
                {
                    if (!obj || obj->GetIsDestroyed()) continue;
                    BoxCollider3D* collider = obj->GetComponent<BoxCollider3D>();
                    if (collider)
                    {
                        collider->DrawDebug(camera);
                    }
                }
            }
        }
    }
}

namespace fs = std::filesystem;

void CInspectorUI::OpenPrefabEditMode(const std::string& jsonPath)
{
    m_editingPrefabPath = jsonPath;
    std::string prefabName = fs::path(jsonPath).stem().string();

    CObject* rawObj = PrefabManager::GetInstance().InstantiateFromJSON(jsonPath, "Editing_" + prefabName);
    if (rawObj)
    {
        rawObj->Awake();
        rawObj->Start();

        m_prefabEditTarget = std::unique_ptr<CObject>(rawObj);
        m_isPrefabEditMode = true;
    }
}

void CInspectorUI::ClosePrefabEditMode()
{
    m_prefabEditTarget.reset();
    m_editingPrefabPath = "";
    m_isPrefabEditMode = false;
}

bool CInspectorUI::SaveCurrentPrefab()
{
    if (!m_isPrefabEditMode || !m_prefabEditTarget || m_editingPrefabPath.empty()) return false;
    bool success = PrefabSerializer::SavePrefab(m_editingPrefabPath, m_prefabEditTarget.get());
    if (success)
    {
        std::string stemName = std::filesystem::path(m_editingPrefabPath).stem().string();
        PrefabManager::GetInstance().RegisterPrefabJSON(stemName, m_editingPrefabPath);
        PrefabManager::GetInstance().RegisterPrefabJSON(stemName + "JSON", m_editingPrefabPath);
        CContentDrawerUI::GetInstance().RefreshPrefabList();
        SetStatusMessage("Prefab updated: " + m_editingPrefabPath, 3.0f);
    }
    return success;
}

