#include "ViewportUI.h"
#include "Object.h"
#include "InspectorUI.h"
#include "ContentDrawerUI.h"
#include "D2DTextRenderer.h"
#include "ModelManager.h"
#include "Model.h"
#include "PrefabManager.h"
#include "MaterialManager.h"
#include "ObjectManager.h"
#include "EditorCamera.h"
#include "EditorRaycast.h"
#include "UndoManager.h"
#include "CameraComponent.h"
#include "SpriteRenderer.h"
#include "Transform.h"
#include "BoxCollider3D.h"
#include "SceneManager.h"
#include "HotReloadManager.h"
#include <algorithm>
#include <filesystem>
#include <Windows.h>

void CViewportUI::Draw()
{
    if (!m_isVisible) return;

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;
    float toolbarH = 48.0f;
    float hierarchyW = 320.0f;
    float inspectorW = 460.0f;
    float centerW = screenW - hierarchyW - inspectorW;
    float drawerH = CContentDrawerUI::GetInstance().IsVisible() ? 280.0f : 0.0f;
    float viewportH = screenH - toolbarH - drawerH;

    ImGui::SetNextWindowPos(ImVec2(hierarchyW, toolbarH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(centerW, viewportH), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    // パディングゼロにして画面端までぴったり合わせる
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Viewport", &m_isVisible, flags))
    {
        m_isHovered = ImGui::IsWindowHovered();
        m_isFocused = ImGui::IsWindowFocused();
        m_viewportPos = ImGui::GetWindowPos();
        m_viewportSize = ImGui::GetWindowSize();

        ImVec2 contentSize = ImGui::GetContentRegionAvail();

        if (contentSize.x > 0.0f && contentSize.y > 0.0f && m_isTextureAvailable && m_textureSRV.ptr != 0)
        {
            // 16:9 のアスペクト比を維持
            const float targetAspect = 16.0f / 9.0f;
            const float currentAspect = contentSize.x / contentSize.y;

            float renderW = contentSize.x;
            float renderH = contentSize.y;

            if (currentAspect > targetAspect)
            {
                // 横長の場合（ピラーボックス: 左右に余白）
                renderW = contentSize.y * targetAspect;
                renderH = contentSize.y;
            }
            else
            {
                // 縦長の場合（レターボックス: 上下に余白）
                renderW = contentSize.x;
                renderH = contentSize.x / targetAspect;
            }

            // 中央揃えのためのオフセット計算
            float offsetX = (contentSize.x - renderW) * 0.5f;
            float offsetY = (contentSize.y - renderH) * 0.5f;

            ImVec2 cursorPos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursorPos.x + offsetX, cursorPos.y + offsetY));

            m_imagePos = ImGui::GetCursorScreenPos();
            m_imageSize = ImVec2(renderW, renderH);

            // ImTextureID に変換して描画
            ImTextureID texID = static_cast<ImTextureID>(m_textureSRV.ptr);
            ImGui::Image(texID, ImVec2(renderW, renderH));

            // Drag & Drop Target: Spawn / Apply asset from Content Drawer
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_DRAWER_PATH"))
                {
                    const char* assetPath = (const char*)payload->Data;
                    HandleAssetDropOnViewport(assetPath);
                }
                ImGui::EndDragDropTarget();
            }

            // 1. 3Dトランスフォームギズモ描画（Viewportウィンドウ内DrawList）
            CInspectorUI::GetInstance().DrawColliders(ImGui::GetWindowDrawList(), m_imagePos, m_imageSize);
            CInspectorUI::GetInstance().DrawLights(ImGui::GetWindowDrawList(), m_imagePos, m_imageSize);
            CInspectorUI::GetInstance().DrawGizmo(ImGui::GetWindowDrawList(), m_imagePos, m_imageSize);

            // 2. 2Dテキストのオーバーレイ描画
            const auto& textQueue = D2DTextRenderer::GetInstance().GetTextQueue();
            if (!textQueue.empty())
            {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->PushClipRect(m_imagePos, ImVec2(m_imagePos.x + m_imageSize.x, m_imagePos.y + m_imageSize.y), true);

                const float scaleX = renderW / 1920.0f;
                const float scaleY = renderH / 1080.0f;

                for (const auto& info : textQueue)
                {
                    if (info.text.empty()) continue;

                    // UTF-16 -> UTF-8 変換
                    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, info.text.c_str(), (int)info.text.length(), nullptr, 0, nullptr, nullptr);
                    if (sizeNeeded <= 0) continue;

                    std::string utf8Text(sizeNeeded, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, info.text.c_str(), (int)info.text.length(), &utf8Text[0], sizeNeeded, nullptr, nullptr);

                    ImVec2 textPos(m_imagePos.x + info.x * scaleX, m_imagePos.y + info.y * scaleY);
                    ImU32 textColor = ImColor(info.color.r, info.color.g, info.color.b, info.color.a);

                    float scaledFontSize = info.fontSize * scaleY;
                    if (scaledFontSize < 1.0f) scaledFontSize = 1.0f;

                    drawList->AddText(ImGui::GetFont(), scaledFontSize, textPos, textColor, utf8Text.c_str());
                }

                drawList->PopClipRect();
            }
        }
        else
        {
            m_imagePos = m_viewportPos;
            m_imageSize = ImVec2(0.0f, 0.0f);

            // フォールバック表示（テクスチャ未受信時）
            ImVec2 cursorPos = ImGui::GetCursorPos();
            const char* msg = "No Render Texture Available";
            ImVec2 textSize = ImGui::CalcTextSize(msg);
            float offsetX = (contentSize.x - textSize.x) * 0.5f;
            float offsetY = (contentSize.y - textSize.y) * 0.5f;
            if (offsetX < 0.0f) offsetX = 0.0f;
            if (offsetY < 0.0f) offsetY = 0.0f;

            ImGui::SetCursorPos(ImVec2(cursorPos.x + offsetX, cursorPos.y + offsetY));
            ImGui::TextDisabled("%s", msg);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

bool CViewportUI::GetNormalizedMousePos(float& outX, float& outY) const
{
    if (m_imageSize.x <= 0.0f || m_imageSize.y <= 0.0f)
    {
        return false;
    }

    ImVec2 mousePos = ImGui::GetMousePos();
    float relX = mousePos.x - m_imagePos.x;
    float relY = mousePos.y - m_imagePos.y;

    if (relX >= 0.0f && relX <= m_imageSize.x &&
        relY >= 0.0f && relY <= m_imageSize.y)
    {
        outX = relX / m_imageSize.x;
        outY = relY / m_imageSize.y;
        return true;
    }

    return false;
}


void CViewportUI::HandleAssetDropOnViewport(const std::string& assetPath)
{
    float normX = 0.0f, normY = 0.0f;
    if (!GetNormalizedMousePos(normX, normY))
    {
        return;
    }

    float screenX = normX * 1920.0f;
    float screenY = normY * 1080.0f;

    // View / Proj matrix from editor camera or scene camera
    CameraComponent* camera = ObjectManager::GetInstance().GetCamera();
    bool isEditMode = CInspectorUI::GetInstance().IsEditMode() || CInspectorUI::GetInstance().IsPrefabEditMode();
    DirectX::XMMATRIX view = isEditMode ? EditorCamera::GetInstance().GetViewMatrix() : (camera ? camera->GetViewMatrix() : DirectX::XMMatrixIdentity());
    DirectX::XMMATRIX proj = isEditMode ? EditorCamera::GetInstance().GetProjectionMatrix() : (camera ? camera->GetProjectionMatrix() : DirectX::XMMatrixIdentity());

    // Create Ray from screen mouse coordinates
    Ray ray = EditorRaycast::CreateRayFromScreen(screenX, screenY, 1920.0f, 1080.0f, view, proj);

    // Pick existing object under cursor
    int hitTag = -1, hitObj = -1, hitMesh = -1, hitMatSlot = -1;
    const auto& objectList = ObjectManager::GetInstance().GetObjectList();
    CObject* hitObjPtr = EditorRaycast::PickObject(screenX, screenY, 1920.0f, 1080.0f, view, proj, objectList, hitTag, hitObj, &hitMesh, &hitMatSlot);

    DirectX::XMFLOAT3 spawnPos = { 0.0f, 0.0f, 0.0f };

    if (hitObjPtr)
    {
        CTransform* hitTrans = hitObjPtr->GetComponent<CTransform>();
        BoxCollider3D* hitCol = hitObjPtr->GetComponent<BoxCollider3D>();
        if (hitTrans)
        {
            DirectX::XMFLOAT3 pos = hitTrans->GetPos();
            DirectX::XMFLOAT3 scale = hitTrans->GetScale();
            float topY = pos.y + (hitCol ? (hitCol->GetOffset().y + hitCol->GetSize().y * scale.y * 0.5f) : 1.0f);
            spawnPos = { pos.x, topY, pos.z };
        }
        else
        {
            spawnPos = { ray.origin.x + ray.direction.x * 5.0f, ray.origin.y + ray.direction.y * 5.0f, ray.origin.z + ray.direction.z * 5.0f };
        }
    }
    else
    {
        // Ground plane (Y = 0) intersection
        if (ray.direction.y < -1e-4f)
        {
            float t = -ray.origin.y / ray.direction.y;
            if (t > 0.0f)
            {
                spawnPos.x = ray.origin.x + ray.direction.x * t;
                spawnPos.y = 0.0f;
                spawnPos.z = ray.origin.z + ray.direction.z * t;
            }
            else
            {
                spawnPos.x = ray.origin.x + ray.direction.x * 10.0f;
                spawnPos.y = ray.origin.y + ray.direction.y * 10.0f;
                spawnPos.z = ray.origin.z + ray.direction.z * 10.0f;
            }
        }
        else
        {
            // Upwards or horizontal: spawn 10 units in front of camera
            spawnPos.x = ray.origin.x + ray.direction.x * 10.0f;
            spawnPos.y = ray.origin.y + ray.direction.y * 10.0f;
            spawnPos.z = ray.origin.z + ray.direction.z * 10.0f;
        }
    }

    std::filesystem::path p(assetPath);
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::string baseName = p.stem().string();

    auto SelectSpawned = [](CObject* target) {
        if (!target) return;
        const auto& objList = ObjectManager::GetInstance().GetObjectList();
        for (size_t t = 0; t < objList.size(); ++t)
        {
            for (size_t o = 0; o < objList[t].size(); ++o)
            {
                if (objList[t][o].get() == target)
                {
                    CInspectorUI::GetInstance().SetSelectedObject(static_cast<int>(t), static_cast<int>(o));
                    return;
                }
            }
        }
    };

    if (ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".fbx")
    {
        auto sharedModel = ModelManager::GetInstance().GetModel(assetPath);
        if (sharedModel)
        {
            static int modelCounter = 0;
            std::string instName = baseName + "_" + std::to_string(++modelCounter);
            CObject* newObj = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::NONE, "CObject", instName);
            if (newObj)
            {
                CModel* m = newObj->GetComponent<CModel>();
                if (!m) m = newObj->AddComponent<CModel>();
                if (m)
                {
                    m->CopyFrom(sharedModel);
                    m->SetModelPath(assetPath);
                }
                CTransform* trans = newObj->GetComponent<CTransform>();
                if (trans) trans->SetPos(spawnPos);

                newObj->Init();
                newObj->Awake();

                UndoManager::GetInstance().RecordCommand(std::make_unique<SpawnObjectUndoCommand>(newObj));
                SelectSpawned(newObj);
                CInspectorUI::GetInstance().SetStatusMessage("Spawned Model: " + instName, 3.5f);
            }
        }
        else
        {
            CInspectorUI::GetInstance().SetStatusMessage("Failed to load model: " + assetPath, 4.0f);
        }
    }
    else if (ext == ".prefab")
    {
        static int prefabCounter = 0;
        std::string instName = baseName + "_" + std::to_string(++prefabCounter);
        CObject* newObj = PrefabManager::GetInstance().InstantiateFromJSON(assetPath, instName);
        if (newObj)
        {
            CTransform* trans = newObj->GetComponent<CTransform>();
            if (trans) trans->SetPos(spawnPos);

            newObj->Init();
            newObj->Awake();

            UndoManager::GetInstance().RecordCommand(std::make_unique<SpawnObjectUndoCommand>(newObj));
            SelectSpawned(newObj);
            CInspectorUI::GetInstance().SetStatusMessage("Spawned Prefab: " + instName, 3.5f);
        }
        else
        {
            CInspectorUI::GetInstance().SetStatusMessage("Failed to instantiate prefab: " + assetPath, 4.0f);
        }
    }
    else if (ext == ".mat")
    {
        if (hitObjPtr)
        {
            CModel* model = hitObjPtr->GetComponent<CModel>();
            if (model && !model->GetMaterials().empty())
            {
                auto matInst = MaterialManager::GetInstance().CreateInstance(assetPath);
                if (matInst)
                {
                    int slot = (hitMatSlot >= 0 && hitMatSlot < static_cast<int>(model->GetMaterials().size()))
                        ? hitMatSlot
                        : model->GetSelectedMaterialIndex();
                    if (slot >= 0 && slot < static_cast<int>(model->GetMaterials().size()))
                    {
                        model->SetMaterial(matInst, static_cast<UINT>(slot));
                    }
                    else
                    {
                        for (size_t mIdx = 0; mIdx < model->GetMaterials().size(); ++mIdx)
                        {
                            model->SetMaterial(matInst->Clone(), static_cast<UINT>(mIdx));
                        }
                    }
                    SelectSpawned(hitObjPtr);
                    CInspectorUI::GetInstance().SetStatusMessage("Applied material to " + hitObjPtr->GetName() + ": " + baseName, 3.5f);
                }
            }
            else
            {
                CInspectorUI::GetInstance().SetStatusMessage("Hit object has no 3D Model.", 3.0f);
            }
        }
        else
        {
            CInspectorUI::GetInstance().SetStatusMessage("Drop .mat onto a 3D object to apply it.", 3.0f);
        }
    }
    else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".dds" || ext == ".tga")
    {
        if (hitObjPtr)
        {
            int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, assetPath.c_str(), static_cast<int>(assetPath.size()), nullptr, 0);
            std::wstring wTex(sizeNeeded, 0);
            MultiByteToWideChar(CP_UTF8, 0, assetPath.c_str(), static_cast<int>(assetPath.size()), &wTex[0], sizeNeeded);

            if (CSpriteRenderer* spr = hitObjPtr->GetComponent<CSpriteRenderer>())
            {
                spr->SetTexture(wTex);
                SelectSpawned(hitObjPtr);
                CInspectorUI::GetInstance().SetStatusMessage("Applied texture to Sprite: " + hitObjPtr->GetName(), 3.5f);
            }
            else if (CModel* model = hitObjPtr->GetComponent<CModel>())
            {
                int slot = (hitMatSlot >= 0 && hitMatSlot < static_cast<int>(model->GetMaterials().size()))
                    ? hitMatSlot
                    : model->GetSelectedMaterialIndex();
                if (slot >= 0 && slot < static_cast<int>(model->GetMaterials().size()) && model->GetMaterial(slot))
                {
                    model->GetMaterial(slot)->LoadTexture(wTex);
                }
                else
                {
                    for (auto& mat : model->GetMaterials())
                    {
                        if (mat) mat->LoadTexture(wTex);
                    }
                }
                SelectSpawned(hitObjPtr);
                CInspectorUI::GetInstance().SetStatusMessage("Applied texture to Model: " + hitObjPtr->GetName(), 3.5f);
            }
        }
        else
        {
            CInspectorUI::GetInstance().SetStatusMessage("Drop texture onto a 3D object to apply it.", 3.0f);
        }
    }
    else if (ext == ".scene")
    {
        SceneManager::GetInstance().ChangeSceneByPath(assetPath, !CInspectorUI::GetInstance().IsEditMode(), 0.4f);
        HotReloadManager::GetInstance().CaptureSceneSnapshot();
        CInspectorUI::GetInstance().SetStatusMessage("Loaded Scene: " + assetPath, 3.0f);
    }
}
