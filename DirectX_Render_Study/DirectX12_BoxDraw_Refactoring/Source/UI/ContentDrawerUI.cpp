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

// -----------------------------------------------------------------
// Assets/Prefabs フォルダ内の JSON プレハブ一覧をスキャン・更新する
// -----------------------------------------------------------------
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
                // パス区切り文字をスラッシュに統一
                std::replace(item.path.begin(), item.path.end(), '\\', '/');
                item.name = entry.path().stem().string();

                // JSONファイルを開いてタグ情報を読み取る
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

// -----------------------------------------------------------------
// コンテンツドロワーのImGuiウィンドウを描画する
// -----------------------------------------------------------------
void CContentDrawerUI::Draw()
{
    // ショートカットキー (Ctrl + Space) による表示切り替え
    if (CInputManager::GetInstance().IsKeyPress(VK_CONTROL) && CInputManager::GetInstance().IsKeyTrigger(VK_SPACE))
    {
        ToggleVisible();
    }

    if (!m_isVisible) return;

    // 未初期化の場合はプレハブ一覧を取得
    if (!m_isInitialized)
    {
        RefreshPrefabList();
    }

    ImGui::SetNextWindowSize(ImVec2(650, 260), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Content Drawer (Prefabs)", &m_isVisible, ImGuiWindowFlags_NoCollapse))
    {
        // 手動更新ボタン
        if (ImGui::Button("Refresh"))
        {
            RefreshPrefabList();
        }
        ImGui::SameLine();
        
        // 検索フィルター入力欄
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("Filter", m_searchFilter, sizeof(m_searchFilter));

        ImGui::SameLine();
        ImGui::TextDisabled("Shortcut: Ctrl + Space");

        ImGui::Separator();

        // プレハブカードのグリッドレイアウト設定
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float cardWidth = 180.0f;
        int columns = static_cast<int>(windowWidth / (cardWidth + 12.0f));
        if (columns < 1) columns = 1;

        if (ImGui::BeginTable("PrefabGrid", columns))
        {
            for (size_t i = 0; i < m_prefabItems.size(); ++i)
            {
                const auto& item = m_prefabItems[i];

                // 検索フィルター適用
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
                
                // プレハブ情報の表示（名前・タグ）
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "[Prefab] %s", item.name.c_str());
                ImGui::TextDisabled("Tag: %s", item.tag.c_str());

                // プレハブ編集ボタン（インスペクターで編集モードを開く）
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.1f, 1.0f));
                if (ImGui::Button("Edit Prefab"))
                {
                    CInspectorUI::GetInstance().OpenPrefabEditMode(item.path);
                }
                ImGui::PopStyleColor();

                // カメラ前方に生成するボタン
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

                // ワールド原点 (0, 0, 0) に生成するボタン
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