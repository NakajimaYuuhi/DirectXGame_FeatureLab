#include "AssetSecurityUI.h"
#include "Source/Resources/AssetEncryptor.h"
#include "Source/Resources/AssetHeader.h"
#include "imgui.h"
#include <filesystem>
#include <fstream>
#include <iostream>

void CAssetSecurityUI::RefreshModelList()
{
    m_modelList.clear();
    const std::string modelDir = "Assets/Model";

    if (!std::filesystem::exists(modelDir)) return;

    for (const auto& entry : std::filesystem::directory_iterator(modelDir))
    {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        if (ext != ".glb" && ext != ".dat") continue;

        ModelAssetInfo info;
        info.filename = entry.path().filename().string();
        info.fullPath = entry.path().string();
        info.fileSize = entry.file_size();

        // マジックナンバーを読んで暗号化済みか判定
        std::ifstream file(info.fullPath, std::ios::binary);
        if (file.is_open())
        {
            uint32_t magic = 0;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            info.isEncrypted = (magic == AssetSecurity::ASSET_MAGIC_ENCRYPTED);
        }

        m_modelList.push_back(info);
    }
}

void CAssetSecurityUI::Draw()
{
#ifndef _DEBUG
    return;
#endif

    if (!m_isVisible) return;

    if (!m_isInitialized)
    {
        RefreshModelList();
        m_isInitialized = true;
    }

    ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Asset Security & Model Encryptor", &m_isVisible))
    {
        ImGui::Text("Model Assets Encryption Tool (XOR Cipher)");
        ImGui::Separator();

        // ツールボタン群
        if (ImGui::Button("Refresh List"))
        {
            RefreshModelList();
            m_statusMessage = "List refreshed.";
        }
        ImGui::SameLine();
        if (ImGui::Button("Batch Encrypt All GLB Models"))
        {
            int encryptedCount = 0;
            for (const auto& item : m_modelList)
            {
                if (item.filename.ends_with(".glb"))
                {
                    std::string outPath = item.fullPath.substr(0, item.fullPath.length() - 4) + ".dat";
                    if (AssetSecurity::EncryptModelFile(item.fullPath, outPath))
                    {
                        encryptedCount++;
                    }
                }
            }
            RefreshModelList();
            m_statusMessage = "Batch encrypted " + std::to_string(encryptedCount) + " models.";
        }

        if (!m_statusMessage.empty())
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "[Status] %s", m_statusMessage.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // テーブル表示
        if (ImGui::BeginTable("ModelTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableHeadersRow();

            for (const auto& item : m_modelList)
            {
                ImGui::TableNextRow();

                // File Name
                ImGui::TableNextColumn();
                ImGui::Text("%s", item.filename.c_str());

                // Size
                ImGui::TableNextColumn();
                if (item.fileSize > 1024 * 1024)
                {
                    ImGui::Text("%.2f MB", static_cast<float>(item.fileSize) / (1024.0f * 1024.0f));
                }
                else
                {
                    ImGui::Text("%.1f KB", static_cast<float>(item.fileSize) / 1024.0f);
                }

                // Status
                ImGui::TableNextColumn();
                if (item.isEncrypted)
                {
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.3f, 1.0f), "[Encrypted]");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Plaintext");
                }

                // Action
                ImGui::TableNextColumn();
                std::string btnId = "##" + item.filename;
                if (!item.isEncrypted && item.filename.ends_with(".glb"))
                {
                    if (ImGui::Button(("Encrypt" + btnId).c_str()))
                    {
                        std::string outPath = item.fullPath.substr(0, item.fullPath.length() - 4) + ".dat";
                        if (AssetSecurity::EncryptModelFile(item.fullPath, outPath))
                        {
                            m_statusMessage = "Encrypted: " + item.filename;
                            RefreshModelList();
                        }
                    }
                }
                else if (item.isEncrypted)
                {
                    if (ImGui::Button(("Decrypt" + btnId).c_str()))
                    {
                        std::string outPath = item.fullPath.substr(0, item.fullPath.length() - 4) + "_dec.glb";
                        if (AssetSecurity::DecryptModelFile(item.fullPath, outPath))
                        {
                            m_statusMessage = "Decrypted to: " + outPath;
                            RefreshModelList();
                        }
                    }
                }
                else
                {
                    ImGui::TextDisabled("-");
                }
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}
