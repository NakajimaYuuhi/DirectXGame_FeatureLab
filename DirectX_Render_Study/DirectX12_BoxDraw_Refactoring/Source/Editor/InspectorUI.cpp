#include "InspectorUI.h"
#include "EditorCamera.h"
#include "ViewportUI.h"
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
#include "MaterialManager.h"
#include "ObjectInfo.h"
#include "BoxCollider3D.h"
#include "CapsuleCollider3D.h"
#include "TimeManager.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"
#include "SceneEnums.h"
#include "Source/UI/RectTransform.h"
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
#include "FieldComponent.h"
#include "LightComponent.h"
#include "CollisionLayers.h"
#include "RenderLayer.h"
#include "LightManager.h"
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

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;
    float toolbarH = 48.0f;
    float inspectorW = 460.0f;

    ImGui::SetNextWindowPos(ImVec2(screenW - inspectorW, toolbarH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(inspectorW, screenH - toolbarH), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::Begin("Level Editor & Inspector", nullptr, flags);
    CameraComponent* camera = ObjectManager::GetInstance().GetCamera();

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

        // If Viewport image is active, calculate raycast relative to the rendered image quad
        if (CViewportUI::GetInstance().IsVisible())
        {
            ImVec2 imgPos = CViewportUI::GetInstance().GetImagePos();
            ImVec2 imgSize = CViewportUI::GetInstance().GetImageSize();
            if (imgSize.x > 1.0f && imgSize.y > 1.0f)
            {
                // Only pick if mouse is inside the 3D viewport rendered area
                if (mouseX >= imgPos.x && mouseX <= imgPos.x + imgSize.x &&
                    mouseY >= imgPos.y && mouseY <= imgPos.y + imgSize.y)
                {
                    mouseX -= imgPos.x;
                    mouseY -= imgPos.y;
                    screenW = imgSize.x;
                    screenH = imgSize.y;
                }
                else
                {
                    screenW = -1.0f; // Skip pick outside viewport image
                }
            }
        }

        if (screenW > 0.0f && screenH > 0.0f)
        {
            int hitTag = -1, hitObj = -1;
            int hitMesh = -1, hitMatSlot = -1;
            const auto& objectListForPick = ObjectManager::GetInstance().GetObjectList();
            DirectX::XMMATRIX view = (m_isEditMode || m_isPrefabEditMode) ? EditorCamera::GetInstance().GetViewMatrix() : (camera ? camera->GetViewMatrix() : DirectX::XMMatrixIdentity());
            DirectX::XMMATRIX proj = (m_isEditMode || m_isPrefabEditMode) ? EditorCamera::GetInstance().GetProjectionMatrix() : (camera ? camera->GetProjectionMatrix() : DirectX::XMMatrixIdentity());
            CObject* hit = EditorRaycast::PickObject(mouseX, mouseY, screenW, screenH, view, proj, objectListForPick, hitTag, hitObj, &hitMesh, &hitMatSlot);
            if (hit && hitTag >= 0 && hitObj >= 0)
            {
                m_selectedTagIndex = hitTag;
                m_selectedObjectIndex = hitObj;

                CModel* model = hit->GetComponent<CModel>();
                if (model)
                {
                    if (hitMatSlot >= 0)
                    {
                        model->SetSelectedMaterialIndex(hitMatSlot);
                    }
                }
            }
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
            if (obj->GetComponent<ButtonComponent>())
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
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::UI, "UIImage", name);
        if (newObj) newObj->Awake();
    }
    ImGui::SameLine();
    if (ImGui::Button("+ UI Button"))
    {
        static int btnCounter = 0;
        std::string name = "UIButton_" + std::to_string(btnCounter++);
        CObject* newObj = ObjectManager::GetInstance().Instantiate(currentSceneID, ObjectTag::UI, "UIButton", name);
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

    // 4.5 Lighting & Environment (Directional Light / Ambient)
    if (ImGui::CollapsingHeader("Lighting & Environment", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& lightMgr = LightManager::GetInstance();
        auto& dirLight = lightMgr.GetDirectionalLight();

        float pitch = lightMgr.GetPitch();
        float yaw = lightMgr.GetYaw();
        bool angleChanged = false;

        ImGui::Text("Directional Light (Sun)");
        if (ImGui::SliderFloat("Sun Pitch", &pitch, -90.0f, 90.0f, "%.1f deg")) angleChanged = true;
        if (ImGui::SliderFloat("Sun Yaw", &yaw, -180.0f, 180.0f, "%.1f deg")) angleChanged = true;
        if (angleChanged)
        {
            lightMgr.SetAngles(pitch, yaw);
        }

        ImGui::TextDisabled("Light Dir: (%.2f, %.2f, %.2f)", dirLight.direction.x, dirLight.direction.y, dirLight.direction.z);

        float lightColor[3] = { dirLight.color.x, dirLight.color.y, dirLight.color.z };
        if (ImGui::ColorEdit3("Light Color", lightColor))
        {
            dirLight.color.x = lightColor[0];
            dirLight.color.y = lightColor[1];
            dirLight.color.z = lightColor[2];
        }

        float intensity = dirLight.direction.w;
        if (ImGui::SliderFloat("Light Intensity", &intensity, 0.0f, 4.0f, "%.2f"))
        {
            dirLight.direction.w = intensity;
        }

        float ambientColor[3] = { dirLight.ambient.x, dirLight.ambient.y, dirLight.ambient.z };
        if (ImGui::ColorEdit3("Ambient Color", ambientColor))
        {
            dirLight.ambient.x = ambientColor[0];
            dirLight.ambient.y = ambientColor[1];
            dirLight.ambient.z = ambientColor[2];
        }

        float specPower = dirLight.ambient.w;
        if (ImGui::SliderFloat("Specular Power", &specPower, 1.0f, 128.0f, "%.1f"))
        {
            dirLight.ambient.w = specPower;
        }

        ImGui::Separator();
        ImGui::Text("Real-time Shadows (PCF Soft Shadow)");
        bool shadowEnabled = lightMgr.IsShadowEnabled();
        if (ImGui::Checkbox("Enable Shadows", &shadowEnabled))
        {
            lightMgr.SetShadowEnabled(shadowEnabled);
        }

        if (shadowEnabled)
        {
            float orthoSize = lightMgr.GetOrthoSize();
            if (ImGui::SliderFloat("Shadow Range (Ortho)", &orthoSize, 10.0f, 100.0f, "%.1f"))
            {
                lightMgr.SetOrthoSize(orthoSize);
            }

            float shadowBias = lightMgr.GetShadowBias();
            if (ImGui::SliderFloat("Shadow Bias", &shadowBias, 0.0001f, 0.01f, "%.4f"))
            {
                lightMgr.SetShadowBias(shadowBias);
            }

            float shadowDarkness = lightMgr.GetShadowDarkness();
            if (ImGui::SliderFloat("Shadow Darkness", &shadowDarkness, 0.0f, 1.0f, "%.2f (0: dark, 1: light)"))
            {
                lightMgr.SetShadowDarkness(shadowDarkness);
            }
        }

        // Post-Process (Bloom) Settings
        ImGui::Spacing();
        ImGui::Text("Post-Process (Bloom)");
        bool bloomEnabled = lightMgr.IsBloomEnabled();
        if (ImGui::Checkbox("Enable Bloom", &bloomEnabled))
        {
            lightMgr.SetBloomEnabled(bloomEnabled);
        }

        if (bloomEnabled)
        {
            float threshold = lightMgr.GetBloomThreshold();
            if (ImGui::SliderFloat("Bloom Threshold", &threshold, 0.0f, 2.0f, "%.2f"))
            {
                lightMgr.SetBloomThreshold(threshold);
            }

            float bloomIntensity = lightMgr.GetBloomIntensity();
            if (ImGui::SliderFloat("Bloom Intensity", &bloomIntensity, 0.0f, 5.0f, "%.2f"))
            {
                lightMgr.SetBloomIntensity(bloomIntensity);
            }

            float bloomSpread = lightMgr.GetBloomSpread();
            if (ImGui::SliderFloat("Bloom Blur Spread", &bloomSpread, 0.2f, 3.0f, "%.2f"))
            {
                lightMgr.SetBloomSpread(bloomSpread);
            }
        }

        // Post-Process Filters (Grayscale, Sepia, Invert, Vignette)
        ImGui::Spacing();
        ImGui::Text("Post-Process Color Filter");
        const char* ppEffects[] = { "None", "Grayscale", "Sepia", "Invert", "Vignette" };
        int currentPPEffect = lightMgr.GetPostProcessEffectType();
        if (ImGui::Combo("Filter Effect", &currentPPEffect, ppEffects, IM_ARRAYSIZE(ppEffects)))
        {
            lightMgr.SetPostProcessEffectType(currentPPEffect);
        }

        ImGui::Separator();
    }

    // Inspector (Selected Object Details)
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
                        "FIELD", "TRIANGLE", "BILLBOARD", "EFFECT", "UI", "TEXT", "CAMERA", "FADE", "MANAGER", "LIGHT"
                    };
                    static const ObjectTag tagValues[] = {
                        ObjectTag::NONE, ObjectTag::BACKGROUND, ObjectTag::PLAYER, ObjectTag::PLAYER_BULLET,
                        ObjectTag::ENEMY, ObjectTag::ENEMY_BULLET, ObjectTag::FIELD, ObjectTag::TRIANGLE,
                        ObjectTag::BILLBOARD, ObjectTag::EFFECT, ObjectTag::UI, ObjectTag::TEXT,
                        ObjectTag::CAMERA, ObjectTag::FADE, ObjectTag::MANAGER, ObjectTag::LIGHT
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
                        // 髫主ｱ､諠・�ｱ
                        CTransform* parentTrans = transform->GetParent();
                        if (parentTrans)
                        {
                            CObject* parentObj = parentTrans->GetOwner();
                            CObjectInfo* pInfo = parentObj ? parentObj->GetComponent<CObjectInfo>() : nullptr;
                            std::string pName = pInfo ? pInfo->GetObjectName() : "CObject";
                            ImGui::Text("Parent: %s", pName.c_str());
                            ImGui::SameLine();
                            if (ImGui::SmallButton("Detach##Parent"))
                            {
                                transform->SetParent(nullptr, true);
                            }
                        }
                        else
                        {
                            ImGui::TextDisabled("Parent: None (Root)");
                        }

                        const auto& children = transform->GetChildren();
                        if (!children.empty())
                        {
                            ImGui::SameLine();
                            ImGui::TextDisabled(" | Children: %d", static_cast<int>(children.size()));
                        }

                        ImGui::Separator();

                        // 繝ｭ繝ｼ繧ｫ繝ｫ蟋ｿ蜍｢
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

                        // 繝ｯ繝ｼ繝ｫ繝牙ｧｿ蜍｢縺ｮ繝・ヰ繝・げ陦ｨ遉ｺ
                        if (ImGui::TreeNode("World Transform (Read Only)"))
                        {
                            DirectX::XMFLOAT3 wPos = transform->GetWorldPosition();
                            ImGui::Text("Position : (%.2f, %.2f, %.2f)", wPos.x, wPos.y, wPos.z);

                            DirectX::XMFLOAT4 wRot = transform->GetWorldRotation();
                            ImGui::Text("Rotation : (%.2f, %.2f, %.2f, %.2f)", wRot.x, wRot.y, wRot.z, wRot.w);

                            DirectX::XMFLOAT3 wScale = transform->GetWorldScale();
                            ImGui::Text("Scale    : (%.2f, %.2f, %.2f)", wScale.x, wScale.y, wScale.z);

                            DirectX::XMFLOAT3 front = transform->GetFront();
                            ImGui::Text("Front    : (%.2f, %.2f, %.2f)", front.x, front.y, front.z);

                            ImGui::TreePop();
                        }
                    }
                }

                CRectTransform* rectTransform = selectedObj->GetComponent<CRectTransform>();
                if (rectTransform)
                {
                    if (ImGui::CollapsingHeader("RectTransform", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text("Anchor Preset:");
                        AnchorPreset currentPreset = rectTransform->GetCurrentAnchorPreset();

                        auto AnchorBtn = [&](const char* label, AnchorPreset p, bool sameLine = true) {
                            if (currentPreset == p)
                            {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
                            }
                            if (ImGui::Button(label, ImVec2(40, 25)))
                            {
                                rectTransform->SetAnchorPreset(p, true);
                            }
                            if (currentPreset == p)
                            {
                                ImGui::PopStyleColor();
                            }
                            if (sameLine) ImGui::SameLine();
                        };

                        AnchorBtn("TL", AnchorPreset::TopLeft);
                        AnchorBtn("TC", AnchorPreset::TopCenter);
                        AnchorBtn("TR", AnchorPreset::TopRight, false);

                        AnchorBtn("ML", AnchorPreset::MiddleLeft);
                        AnchorBtn("MC", AnchorPreset::MiddleCenter);
                        AnchorBtn("MR", AnchorPreset::MiddleRight, false);

                        AnchorBtn("BL", AnchorPreset::BottomLeft);
                        AnchorBtn("BC", AnchorPreset::BottomCenter);
                        AnchorBtn("BR", AnchorPreset::BottomRight, false);

                        if (ImGui::Button("Stretch H", ImVec2(75, 22))) rectTransform->SetAnchorPreset(AnchorPreset::StretchHorizontal);
                        ImGui::SameLine();
                        if (ImGui::Button("Stretch V", ImVec2(75, 22))) rectTransform->SetAnchorPreset(AnchorPreset::StretchVertical);
                        ImGui::SameLine();
                        if (ImGui::Button("Stretch All", ImVec2(80, 22))) rectTransform->SetAnchorPreset(AnchorPreset::StretchAll);

                        ImGui::Separator();

                        DirectX::XMFLOAT2 pos = rectTransform->GetAnchoredPosition();
                        float posArr[2] = { pos.x, pos.y };
                        if (ImGui::DragFloat2("Pos (X, Y)", posArr, 1.0f))
                        {
                            rectTransform->SetAnchoredPosition(posArr[0], posArr[1]);
                        }

                        DirectX::XMFLOAT2 size = rectTransform->GetSizeDelta();
                        float sizeArr[2] = { size.x, size.y };
                        if (ImGui::DragFloat2("Size (W, H)", sizeArr, 1.0f, 0.0f, 4000.0f))
                        {
                            rectTransform->SetSizeDelta(sizeArr[0], sizeArr[1]);
                        }

                        DirectX::XMFLOAT2 pivot = rectTransform->GetPivot();
                        float pivotArr[2] = { pivot.x, pivot.y };
                        if (ImGui::SliderFloat2("Pivot (X, Y)", pivotArr, 0.0f, 1.0f, "%.2f"))
                        {
                            rectTransform->SetPivot(pivotArr[0], pivotArr[1]);
                        }

                        if (ImGui::Button("Center Pivot (0.5, 0.5)")) rectTransform->SetPivot(0.5f, 0.5f);
                        ImGui::SameLine();
                        if (ImGui::Button("TopLeft Pivot (0, 0)")) rectTransform->SetPivot(0.0f, 0.0f);

                        float rotZ = rectTransform->GetRotationZ();
                        if (ImGui::DragFloat("Rotation Z", &rotZ, 1.0f, -360.0f, 360.0f, "%.1f deg"))
                        {
                            rectTransform->SetRotationZ(rotZ);
                        }

                        DirectX::XMFLOAT2 scale = rectTransform->GetScale();
                        float scaleArr[2] = { scale.x, scale.y };
                        if (ImGui::DragFloat2("Scale (X, Y)", scaleArr, 0.01f, 0.0f, 10.0f))
                        {
                            rectTransform->SetScale(scaleArr[0], scaleArr[1]);
                        }

                        if (ImGui::TreeNode("Computed Screen Rect (Read Only)"))
                        {
                            DirectX::XMFLOAT4 sRect = rectTransform->GetScreenRect();
                            ImGui::Text("Screen Left-Top : (%.1f, %.1f)", sRect.x, sRect.y);
                            ImGui::Text("Screen Size     : %.1f x %.1f", sRect.z, sRect.w);
                            DirectX::XMFLOAT2 cPos = rectTransform->GetCenterPosition();
                            ImGui::Text("Pivot Position  : (%.1f, %.1f)", cPos.x, cPos.y);
                            ImGui::TreePop();
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

                        static const char* rLayerNames[] = { "Opaque", "Transparent", "Debug", "UI" };
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

                        static const char* rLayerNames[] = { "Opaque", "Transparent", "Debug", "UI" };
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
                                if (obj->GetComponent<ButtonComponent>())
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

                // FieldComponent
                FieldComponent* fComp = selectedObj->GetComponent<FieldComponent>();
                if (fComp)
                {
                    if (ImGui::CollapsingHeader("FieldComponent Settings", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        float w = fComp->GetWidth();
                        if (ImGui::DragFloat("Width", &w, 1.0f, 1.0f, 500.0f)) fComp->SetWidth(w);

                        float d = fComp->GetDepth();
                        if (ImGui::DragFloat("Depth", &d, 1.0f, 1.0f, 500.0f)) fComp->SetDepth(d);

                        int gx = fComp->GetGridX();
                        if (ImGui::InputInt("Grid X", &gx)) fComp->SetGridX(gx);

                        int gz = fComp->GetGridZ();
                        if (ImGui::InputInt("Grid Z", &gz)) fComp->SetGridZ(gz);

                        float tiling = fComp->GetUVTiling();
                        if (ImGui::DragFloat("UV Tiling", &tiling, 0.5f, 0.1f, 100.0f)) fComp->SetUVTiling(tiling);
                    }
                }

                // LightComponent
                LightComponent* lightComp = selectedObj->GetComponent<LightComponent>();
                if (lightComp)
                {
                    if (ImGui::CollapsingHeader("LightComponent Settings", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        // Light Type
                        const char* lightTypes[] = { "Directional", "Point", "Spot" };
                        int currentType = static_cast<int>(lightComp->GetLightType());
                        if (ImGui::Combo("Light Type", &currentType, lightTypes, IM_ARRAYSIZE(lightTypes)))
                        {
                            lightComp->SetLightType(static_cast<LightType>(currentType));
                        }

                        // Light Color
                        DirectX::XMFLOAT3 lColor = lightComp->GetColor();
                        float colArr[3] = { lColor.x, lColor.y, lColor.z };
                        if (ImGui::ColorEdit3("Light Color", colArr))
                        {
                            lightComp->SetColor(colArr[0], colArr[1], colArr[2]);
                        }

                        // Intensity
                        float intensity = lightComp->GetIntensity();
                        if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f, "%.2f"))
                        {
                            lightComp->SetIntensity(intensity);
                        }

                        // Ambient Color
                        DirectX::XMFLOAT3 ambColor = lightComp->GetAmbientColor();
                        float ambArr[3] = { ambColor.x, ambColor.y, ambColor.z };
                        if (ImGui::ColorEdit3("Ambient Color", ambArr))
                        {
                            lightComp->SetAmbientColor(ambArr[0], ambArr[1], ambArr[2]);
                        }

                        // Specular Power
                        float specPower = lightComp->GetSpecularPower();
                        if (ImGui::SliderFloat("Specular Power", &specPower, 1.0f, 128.0f, "%.1f"))
                        {
                            lightComp->SetSpecularPower(specPower);
                        }

                        ImGui::Separator();
                        ImGui::Text("Shadow Settings");
                        bool castShadow = lightComp->GetCastShadow();
                        if (ImGui::Checkbox("Cast Shadow", &castShadow))
                        {
                            lightComp->SetCastShadow(castShadow);
                        }

                        if (castShadow)
                        {
                            float bias = lightComp->GetShadowBias();
                            if (ImGui::SliderFloat("Shadow Bias", &bias, 0.0001f, 0.01f, "%.5f"))
                            {
                                lightComp->SetShadowBias(bias);
                            }

                            float darkness = lightComp->GetShadowDarkness();
                            if (ImGui::SliderFloat("Shadow Darkness", &darkness, 0.0f, 1.0f, "%.2f"))
                            {
                                lightComp->SetShadowDarkness(darkness);
                            }
                        }

                        if (lightComp->GetLightType() != LightType::Directional)
                        {
                            ImGui::Separator();
                            ImGui::Text("Attenuation & Shape");
                            float range = lightComp->GetRange();
                            if (ImGui::DragFloat("Range", &range, 0.5f, 0.1f, 500.0f, "%.1f"))
                            {
                                lightComp->SetRange(range);
                            }

                            if (lightComp->GetLightType() == LightType::Spot)
                            {
                                float spotAngle = lightComp->GetSpotAngle();
                                if (ImGui::SliderFloat("Spot Angle", &spotAngle, 1.0f, 179.0f, "%.1f deg"))
                                {
                                    lightComp->SetSpotAngle(spotAngle);
                                }
                            }
                        }
                    }
                }

                // BoxCollider3D Settings
                BoxCollider3D* boxCol = selectedObj->GetComponent<BoxCollider3D>();
                if (boxCol)
                {
                    if (ImGui::CollapsingHeader("BoxCollider3D Settings", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        DirectX::XMFLOAT3 size = boxCol->GetSize();
                        if (ImGui::DragFloat3("Size", &size.x, 0.05f, 0.01f, 1000.0f))
                        {
                            boxCol->SetSize(size);
                        }

                        DirectX::XMFLOAT3 offset = boxCol->GetOffset();
                        if (ImGui::DragFloat3("Center Offset##Box", &offset.x, 0.05f))
                        {
                            boxCol->SetOffset(offset);
                        }

                        bool isTrigger = boxCol->GetIsTrigger();
                        if (ImGui::Checkbox("Is Trigger##Box", &isTrigger))
                        {
                            boxCol->SetIsTrigger(isTrigger);
                        }

                        int layer = static_cast<int>(boxCol->GetLayer());
                        if (ImGui::InputInt("Layer##Box", &layer))
                        {
                            boxCol->SetLayer(static_cast<uint32_t>(layer));
                        }

                        int mask = static_cast<int>(boxCol->GetCollisionMask());
                        if (ImGui::InputInt("Collision Mask##Box", &mask))
                        {
                            boxCol->SetCollisionMask(static_cast<uint32_t>(mask));
                        }
                    }
                }

                // CapsuleCollider3D Settings
                CapsuleCollider3D* capCol = selectedObj->GetComponent<CapsuleCollider3D>();
                if (capCol)
                {
                    if (ImGui::CollapsingHeader("CapsuleCollider3D Settings", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        float radius = capCol->GetRadius();
                        if (ImGui::DragFloat("Radius##Cap", &radius, 0.05f, 0.01f, 100.0f))
                        {
                            capCol->SetRadius(radius);
                        }

                        float height = capCol->GetHeight();
                        if (ImGui::DragFloat("Height##Cap", &height, 0.05f, capCol->GetRadius() * 2.0f, 100.0f))
                        {
                            capCol->SetHeight(height);
                        }

                        DirectX::XMFLOAT3 offset = capCol->GetOffset();
                        if (ImGui::DragFloat3("Center Offset##Cap", &offset.x, 0.05f))
                        {
                            capCol->SetOffset(offset);
                        }

                        bool isTrigger = capCol->GetIsTrigger();
                        if (ImGui::Checkbox("Is Trigger##Cap", &isTrigger))
                        {
                            capCol->SetIsTrigger(isTrigger);
                        }

                        int layer = static_cast<int>(capCol->GetLayer());
                        if (ImGui::InputInt("Layer##Cap", &layer))
                        {
                            capCol->SetLayer(static_cast<uint32_t>(layer));
                        }

                        int mask = static_cast<int>(capCol->GetCollisionMask());
                        if (ImGui::InputInt("Collision Mask##Cap", &mask))
                        {
                            capCol->SetCollisionMask(static_cast<uint32_t>(mask));
                        }
                    }
                }

                // -------------------------------------------------------------
                // Model & Material Settings
                // -------------------------------------------------------------
                CModel* modelComp = selectedObj->GetComponent<CModel>();
                if (modelComp)
                {
                    if (ImGui::CollapsingHeader("Model & Materials", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text("Model: %s", modelComp->GetModelPath().c_str());
                        auto& materials = modelComp->GetMaterials();
                        size_t meshCount = modelComp->GetMeshCount();
                        ImGui::Text("Meshes: %d | Material Slots: %d", (int)meshCount, (int)materials.size());

                        // Highlight clear button
                        int currentHighlight = modelComp->GetSelectedMaterialIndex();
                        int currentHover = -1;
                        if (currentHighlight >= 0)
                        {
                            ImGui::SameLine();
                            if (ImGui::SmallButton("Clear Highlight"))
                            {
                                modelComp->SetSelectedMaterialIndex(-1);
                                currentHighlight = -1;
                            }
                        }
                        ImGui::Separator();

                        for (size_t matIdx = 0; matIdx < materials.size(); ++matIdx)
                        {
                            auto mat = materials[matIdx];
                            if (!mat) continue;

                            // Count how many meshes reference this material slot
                            int assignedMeshes = 0;
                            for (size_t mi = 0; mi < meshCount; ++mi)
                            {
                                if (modelComp->GetMeshMaterialIndex(mi) == matIdx)
                                {
                                    assignedMeshes++;
                                }
                            }

                            bool isSelected = (currentHighlight == static_cast<int>(matIdx));
                            std::string headerName = "Slot [" + std::to_string(matIdx) + "] (" + std::to_string(assignedMeshes) + " meshes)";
                            if (isSelected)
                            {
                                headerName += " [SELECTED]";
                            }

                            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                            if (isSelected) nodeFlags |= ImGuiTreeNodeFlags_Selected;

                            bool nodeOpen = ImGui::TreeNodeEx((void*)(uintptr_t)matIdx, nodeFlags, "%s", headerName.c_str());

                            if (ImGui::IsItemHovered())
                            {
                                currentHover = static_cast<int>(matIdx);
                            }

                            // Highlight slot on click
                            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                            {
                                if (isSelected)
                                    modelComp->SetSelectedMaterialIndex(-1);
                                else
                                    modelComp->SetSelectedMaterialIndex(static_cast<int>(matIdx));
                            }

                            if (nodeOpen)
                            {
                                ImGui::PushID((int)matIdx);

                                // Highlight toggle button inside
                                if (isSelected)
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.7f, 0.1f, 1.0f));
                                    if (ImGui::Button("Highlighting Active (Click to disable)"))
                                    {
                                        modelComp->SetSelectedMaterialIndex(-1);
                                    }
                                    ImGui::PopStyleColor();
                                }
                                else
                                {
                                    if (ImGui::Button("Highlight Meshes in Viewport"))
                                    {
                                        modelComp->SetSelectedMaterialIndex(static_cast<int>(matIdx));
                                    }
                                }

                                // Base Color
                                auto& matData = mat->GetData();
                                float col[4] = { matData.baseColor.x, matData.baseColor.y, matData.baseColor.z, matData.baseColor.w };
                                if (ImGui::ColorEdit4("Base Color", col))
                                {
                                    mat->SetColor(XMFLOAT4(col[0], col[1], col[2], col[3]));
                                }

                                // Roughness & Metallic
                                float roughness = mat->GetRoughness();
                                if (ImGui::SliderFloat("Roughness", &roughness, 0.01f, 1.0f))
                                {
                                    mat->SetRoughness(roughness);
                                }
                                float metallic = mat->GetMetallic();
                                if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f))
                                {
                                    mat->SetMetallic(metallic);
                                }

                                // UV Tiling & Offset
                                float uvTiling[2] = { matData.uvTiling.x, matData.uvTiling.y };
                                if (ImGui::DragFloat2("UV Tiling", uvTiling, 0.05f))
                                {
                                    mat->SetUVTiling(XMFLOAT2(uvTiling[0], uvTiling[1]));
                                }
                                float uvOffset[2] = { matData.uvOffset.x, matData.uvOffset.y };
                                if (ImGui::DragFloat2("UV Offset", uvOffset, 0.05f))
                                {
                                    mat->SetUVOffset(XMFLOAT2(uvOffset[0], uvOffset[1]));
                                }

                                // Blend Mode Combo
                                const char* blendModeNames[] = { "Opaque", "Alpha", "Additive" };
                                int currentBlend = static_cast<int>(mat->GetBlendMode());
                                if (ImGui::Combo("Blend Mode", &currentBlend, blendModeNames, IM_ARRAYSIZE(blendModeNames)))
                                {
                                    mat->SetBlendMode(static_cast<BlendMode>(currentBlend));
                                }

                                // Cull Mode Combo
                                const char* cullModeNames[] = { "None (Cull Off)", "Front", "Back" };
                                int currentCull = 2; // Default back
                                if (mat->GetCullMode() == D3D12_CULL_MODE_NONE) currentCull = 0;
                                else if (mat->GetCullMode() == D3D12_CULL_MODE_FRONT) currentCull = 1;
                                else currentCull = 2;

                                if (ImGui::Combo("Cull Mode", &currentCull, cullModeNames, IM_ARRAYSIZE(cullModeNames)))
                                {
                                    if (currentCull == 0) mat->SetCullMode(D3D12_CULL_MODE_NONE);
                                    else if (currentCull == 1) mat->SetCullMode(D3D12_CULL_MODE_FRONT);
                                    else mat->SetCullMode(D3D12_CULL_MODE_BACK);
                                }

                                // Shader File Path
                                std::wstring shaderWPath = mat->GetShaderFile();
                                std::string shaderStr(shaderWPath.begin(), shaderWPath.end());
                                char shaderBuf[260] = "";
                                strncpy_s(shaderBuf, shaderStr.c_str(), sizeof(shaderBuf) - 1);
                                if (ImGui::InputText("Shader File (.hlsl)", shaderBuf, sizeof(shaderBuf), ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                    std::string newShader = shaderBuf;
                                    mat->SetShader(std::wstring(newShader.begin(), newShader.end()), mat->GetVsEntry(), mat->GetPsEntry());
                                }

                                // Texture File Path & Load
                                std::wstring texWPath = mat->GetTextureFilePath();
                                std::string texPath(texWPath.begin(), texWPath.end());
                                ImGui::Text("Texture: %s", texPath.empty() ? "(Embedded / None)" : texPath.c_str());

                                char texBuf[260] = "";
                                strncpy_s(texBuf, texPath.c_str(), sizeof(texBuf) - 1);
                                if (ImGui::InputText("Replace Texture", texBuf, sizeof(texBuf), ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                    std::string newStr = texBuf;
                                    mat->LoadTexture(std::wstring(newStr.begin(), newStr.end()));
                                }

                                // Custom Params (Float4 x 2)
                                if (ImGui::TreeNode("Custom Shader Parameters"))
                                {
                                    float cp0[4] = { matData.customParams[0].x, matData.customParams[0].y, matData.customParams[0].z, matData.customParams[0].w };
                                    if (ImGui::DragFloat4("Param[0]", cp0, 0.05f))
                                    {
                                        mat->SetCustomParam(0, 0, cp0[0]); mat->SetCustomParam(0, 1, cp0[1]);
                                        mat->SetCustomParam(0, 2, cp0[2]); mat->SetCustomParam(0, 3, cp0[3]);
                                    }
                                    float cp1[4] = { matData.customParams[1].x, matData.customParams[1].y, matData.customParams[1].z, matData.customParams[1].w };
                                    if (ImGui::DragFloat4("Param[1]", cp1, 0.05f))
                                    {
                                        mat->SetCustomParam(1, 0, cp1[0]); mat->SetCustomParam(1, 1, cp1[1]);
                                        mat->SetCustomParam(1, 2, cp1[2]); mat->SetCustomParam(1, 3, cp1[3]);
                                    }
                                    ImGui::TreePop();
                                }

                                // Material File Operations (.mat load / assign / export)
                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Text("Material Asset (.mat):");

                                auto availableMatFiles = MaterialManager::GetInstance().GetAvailableMaterialFiles();
                                if (!availableMatFiles.empty())
                                {
                                    if (ImGui::BeginCombo("Select .mat", "Choose existing..."))
                                    {
                                        for (const auto& matPath : availableMatFiles)
                                        {
                                            if (ImGui::Selectable(matPath.c_str()))
                                            {
                                                auto newMat = MaterialManager::GetInstance().CreateInstance(matPath);
                                                if (newMat)
                                                {
                                                    modelComp->SetMaterial(newMat, static_cast<UINT>(matIdx));
                                                    CInspectorUI::GetInstance().SetStatusMessage("Applied material: " + matPath);
                                                }
                                            }
                                        }
                                        ImGui::EndCombo();
                                    }
                                }

                                static char slotMatFilePath[128] = "Assets/Materials/Default_Mesh.mat";
                                ImGui::InputText("Custom Path", slotMatFilePath, sizeof(slotMatFilePath));

                                if (ImGui::Button("Assign Path to Slot"))
                                {
                                    auto newMat = MaterialManager::GetInstance().CreateInstance(slotMatFilePath);
                                    if (newMat)
                                    {
                                        modelComp->SetMaterial(newMat, static_cast<UINT>(matIdx));
                                        CInspectorUI::GetInstance().SetStatusMessage("Assigned material asset.");
                                    }
                                    else
                                    {
                                        CInspectorUI::GetInstance().SetStatusMessage("Failed to load material asset.");
                                    }
                                }
                                ImGui::SameLine();
                                if (ImGui::Button("Export / Save (.mat)"))
                                {
                                    if (mat->SaveToFile(slotMatFilePath))
                                    {
                                        CInspectorUI::GetInstance().SetStatusMessage("Saved material to " + std::string(slotMatFilePath));
                                    }
                                }

                                ImGui::PopID();
                                ImGui::TreePop();
                            }
                        }

                        modelComp->SetHoveredMaterialIndex(currentHover);
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
                    if (cPtr != transform && cPtr != sprite && cPtr != textComp &&
                        cPtr != btnComp && cPtr != ecComp && cPtr != objInfo &&
                        cPtr != lightComp && cPtr != boxCol && cPtr != capCol &&
                        cPtr != modelComp)
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
                    if (MatchesFilter("Rect Transform Component (CRectTransform)") && !selectedObj->GetComponent<CRectTransform>())
                    {
                        if (ImGui::Selectable("Rect Transform Component (CRectTransform)"))
                        {
                            selectedObj->AddComponent<CRectTransform>();
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
                    if (MatchesFilter("Capsule Collider 3D (CapsuleCollider3D)") && !selectedObj->GetComponent<CapsuleCollider3D>())
                    {
                        if (ImGui::Selectable("Capsule Collider 3D (CapsuleCollider3D)"))
                        {
                            selectedObj->AddComponent<CapsuleCollider3D>();
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
                    if (MatchesFilter("Light Component") && !selectedObj->GetComponent<LightComponent>())
                    {
                        if (ImGui::Selectable("Light Component"))
                        {
                            selectedObj->AddComponent<LightComponent>();
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

    // 3D Transform Gizmo: If Viewport is hidden, draw on BackgroundDrawList
    if (!CViewportUI::GetInstance().IsVisible())
    {
        ImGuiIO& io = ImGui::GetIO();
        DrawGizmo(ImGui::GetBackgroundDrawList(), ImVec2(0.0f, 0.0f), io.DisplaySize);
        DrawColliders(ImGui::GetBackgroundDrawList(), ImVec2(0.0f, 0.0f), io.DisplaySize);
        DrawLights(ImGui::GetBackgroundDrawList(), ImVec2(0.0f, 0.0f), io.DisplaySize);
    }

}


void CInspectorUI::DrawGizmo(ImDrawList* drawList, const ImVec2& vpPos, const ImVec2& vpSize)
{
    if (!drawList || vpSize.x <= 0.0f || vpSize.y <= 0.0f) return;

    // EditMode または PrefabEditMode のときのみギズモを描画・操作する
    if (!m_isEditMode && !m_isPrefabEditMode)
    {
        m_isDraggingGizmo = false;
        m_draggedAxis = -1;
        return;
    }

    DirectX::XMMATRIX view;
    DirectX::XMMATRIX proj;
    if (m_isEditMode || m_isPrefabEditMode)
    {
        view = EditorCamera::GetInstance().GetViewMatrix();
        proj = EditorCamera::GetInstance().GetProjectionMatrix();
    }
    else
    {
        CameraComponent* camera = ObjectManager::GetInstance().GetCamera();
        if (!camera) return;
        view = camera->GetViewMatrix();
        proj = camera->GetProjectionMatrix();
    }

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

    if (!targetObj) return;

    CTransform* transform = targetObj->GetComponent<CTransform>();
    if (!transform) return;

    DirectX::XMFLOAT3 pos = transform->GetPos();


    DirectX::XMVECTOR vOrigin = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
    DirectX::XMVECTOR vAxisX = DirectX::XMVectorSet(pos.x + 1.5f, pos.y, pos.z, 1.0f);
    DirectX::XMVECTOR vAxisY = DirectX::XMVectorSet(pos.x, pos.y + 1.5f, pos.z, 1.0f);
    DirectX::XMVECTOR vAxisZ = DirectX::XMVectorSet(pos.x, pos.y, pos.z + 1.5f, 1.0f);

    D3D12_VIEWPORT viewport{};
    viewport.Width = vpSize.x;
    viewport.Height = vpSize.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = vpPos.x;
    viewport.TopLeftY = vpPos.y;

    DirectX::XMVECTOR pOrigin = DirectX::XMVector3Project(vOrigin, viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());
    DirectX::XMVECTOR pAxisX = DirectX::XMVector3Project(vAxisX, viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());
    DirectX::XMVECTOR pAxisY = DirectX::XMVector3Project(vAxisY, viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());
    DirectX::XMVECTOR pAxisZ = DirectX::XMVector3Project(vAxisZ, viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, proj, view, DirectX::XMMatrixIdentity());

    DirectX::XMFLOAT3 fOrigin, fAxisX, fAxisY, fAxisZ;
    DirectX::XMStoreFloat3(&fOrigin, pOrigin);
    DirectX::XMStoreFloat3(&fAxisX, pAxisX);
    DirectX::XMStoreFloat3(&fAxisY, pAxisY);
    DirectX::XMStoreFloat3(&fAxisZ, pAxisZ);

    bool axisValid[3] = { fOrigin.z > 0.0f && fAxisX.z > 0.0f, fOrigin.z > 0.0f && fAxisY.z > 0.0f, fOrigin.z > 0.0f && fAxisZ.z > 0.0f };
    if (!axisValid[0] && !axisValid[1] && !axisValid[2]) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;

    // Viewportの画像範囲内にクリッピングをかける
    drawList->PushClipRect(vpPos, ImVec2(vpPos.x + vpSize.x, vpPos.y + vpSize.y), true);

    ImVec2 originScreen = ImVec2(fOrigin.x, fOrigin.y);
    ImVec2 axisEndScreen[3] = {
        ImVec2(fAxisX.x, fAxisX.y),
        ImVec2(fAxisY.x, fAxisY.y),
        ImVec2(fAxisZ.x, fAxisZ.y)
    };

    int hoverAxis = -1;
    float minHoverDist = 16.0f; // Threshold in pixels

    // マウスがビューポート画像矩形内にあるか、またはすでにドラッグ中である場合のみホバー/ドラッグ判定
    bool mouseInVp = (mousePos.x >= vpPos.x && mousePos.x <= vpPos.x + vpSize.x &&
                      mousePos.y >= vpPos.y && mousePos.y <= vpPos.y + vpSize.y);

    if (!m_isDraggingGizmo && mouseInVp)
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
                        if (a == 1) newPos.y += projAmount * factor;
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
                        if (a == 1) newScale.y = (std::max)(0.01f, m_dragStartVal.y + projAmount * factor);
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

    drawList->PopClipRect();
}


void CInspectorUI::DrawColliders(ImDrawList* drawList, const ImVec2& vpPos, const ImVec2& vpSize)
{
    if (!m_showColliders) return;

    CameraComponent* camera = ObjectManager::GetInstance().GetCamera();
    if (!camera && !m_isEditMode && !m_isPrefabEditMode) return;

    if (m_isPrefabEditMode && m_prefabEditTarget)
    {
        BoxCollider3D* bCol = m_prefabEditTarget->GetComponent<BoxCollider3D>();
        if (bCol) bCol->DrawDebug(camera, drawList, vpPos, vpSize);

        CapsuleCollider3D* cCol = m_prefabEditTarget->GetComponent<CapsuleCollider3D>();
        if (cCol) cCol->DrawDebug(camera, drawList, vpPos, vpSize);
        return;
    }

    const auto& objectList = ObjectManager::GetInstance().GetObjectList();
    for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
    {
        for (const auto& obj : objectList[tagIdx])
        {
            if (!obj || obj->GetIsDestroyed()) continue;
            BoxCollider3D* boxCollider = obj->GetComponent<BoxCollider3D>();
            if (boxCollider)
            {
                boxCollider->DrawDebug(camera, drawList, vpPos, vpSize);
            }

            CapsuleCollider3D* capsuleCollider = obj->GetComponent<CapsuleCollider3D>();
            if (capsuleCollider)
            {
                capsuleCollider->DrawDebug(camera, drawList, vpPos, vpSize);
            }
        }
    }
}

void CInspectorUI::DrawLights(ImDrawList* drawList, const ImVec2& vpPos, const ImVec2& vpSize)
{
    if (!m_showLights) return;

    CameraComponent* camera = ObjectManager::GetInstance().GetCamera();
    if (!camera && !m_isEditMode && !m_isPrefabEditMode) return;

    if (m_isPrefabEditMode && m_prefabEditTarget)
    {
        LightComponent* lightComp = m_prefabEditTarget->GetComponent<LightComponent>();
        if (lightComp)
        {
            lightComp->DrawDebug(camera, drawList, vpPos, vpSize);
        }
        return;
    }

    const auto& objectList = ObjectManager::GetInstance().GetObjectList();
    for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
    {
        for (const auto& obj : objectList[tagIdx])
        {
            if (!obj || obj->GetIsDestroyed()) continue;
            LightComponent* lightComp = obj->GetComponent<LightComponent>();
            if (lightComp)
            {
                lightComp->DrawDebug(camera, drawList, vpPos, vpSize);
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

