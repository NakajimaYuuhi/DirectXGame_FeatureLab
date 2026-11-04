#include "ContentDrawerUI.h"
#include "AssetRenameService.h"
#include "AssetDeleteService.h"
#include "UndoManager.h"
#include "InspectorUI.h"
#include "imgui.h"
#include "PrefabManager.h"
#include "ObjectManager.h"
#include "Transform.h"
#include "ObjectInfo.h"
#include "InputManager.h"
#include "Model.h"
#include "ModelManager.h"
#include "MaterialManager.h"
#include "ShaderManager.h"
#include "SpriteRenderer.h"
#include "HotReloadManager.h"
#include "DX12Manager.h"
#include "RenderPipeline.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/Scenes/Base/Scene.h"
#include "Source/Rendering/TextureManager.h"
#include "Source/External/json.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

namespace fs = std::filesystem;

static std::string NormalizePathSlash(const std::string& p)
{
    std::string res = p;
    std::replace(res.begin(), res.end(), '\\', '/');
    while (!res.empty() && res.back() == '/')
    {
        res.pop_back();
    }
    return res;
}

static std::string ToLowerStr(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static std::wstring Utf8ToWString(const std::string& str)
{
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size);
    return wstr;
}

static CObject* GetCurrentlySelectedEditorObject()
{
    int tagIdx = CInspectorUI::GetInstance().GetSelectedTagIndex();
    int objIdx = CInspectorUI::GetInstance().GetSelectedObjectIndex();
    const auto& objList = ObjectManager::GetInstance().GetObjectList();
    if (tagIdx >= 0 && tagIdx < static_cast<int>(objList.size()))
    {
        const auto& vec = objList[tagIdx];
        if (objIdx >= 0 && objIdx < static_cast<int>(vec.size()))
        {
            CObject* obj = vec[objIdx].get();
            if (obj && !obj->GetIsDestroyed())
            {
                return obj;
            }
        }
    }
    return nullptr;
}

static void OpenPathInExplorer(const std::string& relPath, bool selectFile = false)
{
    // Debounce to prevent accidental rapid double-click / multi-spawn
    static ULONGLONG s_lastInvokeTick = 0;
    static std::string s_lastInvokePath = "";
    ULONGLONG nowTick = GetTickCount64();
    if (relPath == s_lastInvokePath && (nowTick - s_lastInvokeTick) < 600)
    {
        return;
    }
    s_lastInvokeTick = nowTick;
    s_lastInvokePath = relPath;

    std::error_code ec;
    fs::path absPath = fs::absolute(relPath, ec);
    if (ec || !fs::exists(absPath, ec)) return;

    std::wstring wTarget = absPath.wstring();
    std::replace(wTarget.begin(), wTarget.end(), L'/', L'\\');

    // SHOpenFolderAndSelectItems reuses an already-open Explorer window for that folder
    // instead of spawning a new Explorer window every time the button is pressed!
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (selectFile && fs::is_regular_file(absPath, ec))
    {
        fs::path parentDir = absPath.parent_path();
        std::wstring wFolder = parentDir.wstring();
        std::replace(wFolder.begin(), wFolder.end(), L'/', L'\\');

        PIDLIST_ABSOLUTE pidlFolder = ILCreateFromPathW(wFolder.c_str());
        PIDLIST_ABSOLUTE pidlFile = ILCreateFromPathW(wTarget.c_str());
        if (pidlFolder && pidlFile)
        {
            LPCITEMIDLIST pidlChild = ILFindLastID(pidlFile);
            SHOpenFolderAndSelectItems(pidlFolder, 1, &pidlChild, 0);
        }
        else
        {
            std::wstring param = L"/select,\"" + wTarget + L"\"";
            ShellExecuteW(NULL, L"open", L"explorer.exe", param.c_str(), NULL, SW_SHOWNORMAL);
        }
        if (pidlFile) ILFree(pidlFile);
        if (pidlFolder) ILFree(pidlFolder);
    }
    else
    {
        // Opening a directory: passing 0 items to SHOpenFolderAndSelectItems opens or focuses the existing folder window
        PIDLIST_ABSOLUTE pidlFolder = ILCreateFromPathW(wTarget.c_str());
        if (pidlFolder)
        {
            if (FAILED(SHOpenFolderAndSelectItems(pidlFolder, 0, nullptr, 0)))
            {
                ShellExecuteW(NULL, L"open", wTarget.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            ILFree(pidlFolder);
        }
        else
        {
            ShellExecuteW(NULL, L"open", wTarget.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }

    if (SUCCEEDED(hrCo))
    {
        CoUninitialize();
    }
}

AssetEntryType CContentDrawerUI::ClassifyAsset(const std::string& normalizedPath, bool isDirectory, const std::string& extLower)
{
    if (isDirectory) return AssetEntryType::Folder;

    std::string lowerPath = ToLowerStr(normalizedPath);

    if (extLower == ".hlsl" || extLower == ".hlsli" || extLower == ".cso")
        return AssetEntryType::Shader;

    if (extLower == ".mat")
        return AssetEntryType::Material;

    if (extLower == ".glb" || extLower == ".gltf" || extLower == ".fbx" || extLower == ".obj")
        return AssetEntryType::Model;

    if (extLower == ".png" || extLower == ".jpg" || extLower == ".jpeg" || extLower == ".dds" || extLower == ".tga" || extLower == ".bmp")
        return AssetEntryType::Texture;

    if (extLower == ".wav" || extLower == ".mp3" || extLower == ".ogg")
        return AssetEntryType::Audio;

    if (extLower == ".json")
    {
        if (lowerPath.find("assets/scene/") != std::string::npos)
            return AssetEntryType::Scene;
        if (lowerPath.find("assets/prefabs/") != std::string::npos)
            return AssetEntryType::Prefab;
        if (lowerPath.find("assets/pipeline/") != std::string::npos)
            return AssetEntryType::Pipeline;
    }

    return AssetEntryType::Other;
}

ImTextureID CContentDrawerUI::GetAssetIconTextureID(AssetEntryType type)
{
    int key = static_cast<int>(type);
    auto it = m_iconTextures.find(key);
    if (it != m_iconTextures.end())
    {
        if (it->second)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuH = it->second->GetGpuHandle();
            if (gpuH.ptr != 0)
            {
                return static_cast<ImTextureID>(gpuH.ptr);
            }
        }
        return 0;
    }

    const wchar_t* iconPath = L"Assets/Icons/icon_file.png";
    switch (type)
    {
    case AssetEntryType::Folder:   iconPath = L"Assets/Icons/icon_folder.png"; break;
    case AssetEntryType::Scene:    iconPath = L"Assets/Icons/icon_scene.png"; break;
    case AssetEntryType::Prefab:   iconPath = L"Assets/Icons/icon_prefab.png"; break;
    case AssetEntryType::Shader:   iconPath = L"Assets/Icons/icon_shader.png"; break;
    case AssetEntryType::Material: iconPath = L"Assets/Icons/icon_material.png"; break;
    case AssetEntryType::Model:    iconPath = L"Assets/Icons/icon_model.png"; break;
    case AssetEntryType::Texture:  iconPath = L"Assets/Icons/icon_texture.png"; break;
    case AssetEntryType::Pipeline: iconPath = L"Assets/Icons/icon_pipeline.png"; break;
    case AssetEntryType::Audio:    iconPath = L"Assets/Icons/icon_audio.png"; break;
    default:                       iconPath = L"Assets/Icons/icon_file.png"; break;
    }

    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
    ID3D12CommandQueue* queue = DX12Manager::GetInstance().GetCommandQueue();
    if (!device || !queue) return 0;

    // Dedicated one-shot command list ensures reliable texture upload independent of main render loop state
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> tempAlloc;
    HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tempAlloc));
    if (FAILED(hr)) return 0;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> tempCmdList;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, tempAlloc.Get(), nullptr, IID_PPV_ARGS(&tempCmdList));
    if (FAILED(hr)) return 0;

    auto newTexture = std::make_shared<CTexture>();
    if (newTexture->LoadTexture(device, tempCmdList.Get(), iconPath))
    {
        tempCmdList->Close();
        ID3D12CommandList* cmdLists[] = { tempCmdList.Get() };
        queue->ExecuteCommandLists(1, cmdLists);
        DX12Manager::GetInstance().WaitForPendingOperations();

        newTexture->CreateSRV(device);
        m_iconTextures[key] = newTexture;

        D3D12_GPU_DESCRIPTOR_HANDLE gpuH = newTexture->GetGpuHandle();
        return static_cast<ImTextureID>(gpuH.ptr);
    }
    else
    {
        tempCmdList->Close();
        m_iconTextures[key] = nullptr; // Cache null to avoid repeating failed disk reads
    }

    return 0;
}

static const char* GetAssetBadgeText(AssetEntryType type)
{
    switch (type)
    {
    case AssetEntryType::Folder:   return "[FOLDER]";
    case AssetEntryType::Scene:    return "[SCENE]";
    case AssetEntryType::Prefab:   return "[PREFAB]";
    case AssetEntryType::Shader:   return "[SHADER]";
    case AssetEntryType::Material: return "[MAT]";
    case AssetEntryType::Model:    return "[MODEL]";
    case AssetEntryType::Texture:  return "[TEX]";
    case AssetEntryType::Pipeline: return "[PIPELINE]";
    case AssetEntryType::Audio:    return "[AUDIO]";
    default:                       return "[FILE]";
    }
}

static ImVec4 GetAssetBadgeColor(AssetEntryType type)
{
    switch (type)
    {
    case AssetEntryType::Folder:   return ImVec4(0.95f, 0.78f, 0.25f, 1.0f); // Gold
    case AssetEntryType::Scene:    return ImVec4(0.35f, 0.92f, 0.55f, 1.0f); // Emerald Green
    case AssetEntryType::Prefab:   return ImVec4(0.30f, 0.80f, 1.00f, 1.0f); // Cyan
    case AssetEntryType::Shader:   return ImVec4(0.95f, 0.52f, 0.88f, 1.0f); // Magenta/Pink
    case AssetEntryType::Material: return ImVec4(1.00f, 0.60f, 0.25f, 1.0f); // Orange
    case AssetEntryType::Model:    return ImVec4(0.55f, 0.68f, 1.00f, 1.0f); // Periwinkle Blue
    case AssetEntryType::Texture:  return ImVec4(0.85f, 0.45f, 0.55f, 1.0f); // Coral Red
    case AssetEntryType::Pipeline: return ImVec4(0.65f, 0.88f, 0.88f, 1.0f); // Teal
    case AssetEntryType::Audio:    return ImVec4(0.85f, 0.85f, 0.40f, 1.0f); // Yellow
    default:                       return ImVec4(0.75f, 0.75f, 0.75f, 1.0f); // Gray
    }
}

void CContentDrawerUI::NavigateTo(const std::string& folderPath, bool recordHistory)
{
    std::string norm = NormalizePathSlash(folderPath);
    if (norm.empty()) norm = m_rootDirectory;

    std::error_code ec;
    if (!fs::exists(norm, ec) || !fs::is_directory(norm, ec))
    {
        return;
    }

    if (norm == m_currentDirectory)
    {
        RefreshCurrentDirectory();
        return;
    }

    if (recordHistory && !m_currentDirectory.empty())
    {
        m_backHistory.push_back(m_currentDirectory);
        m_forwardHistory.clear();
    }

    m_currentDirectory = norm;
    RefreshCurrentDirectory();
}

void CContentDrawerUI::RefreshCurrentDirectory()
{
    m_entries.clear();

    std::error_code ec;
    if (!fs::exists(m_currentDirectory, ec) || !fs::is_directory(m_currentDirectory, ec))
    {
        m_currentDirectory = m_rootDirectory;
    }

    bool isSearching = (m_searchFilter[0] != '\0') || (m_typeFilterIndex != 0);

    auto processDirEntry = [&](const fs::directory_entry& entry) {
        std::error_code itemEc;
        bool isDir = entry.is_directory(itemEc);
        bool isFile = entry.is_regular_file(itemEc);
        if (!isDir && !isFile) return;

        std::string normPath = NormalizePathSlash(entry.path().generic_string());
        std::string extLower = isDir ? "" : ToLowerStr(entry.path().extension().string());

        // Skip compiled shader object files (.cso) in source asset browser unless explicitly searched
        if (extLower == ".cso") return;

        AssetEntry item;
        item.path = normPath;
        item.fileName = entry.path().filename().string();
        item.name = isDir ? item.fileName : entry.path().stem().string();
        item.extension = extLower;
        item.type = ClassifyAsset(normPath, isDir, extLower);

        if (item.type == AssetEntryType::Prefab)
        {
            std::ifstream file(item.path);
            if (file.is_open())
            {
                try
                {
                    nlohmann::json j;
                    file >> j;
                    item.subInfo = "Tag: " + j.value("Tag", "NONE");
                }
                catch (...)
                {
                    item.subInfo = "Tag: NONE";
                }
            }
        }
        else if (item.type == AssetEntryType::Scene)
        {
            item.subInfo = "Scene JSON";
        }
        else if (item.type == AssetEntryType::Shader)
        {
            item.subInfo = (extLower == ".hlsli") ? "HLSL Header" : "HLSL Shader";
        }
        else if (item.type == AssetEntryType::Material)
        {
            item.subInfo = "Material (.mat)";
        }
        else if (item.type == AssetEntryType::Model)
        {
            item.subInfo = "3D Model (" + extLower + ")";
        }
        else if (item.type == AssetEntryType::Texture)
        {
            item.subInfo = "Texture (" + extLower + ")";
        }
        else if (item.type == AssetEntryType::Pipeline)
        {
            item.subInfo = "Render Pipeline";
        }
        else if (item.type == AssetEntryType::Folder)
        {
            int count = 0;
            for (const auto& sub : fs::directory_iterator(entry.path(), itemEc))
            {
                (void)sub;
                ++count;
            }
            item.subInfo = std::to_string(count) + " items";
        }
        else
        {
            item.subInfo = extLower;
        }

        m_entries.push_back(item);
    };

    if (isSearching)
    {
        for (const auto& entry : fs::recursive_directory_iterator(m_currentDirectory, ec))
        {
            processDirEntry(entry);
        }
    }
    else
    {
        for (const auto& entry : fs::directory_iterator(m_currentDirectory, ec))
        {
            processDirEntry(entry);
        }
    }

    // Sort: Folders first, then alphabetical by fileName
    std::sort(m_entries.begin(), m_entries.end(), [](const AssetEntry& a, const AssetEntry& b) {
        if ((a.type == AssetEntryType::Folder) != (b.type == AssetEntryType::Folder))
        {
            return a.type == AssetEntryType::Folder;
        }
        return ToLowerStr(a.fileName) < ToLowerStr(b.fileName);
    });

    m_isInitialized = true;
}

void CContentDrawerUI::HandleAssetDoubleClick(const AssetEntry& entry)
{
    switch (entry.type)
    {
    case AssetEntryType::Folder:
        m_searchFilter[0] = '\0';
        m_pendingNavigatePath = entry.path;
        break;

    case AssetEntryType::Scene:
        SceneManager::GetInstance().ChangeSceneByPath(entry.path, !CInspectorUI::GetInstance().IsEditMode(), 0.4f);
        HotReloadManager::GetInstance().CaptureSceneSnapshot();
        CInspectorUI::GetInstance().SetStatusMessage("Loaded Scene: " + entry.path, 3.0f);
        break;

    case AssetEntryType::Prefab:
        CInspectorUI::GetInstance().OpenPrefabEditMode(entry.path);
        break;

    case AssetEntryType::Shader:
    case AssetEntryType::Material:
    case AssetEntryType::Pipeline:
        ShaderManager::GetInstance().OpenInVSCode(entry.path, 1);
        break;

    case AssetEntryType::Model:
    {
        // Spawn 3D Model in front of camera
        auto sharedModel = ModelManager::GetInstance().GetModel(entry.path);
        if (sharedModel)
        {
            static int modelSpawnCounter = 0;
            std::string instName = entry.name + "_" + std::to_string(++modelSpawnCounter);
            CObject* newObj = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::NONE, "CObject", instName);
            if (newObj)
            {
                CModel* m = newObj->GetComponent<CModel>();
                if (!m) m = newObj->AddComponent<CModel>();
                if (m)
                {
                    m->CopyFrom(sharedModel);
                    m->SetModelPath(entry.path);
                }
                newObj->Init();
                newObj->Awake();
                CInspectorUI::GetInstance().SetStatusMessage("Spawned 3D Model: " + instName, 3.0f);
            }
        }
        break;
    }

    default:
        OpenPathInExplorer(entry.path, true);
        break;
    }
}

void CContentDrawerUI::DrawTopNavigationBar()
{
    // 1. Back / Forward / Up Buttons
    bool canBack = !m_backHistory.empty();
    if (!canBack) ImGui::BeginDisabled();
    if (ImGui::Button(" < "))
    {
        std::string prev = m_backHistory.back();
        m_backHistory.pop_back();
        m_forwardHistory.push_back(m_currentDirectory);
        m_currentDirectory = prev;
        RefreshCurrentDirectory();
    }
    if (!canBack) ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Back");

    ImGui::SameLine();
    bool canForward = !m_forwardHistory.empty();
    if (!canForward) ImGui::BeginDisabled();
    if (ImGui::Button(" > "))
    {
        std::string next = m_forwardHistory.back();
        m_forwardHistory.pop_back();
        m_backHistory.push_back(m_currentDirectory);
        m_currentDirectory = next;
        RefreshCurrentDirectory();
    }
    if (!canForward) ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Forward");

    ImGui::SameLine();
    bool canUp = (m_currentDirectory != m_rootDirectory && m_currentDirectory.find('/') != std::string::npos);
    if (!canUp) ImGui::BeginDisabled();
    if (ImGui::Button("Up"))
    {
        fs::path p(m_currentDirectory);
        std::string parentStr = NormalizePathSlash(p.parent_path().generic_string());
        if (parentStr.empty()) parentStr = m_rootDirectory;
        NavigateTo(parentStr, true);
    }
    if (!canUp) ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Refresh"))
    {
        RefreshCurrentDirectory();
    }

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    // 2. Breadcrumb Path Bar (Assets > Shader > ...)
    {
        std::vector<std::pair<std::string, std::string>> crumbs;
        std::string accum = "";
        std::stringstream ss(m_currentDirectory);
        std::string segment;
        while (std::getline(ss, segment, '/'))
        {
            if (segment.empty()) continue;
            accum = accum.empty() ? segment : (accum + "/" + segment);
            crumbs.push_back({ segment, accum });
        }

        for (size_t i = 0; i < crumbs.size(); ++i)
        {
            ImGui::PushID(static_cast<int>(i));
            bool isLast = (i + 1 == crumbs.size());
            if (isLast)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.45f, 0.68f, 1.0f));
            }
            if (ImGui::SmallButton(crumbs[i].first.c_str()))
            {
                m_searchFilter[0] = '\0';
                NavigateTo(crumbs[i].second, true);
            }
            if (isLast)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();

            if (!isLast)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("/");
                ImGui::SameLine();
            }
        }
    }

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    // 3. Search Filter & Type Filter
    ImGui::SetNextItemWidth(160.0f);
    if (ImGui::InputTextWithHint("##AssetSearch", "Search assets...", m_searchFilter, sizeof(m_searchFilter)))
    {
        RefreshCurrentDirectory();
    }
    if (m_searchFilter[0] != '\0')
    {
        ImGui::SameLine();
        if (ImGui::SmallButton("X"))
        {
            m_searchFilter[0] = '\0';
            RefreshCurrentDirectory();
        }
    }

    ImGui::SameLine();
    static const char* typeFilterNames[] = {
        "All Types",
        "Folders",
        "Scenes (.json)",
        "Prefabs (.json)",
        "Shaders (.hlsl)",
        "Materials (.mat)",
        "Models (.glb/.fbx)",
        "Textures (.png/.dds)",
        "Pipelines (.json)",
        "Audio (.wav)"
    };
    ImGui::SetNextItemWidth(135.0f);
    if (ImGui::Combo("##TypeFilter", &m_typeFilterIndex, typeFilterNames, IM_ARRAYSIZE(typeFilterNames)))
    {
        RefreshCurrentDirectory();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(85.0f);
    ImGui::SliderFloat("Size", &m_cardWidth, 75.0f, 160.0f, "%.0f");

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.55f, 0.28f, 1.0f));
    if (ImGui::Button("+ Create"))
    {
        ImGui::OpenPopup("CreateAssetInDrawerPopup");
    }
    ImGui::PopStyleColor();

    if (ImGui::BeginPopup("CreateAssetInDrawerPopup"))
    {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.6f, 1.0f), "Create in: %s", m_currentDirectory.c_str());
        ImGui::Separator();

        static char newAssetNameBuf[128] = "NewAsset";
        ImGui::InputText("Name", newAssetNameBuf, sizeof(newAssetNameBuf));

        if (ImGui::Selectable("?? New Subfolder"))
        {
            std::error_code ec;
            fs::path newFolder = fs::path(m_currentDirectory) / newAssetNameBuf;
            if (fs::create_directories(newFolder, ec))
            {
                RefreshCurrentDirectory();
                CInspectorUI::GetInstance().SetStatusMessage("Created folder: " + newFolder.generic_string(), 3.0f);
            }
        }
        if (ImGui::Selectable("?? New 3D Test Scene (.json)"))
        {
            std::string createdPath;
            if (SceneManager::GetInstance().CreateNewScene(newAssetNameBuf, SceneTemplateType::Standard3DTest, &createdPath))
            {
                NavigateTo("Assets/Scene", true);
                CInspectorUI::GetInstance().SetStatusMessage("Created scene: " + createdPath, 3.0f);
            }
        }
        if (ImGui::Selectable("?? New Shadow Showcase Scene (.json)"))
        {
            std::string createdPath;
            if (SceneManager::GetInstance().CreateNewScene(newAssetNameBuf, SceneTemplateType::ShadowShowcase, &createdPath))
            {
                NavigateTo("Assets/Scene", true);
                CInspectorUI::GetInstance().SetStatusMessage("Created shadow scene: " + createdPath, 3.0f);
            }
        }
        if (ImGui::Selectable("? New Lit Shadow Shader (.hlsl)"))
        {
            std::string shaderPath = "Assets/Shader/" + std::string(newAssetNameBuf) + ".hlsl";
            if (ShaderManager::GetInstance().CreateShaderFromTemplate(shaderPath, ShaderTemplateType::LitShadowMesh))
            {
                NavigateTo("Assets/Shader", true);
                ShaderManager::GetInstance().OpenInVSCode(shaderPath, 1);
                CInspectorUI::GetInstance().SetStatusMessage("Created shader: " + shaderPath, 3.0f);
            }
        }
        if (ImGui::Selectable("?? New Material (.mat)"))
        {
            std::error_code ec;
            fs::create_directories("Assets/Materials", ec);
            std::string matPath = "Assets/Materials/" + std::string(newAssetNameBuf) + ".mat";
            auto newMat = std::make_shared<CMaterial>();
            newMat->SetShader(L"Assets/Shader/StandardShadow.hlsl", "VSMain", "PSMain");
            if (newMat->SaveToFile(matPath))
            {
                NavigateTo("Assets/Materials", true);
                CInspectorUI::GetInstance().SetStatusMessage("Created material: " + matPath, 3.0f);
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Explorer"))
    {
        OpenPathInExplorer(m_currentDirectory, false);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Open '%s' in Windows Explorer", m_currentDirectory.c_str());
    }
}

void CContentDrawerUI::DrawFolderTreeRecursive(const std::string& folderPath)
{
    std::string normPath = NormalizePathSlash(folderPath);
    std::string folderName = (normPath == m_rootDirectory) ? "Assets (Root)" : fs::path(normPath).filename().string();

    // Check if this folder has any subdirectories
    bool hasSubDirs = false;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(normPath, ec))
    {
        if (entry.is_directory(ec))
        {
            hasSubDirs = true;
            break;
        }
    }

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (normPath == m_rootDirectory)
    {
        nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
    }
    if (!hasSubDirs)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (normPath == m_currentDirectory)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }
    // Auto-open ancestors of m_currentDirectory
    if (m_currentDirectory.rfind(normPath + "/", 0) == 0)
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    }

    std::string label = "[Dir] " + folderName + "##" + normPath;
    bool opened = ImGui::TreeNodeEx(label.c_str(), nodeFlags);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        m_searchFilter[0] = '\0';
        NavigateTo(normPath, true);
    }

    if (opened)
    {
        std::vector<std::string> subFolders;
        for (const auto& entry : fs::directory_iterator(normPath, ec))
        {
            if (entry.is_directory(ec))
            {
                subFolders.push_back(NormalizePathSlash(entry.path().generic_string()));
            }
        }
        std::sort(subFolders.begin(), subFolders.end());

        for (const auto& sub : subFolders)
        {
            DrawFolderTreeRecursive(sub);
        }
        ImGui::TreePop();
    }
}

void CContentDrawerUI::DrawAssetGrid()
{
    m_isAnyCardHovered = false;

    // Global/Window Shortcuts: Undo (Ctrl+Z) and Redo (Ctrl+Y / Ctrl+Shift+Z)
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
    {
        if (ImGui::GetIO().KeyShift)
        {
            if (UndoManager::GetInstance().CanRedo())
            {
                std::string name = UndoManager::GetInstance().GetRedoName();
                UndoManager::GetInstance().Redo();
                CInspectorUI::GetInstance().SetStatusMessage("Redo: " + name, 2.5f);
                m_pendingRefresh = true;
            }
        }
        else
        {
            if (UndoManager::GetInstance().CanUndo())
            {
                std::string name = UndoManager::GetInstance().GetUndoName();
                UndoManager::GetInstance().Undo();
                CInspectorUI::GetInstance().SetStatusMessage("Undo: " + name, 2.5f);
                m_pendingRefresh = true;
            }
        }
    }
    else if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false))
    {
        if (UndoManager::GetInstance().CanRedo())
        {
            std::string name = UndoManager::GetInstance().GetRedoName();
            UndoManager::GetInstance().Redo();
            CInspectorUI::GetInstance().SetStatusMessage("Redo: " + name, 2.5f);
            m_pendingRefresh = true;
        }
    }

    // F2 Shortcut for renaming currently selected asset
    if (!m_selectedAssetPath.empty() && m_renamingAssetPath.empty() && ImGui::IsKeyPressed(ImGuiKey_F2, false))
    {
        for (const auto& e : m_entries)
        {
            if (e.path == m_selectedAssetPath)
            {
                m_renamingAssetPath = e.path;
                std::string initialName = (e.type == AssetEntryType::Folder) ? e.fileName : e.name;
                strncpy_s(m_renameInputBuf, sizeof(m_renameInputBuf), initialName.c_str(), _TRUNCATE);
                m_renameInputFocusNeeded = true;
                break;
            }
        }
    }

    // Delete Shortcut for currently selected asset
    if (!m_selectedAssetPath.empty() && m_renamingAssetPath.empty() && ImGui::IsKeyPressed(ImGuiKey_Delete, false))
    {
        m_activeDeletePlan = AssetDeleteService::AnalyzeDelete(m_selectedAssetPath);
        if (!m_activeDeletePlan.isValid)
        {
            CInspectorUI::GetInstance().SetStatusMessage("Cannot delete: " + m_activeDeletePlan.errorMessage, 4.0f);
        }
        else
        {
            m_showDeleteConfirmPopup = true;
        }
    }

    float availWidth = ImGui::GetContentRegionAvail().x;
    int columns = static_cast<int>(availWidth / (m_cardWidth + 10.0f));
    if (columns < 1) columns = 1;

    std::string searchLower = ToLowerStr(m_searchFilter);

    auto MatchesTypeFilter = [&](AssetEntryType t) -> bool {
        switch (m_typeFilterIndex)
        {
        case 0: return true;
        case 1: return t == AssetEntryType::Folder;
        case 2: return t == AssetEntryType::Scene;
        case 3: return t == AssetEntryType::Prefab;
        case 4: return t == AssetEntryType::Shader;
        case 5: return t == AssetEntryType::Material;
        case 6: return t == AssetEntryType::Model;
        case 7: return t == AssetEntryType::Texture;
        case 8: return t == AssetEntryType::Pipeline;
        case 9: return t == AssetEntryType::Audio;
        default: return true;
        }
    };

    if (ImGui::BeginTable("AssetBrowserGrid", columns, ImGuiTableFlags_PadOuterX))
    {
        int visibleCount = 0;

        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            const AssetEntry item = m_entries[i];

            if (!MatchesTypeFilter(item.type)) continue;

            if (!searchLower.empty())
            {
                std::string nameLower = ToLowerStr(item.fileName);
                std::string subLower = ToLowerStr(item.subInfo);
                std::string pathLower = ToLowerStr(item.path);
                if (nameLower.find(searchLower) == std::string::npos &&
                    subLower.find(searchLower) == std::string::npos &&
                    pathLower.find(searchLower) == std::string::npos)
                {
                    continue;
                }
            }

            ++visibleCount;
            ImGui::TableNextColumn();
            ImGui::PushID(item.path.c_str());

            float iconSize = std::clamp(m_cardWidth * 0.55f, 32.0f, 72.0f);
            float cardHeight = iconSize + 36.0f;

            bool isSelected = (m_selectedAssetPath == item.path);
            ImVec4 cardBg = isSelected
                ? ImVec4(0.18f, 0.38f, 0.65f, 0.95f)
                : ImVec4(0.14f, 0.15f, 0.18f, 0.70f);
            ImVec4 cardBorder = isSelected
                ? ImVec4(0.35f, 0.82f, 1.00f, 1.00f)
                : ImVec4(0.24f, 0.26f, 0.30f, 0.60f);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
            ImGui::PushStyleColor(ImGuiCol_Border, cardBorder);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, isSelected ? 2.0f : 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::BeginChild("AssetCard", ImVec2(m_cardWidth, cardHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            // Detect click / double-click / right-click on the card
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
            {
                m_isAnyCardHovered = true;
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    m_selectedAssetPath = item.path;
                }
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    m_selectedAssetPath = item.path;
                    ImGui::OpenPopup("AssetContextMenu");
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemActive())
                {
                    m_selectedAssetPath = item.path;
                    HandleAssetDoubleClick(item);
                }
            }

            // Right-click Context Menu
            if (ImGui::BeginPopup("AssetContextMenu"))
            {
                ImGui::TextColored(GetAssetBadgeColor(item.type), "[%s]", GetAssetBadgeText(item.type));
                ImGui::SameLine();
                ImGui::TextUnformatted(item.fileName.c_str());
                if (!item.subInfo.empty())
                {
                    ImGui::TextDisabled("%s", item.subInfo.c_str());
                }
                ImGui::Separator();

                switch (item.type)
                {
                case AssetEntryType::Folder:
                    if (ImGui::MenuItem("Open Folder"))
                    {
                        m_searchFilter[0] = '\0';
                        m_pendingNavigatePath = item.path;
                    }
                    break;

                case AssetEntryType::Scene:
                {
                    bool isCurrentScene = (SceneManager::GetInstance().GetActiveScenePath() == item.path);
                    if (ImGui::MenuItem(isCurrentScene ? "Reload Scene" : "Load Scene"))
                    {
                        SceneManager::GetInstance().ChangeSceneByPath(item.path, !CInspectorUI::GetInstance().IsEditMode(), 0.4f);
                        HotReloadManager::GetInstance().CaptureSceneSnapshot();
                        CInspectorUI::GetInstance().SetStatusMessage("Loaded Scene: " + item.path, 3.0f);
                    }
                    if (ImGui::MenuItem("Load Scene (Additive)"))
                    {
                        SceneManager::GetInstance().LoadSceneAdditiveByPath(item.path, true);
                        CInspectorUI::GetInstance().SetStatusMessage("Loaded Additive: " + item.path, 3.0f);
                    }
                    if (ImGui::MenuItem("Duplicate Scene"))
                    {
                        std::string dupPath;
                        std::string newName = item.name + "_Copy";
                        if (SceneManager::GetInstance().DuplicateScene(item.path, newName, &dupPath))
                        {
                            m_pendingRefresh = true;
                            CInspectorUI::GetInstance().SetStatusMessage("Duplicated scene: " + dupPath, 3.0f);
                        }
                    }
                    if (ImGui::MenuItem("Open in VS Code"))
                    {
                        ShaderManager::GetInstance().OpenInVSCode(item.path, 1);
                    }
                    break;
                }

                case AssetEntryType::Prefab:
                    if (ImGui::MenuItem("Edit Prefab"))
                    {
                        CInspectorUI::GetInstance().OpenPrefabEditMode(item.path);
                    }
                    if (ImGui::MenuItem("Spawn in Front of Camera"))
                    {
                        CObject* cam = ObjectManager::GetInstance().GetCameraObject();
                        DirectX::XMFLOAT3 spawnPos = { 0.0f, 0.0f, 0.0f };
                        if (cam)
                        {
                            if (CTransform* camTransform = cam->GetComponent<CTransform>())
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
                            if (CTransform* t = newObj->GetComponent<CTransform>()) t->SetPos(spawnPos);
                            CObjectInfo* info = newObj->GetComponent<CObjectInfo>();
                            ObjectTag tag = info ? info->GetObjectTag() : ObjectTag::NONE;
                            ObjectManager::GetInstance().AddObject(tag, newObj);
                            newObj->Init();
                            newObj->Awake();
                            CInspectorUI::GetInstance().SetStatusMessage("Spawned Prefab: " + instanceName, 3.0f);
                        }
                    }
                    if (ImGui::MenuItem("Spawn at Origin (0,0,0)"))
                    {
                        static int spawnCounter = 0;
                        std::string instanceName = item.name + "_" + std::to_string(++spawnCounter);
                        CObject* newObj = PrefabManager::GetInstance().InstantiateFromJSON(item.path, instanceName);
                        if (newObj)
                        {
                            CObjectInfo* info = newObj->GetComponent<CObjectInfo>();
                            ObjectTag tag = info ? info->GetObjectTag() : ObjectTag::NONE;
                            ObjectManager::GetInstance().AddObject(tag, newObj);
                            newObj->Init();
                            newObj->Awake();
                            CInspectorUI::GetInstance().SetStatusMessage("Spawned Prefab at Origin: " + instanceName, 3.0f);
                        }
                    }
                    break;

                case AssetEntryType::Shader:
                    if (ImGui::MenuItem("Open in VS Code"))
                    {
                        ShaderManager::GetInstance().OpenInVSCode(item.path, 1);
                    }
                    if (item.extension == ".hlsl")
                    {
                        if (ImGui::MenuItem("Apply to Selected 3D Model"))
                        {
                            CObject* selObj = GetCurrentlySelectedEditorObject();
                            CModel* model = selObj ? selObj->GetComponent<CModel>() : nullptr;
                            if (model && !model->GetMaterials().empty())
                            {
                                int slot = model->GetSelectedMaterialIndex();
                                std::wstring wShader = Utf8ToWString(item.path);
                                if (slot >= 0 && slot < static_cast<int>(model->GetMaterials().size()) && model->GetMaterial(slot))
                                {
                                    model->GetMaterial(slot)->SetShader(wShader, "VSMain", "PSMain");
                                }
                                else
                                {
                                    for (auto& mat : model->GetMaterials())
                                    {
                                        if (mat) mat->SetShader(wShader, "VSMain", "PSMain");
                                    }
                                }
                                CInspectorUI::GetInstance().SetStatusMessage("Applied shader to " + selObj->GetName() + ": " + item.fileName, 3.0f);
                            }
                            else
                            {
                                CInspectorUI::GetInstance().SetStatusMessage("Select a 3D Model object first to apply shader.", 3.0f);
                            }
                        }
                    }
                    break;

                case AssetEntryType::Material:
                    if (ImGui::MenuItem("Apply to Selected 3D Model"))
                    {
                        CObject* selObj = GetCurrentlySelectedEditorObject();
                        CModel* model = selObj ? selObj->GetComponent<CModel>() : nullptr;
                        if (model && !model->GetMaterials().empty())
                        {
                            auto matInst = MaterialManager::GetInstance().CreateInstance(item.path);
                            if (matInst)
                            {
                                int slot = model->GetSelectedMaterialIndex();
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
                                CInspectorUI::GetInstance().SetStatusMessage("Applied material to " + selObj->GetName() + ": " + item.name, 3.0f);
                            }
                        }
                        else
                        {
                            CInspectorUI::GetInstance().SetStatusMessage("Select a 3D Model object first to apply .mat.", 3.0f);
                        }
                    }
                    if (ImGui::MenuItem("Open in VS Code"))
                    {
                        ShaderManager::GetInstance().OpenInVSCode(item.path, 1);
                    }
                    break;

                case AssetEntryType::Model:
                    if (ImGui::MenuItem("Spawn Model in Scene"))
                    {
                        HandleAssetDoubleClick(item);
                    }
                    if (ImGui::MenuItem("Assign to Selected Object"))
                    {
                        CObject* selObj = GetCurrentlySelectedEditorObject();
                        if (selObj)
                        {
                            auto sharedModel = ModelManager::GetInstance().GetModel(item.path);
                            if (sharedModel)
                            {
                                CModel* m = selObj->GetComponent<CModel>();
                                if (!m) m = selObj->AddComponent<CModel>();
                                if (m)
                                {
                                    m->CopyFrom(sharedModel);
                                    m->SetModelPath(item.path);
                                    CInspectorUI::GetInstance().SetStatusMessage("Assigned model to " + selObj->GetName() + ": " + item.fileName, 3.0f);
                                }
                            }
                        }
                        else
                        {
                            CInspectorUI::GetInstance().SetStatusMessage("Select an object in Hierarchy first.", 3.0f);
                        }
                    }
                    break;

                case AssetEntryType::Texture:
                    if (ImGui::MenuItem("Apply to Selected (Sprite/Model)"))
                    {
                        CObject* selObj = GetCurrentlySelectedEditorObject();
                        std::wstring wTexPath = Utf8ToWString(item.path);
                        if (selObj)
                        {
                            if (CSpriteRenderer* spr = selObj->GetComponent<CSpriteRenderer>())
                            {
                                spr->SetTexture(wTexPath);
                                CInspectorUI::GetInstance().SetStatusMessage("Applied texture to Sprite: " + item.fileName, 3.0f);
                            }
                            else if (CModel* model = selObj->GetComponent<CModel>())
                            {
                                int slot = model->GetSelectedMaterialIndex();
                                if (slot < 0) slot = 0;
                                if (auto mat = model->GetMaterial(static_cast<UINT>(slot)))
                                {
                                    mat->LoadTexture(wTexPath);
                                    CInspectorUI::GetInstance().SetStatusMessage("Applied texture to Material[" + std::to_string(slot) + "]: " + item.fileName, 3.0f);
                                }
                            }
                            else
                            {
                                CInspectorUI::GetInstance().SetStatusMessage("Selected object has no CModel or CSpriteRenderer.", 3.0f);
                            }
                        }
                        else
                        {
                            CInspectorUI::GetInstance().SetStatusMessage("Select a Model or Sprite object first.", 3.0f);
                        }
                    }
                    break;

                case AssetEntryType::Pipeline:
                    if (ImGui::MenuItem("Load Pipeline"))
                    {
                        if (CScene* activeScene = SceneManager::GetInstance().GetActiveScene())
                        {
                            if (RenderPipeline* pipeline = activeScene->GetRenderPipeline())
                            {
                                DX12Manager::GetInstance().WaitForPendingOperations();
                                if (pipeline->LoadFromJson(item.path, DX12Manager::GetInstance().GetDevice(), activeScene->GetSceneTexture()))
                                {
                                    CInspectorUI::GetInstance().SetStatusMessage("Loaded RenderPipeline: " + item.path, 3.0f);
                                }
                            }
                        }
                    }
                    if (ImGui::MenuItem("Open in VS Code"))
                    {
                        ShaderManager::GetInstance().OpenInVSCode(item.path, 1);
                    }
                    break;

                default:
                    break;
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Rename", "F2"))
                {
                    m_renamingAssetPath = item.path;
                    std::string initialName = (item.type == AssetEntryType::Folder) ? item.fileName : item.name;
                    strncpy_s(m_renameInputBuf, sizeof(m_renameInputBuf), initialName.c_str(), _TRUNCATE);
                    m_renameInputFocusNeeded = true;
                }
                if (ImGui::MenuItem("Delete", "Del"))
                {
                    m_activeDeletePlan = AssetDeleteService::AnalyzeDelete(item.path);
                    if (!m_activeDeletePlan.isValid)
                    {
                        CInspectorUI::GetInstance().SetStatusMessage("Cannot delete: " + m_activeDeletePlan.errorMessage, 4.0f);
                    }
                    else
                    {
                        m_showDeleteConfirmPopup = true;
                    }
                }
                if (ImGui::MenuItem("Show in Explorer"))
                {
                    OpenPathInExplorer(item.path, item.type != AssetEntryType::Folder);
                }
                if (ImGui::MenuItem("Copy Relative Path"))
                {
                    ImGui::SetClipboardText(item.path.c_str());
                    CInspectorUI::GetInstance().SetStatusMessage("Copied: " + item.path, 2.0f);
                }
                if (ImGui::MenuItem("Copy File Name"))
                {
                    ImGui::SetClipboardText(item.fileName.c_str());
                    CInspectorUI::GetInstance().SetStatusMessage("Copied: " + item.fileName, 2.0f);
                }

                ImGui::EndPopup();
            }

            // Top padding
            ImGui::Dummy(ImVec2(0.0f, 1.0f));

            // 1. Icon Image Centered on top
            ImTextureID iconTexID = GetAssetIconTextureID(item.type);
            float iconX = (m_cardWidth - iconSize) * 0.5f;
            if (iconX > 0.0f) ImGui::SetCursorPosX(iconX);
            if (iconTexID != 0)
            {
                ImGui::Image(iconTexID, ImVec2(iconSize, iconSize));
            }
            else
            {
                ImGui::Dummy(ImVec2(iconSize, iconSize));
            }

            // 2. File / Asset Name Centered underneath (or inline rename input)
            ImGui::Spacing();
            if (m_renamingAssetPath == item.path)
            {
                ImGui::PushItemWidth(m_cardWidth - 8.0f);
                if (m_renameInputFocusNeeded)
                {
                    ImGui::SetKeyboardFocusHere();
                    m_renameInputFocusNeeded = false;
                }
                bool enterPressed = ImGui::InputText("##InlineRename", m_renameInputBuf, sizeof(m_renameInputBuf),
                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
                ImGui::PopItemWidth();

                if (enterPressed)
                {
                    std::string requestedName = m_renameInputBuf;
                    m_renamingAssetPath.clear();
                    m_activeRenamePlan = AssetRenameService::AnalyzeRename(item.path, requestedName);
                    if (!m_activeRenamePlan.isValid)
                    {
                        CInspectorUI::GetInstance().SetStatusMessage("Rename failed: " + m_activeRenamePlan.errorMessage, 4.0f);
                    }
                    else if (m_activeRenamePlan.hits.empty())
                    {
                        std::string err;
                        if (AssetRenameService::ExecuteRename(m_activeRenamePlan, true, &err))
                        {
                            CInspectorUI::GetInstance().SetStatusMessage("Renamed to: " + m_activeRenamePlan.newFileName, 3.0f);
                            m_selectedAssetPath = m_activeRenamePlan.newPath;
                            m_pendingRefresh = true;
                        }
                        else
                        {
                            CInspectorUI::GetInstance().SetStatusMessage("Rename error: " + err, 4.0f);
                        }
                    }
                    else
                    {
                        m_showRenameConfirmPopup = true;
                    }
                }
                else if (ImGui::IsKeyPressed(ImGuiKey_Escape) || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()))
                {
                    m_renamingAssetPath.clear();
                }
            }
            else
            {
                std::string dispName = item.fileName;
                float maxTextWidth = m_cardWidth - 8.0f;
                if (ImGui::CalcTextSize(dispName.c_str()).x > maxTextWidth)
                {
                    while (dispName.length() > 3 && ImGui::CalcTextSize((dispName + "...").c_str()).x > maxTextWidth)
                    {
                        dispName.pop_back();
                    }
                    dispName += "...";
                }
                ImVec2 textSize = ImGui::CalcTextSize(dispName.c_str());
                float textX = (m_cardWidth - textSize.x) * 0.5f;
                if (textX > 4.0f) ImGui::SetCursorPosX(textX);

                if (isSelected)
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.45f, 1.0f), "%s", dispName.c_str());
                }
                else
                {
                    ImGui::TextUnformatted(dispName.c_str());
                }
            }

            // Tooltip with full path, type and double-click/right-click hints
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(GetAssetBadgeColor(item.type), "[%s] %s", GetAssetBadgeText(item.type), item.fileName.c_str());
                ImGui::TextDisabled("Path: %s", item.path.c_str());
                if (!item.subInfo.empty())
                {
                    ImGui::TextDisabled("Info: %s", item.subInfo.c_str());
                }
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Left-click: Select\nDouble-click: Open\nRight-click: Context Menu");
                ImGui::EndTooltip();
            }

            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::PopID();
        }

        ImGui::EndTable();

        if (visibleCount == 0)
        {
            ImGui::TextDisabled("No matching assets found in '%s'.", m_currentDirectory.c_str());
        }
    }

    // Confirmation Modal Popup for Renaming with references
    if (m_showRenameConfirmPopup)
    {
        ImGui::OpenPopup("Confirm Asset Rename");
        m_showRenameConfirmPopup = false;
    }
    if (ImGui::BeginPopupModal("Confirm Asset Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Update references for '%s' -> '%s'?",
            m_activeRenamePlan.oldFileName.c_str(), m_activeRenamePlan.newFileName.c_str());
        ImGui::Separator();
        ImGui::Text("Found %d reference(s) across %d file(s):",
            m_activeRenamePlan.totalOccurrences, static_cast<int>(m_activeRenamePlan.hits.size()));

        ImGui::BeginChild("ReferenceHitList", ImVec2(480.0f, 140.0f), true);
        for (const auto& hit : m_activeRenamePlan.hits)
        {
            ImGui::BulletText("%s (%d occurrence%s)", hit.filePath.c_str(), hit.occurrences, hit.occurrences > 1 ? "s" : "");
        }
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
        if (ImGui::Button("Update References & Rename", ImVec2(220.0f, 28.0f)))
        {
            std::string err;
            if (AssetRenameService::ExecuteRename(m_activeRenamePlan, true, &err))
            {
                CInspectorUI::GetInstance().SetStatusMessage("Renamed and updated references: " + m_activeRenamePlan.newFileName, 3.0f);
                m_selectedAssetPath = m_activeRenamePlan.newPath;
                m_pendingRefresh = true;
            }
            else
            {
                CInspectorUI::GetInstance().SetStatusMessage("Rename error: " + err, 4.0f);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.45f, 0.15f, 1.0f));
        if (ImGui::Button("Rename Without Updating", ImVec2(190.0f, 28.0f)))
        {
            std::string err;
            if (AssetRenameService::ExecuteRename(m_activeRenamePlan, false, &err))
            {
                CInspectorUI::GetInstance().SetStatusMessage("Renamed (references kept): " + m_activeRenamePlan.newFileName, 3.0f);
                m_selectedAssetPath = m_activeRenamePlan.newPath;
                m_pendingRefresh = true;
            }
            else
            {
                CInspectorUI::GetInstance().SetStatusMessage("Rename error: " + err, 4.0f);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80.0f, 28.0f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Confirmation Modal Popup for Deletion
    if (m_showDeleteConfirmPopup)
    {
        ImGui::OpenPopup("Confirm Asset Deletion");
        m_showDeleteConfirmPopup = false;
    }
    if (ImGui::BeginPopupModal("Confirm Asset Deletion", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Delete '%s'?", m_activeDeletePlan.fileName.c_str());
        ImGui::Separator();

        if (m_activeDeletePlan.totalOccurrences > 0)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f),
                "WARNING: Found %d reference(s) across %d file(s)!",
                m_activeDeletePlan.totalOccurrences, static_cast<int>(m_activeDeletePlan.hits.size()));
            ImGui::TextDisabled("Deleting this asset will cause missing references in other files:");

            ImGui::BeginChild("DeleteReferenceHitList", ImVec2(480.0f, 120.0f), true);
            for (const auto& hit : m_activeDeletePlan.hits)
            {
                ImGui::BulletText("%s (%d occurrence%s)", hit.filePath.c_str(), hit.occurrences, hit.occurrences > 1 ? "s" : "");
            }
            ImGui::EndChild();
        }
        else
        {
            ImGui::Text("Are you sure you want to delete this asset? (Can be undone with Ctrl+Z)");
        }

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("Delete", ImVec2(120.0f, 28.0f)))
        {
            std::string err;
            if (AssetDeleteService::ExecuteDelete(m_activeDeletePlan, &err))
            {
                CInspectorUI::GetInstance().SetStatusMessage("Deleted: " + m_activeDeletePlan.fileName + " (Ctrl+Z to undo)", 3.5f);
                m_selectedAssetPath.clear();
                m_pendingRefresh = true;
            }
            else
            {
                CInspectorUI::GetInstance().SetStatusMessage("Delete failed: " + err, 4.0f);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 28.0f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Apply deferred folder navigation or refresh safely after m_entries iteration finishes
    if (!m_pendingNavigatePath.empty())
    {
        std::string target = m_pendingNavigatePath;
        m_pendingNavigatePath.clear();
        m_pendingRefresh = false;
        NavigateTo(target, true);
    }
    else if (m_pendingRefresh)
    {
        m_pendingRefresh = false;
        RefreshCurrentDirectory();
    }
}

void CContentDrawerUI::Draw()
{
#ifndef _DEBUG
    return;
#endif // !_DEBUG

    // Shortcut Key (Ctrl + Space) to toggle visibility
    if (CInputManager::GetInstance().IsKeyPress(VK_CONTROL) && CInputManager::GetInstance().IsKeyTrigger(VK_SPACE))
    {
        ToggleVisible();
    }

    if (!m_isVisible) return;

    if (!m_isInitialized)
    {
        RefreshCurrentDirectory();
    }

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;
    float hierarchyW = 320.0f;
    float inspectorW = 460.0f;
    float centerW = screenW - hierarchyW - inspectorW;
    float drawerH = 295.0f;

    ImGui::SetNextWindowPos(ImVec2(hierarchyW, screenH - drawerH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(centerW, drawerH), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (ImGui::Begin("Content Drawer (Asset Browser)", &m_isVisible, flags))
    {
        // Top Navigation Bar (Back/Forward/Up, Breadcrumbs, Search, Type Filter, +Create)
        DrawTopNavigationBar();

        // Selected Asset Info Bar
        if (!m_selectedAssetPath.empty())
        {
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "Selected: %s", m_selectedAssetPath.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Copy Path"))
            {
                ImGui::SetClipboardText(m_selectedAssetPath.c_str());
                CInspectorUI::GetInstance().SetStatusMessage("Copied path: " + m_selectedAssetPath, 2.0f);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Show in Explorer"))
            {
                OpenPathInExplorer(m_selectedAssetPath, true);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Clear Selection"))
            {
                m_selectedAssetPath.clear();
            }
        }
        else
        {
            ImGui::TextDisabled("Selected: (None - Click any asset card to select)");
        }
        ImGui::Separator();

        // 2-Pane Split Layout: Left = Folder Hierarchy Tree, Right = Asset Grid
        if (ImGui::BeginTable("ContentDrawerSplitTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Folders", ImGuiTableColumnFlags_WidthFixed, 195.0f);
            ImGui::TableSetupColumn("Assets", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();

            // Left Pane: Folder Tree
            ImGui::TableSetColumnIndex(0);
            ImGui::BeginChild("FolderTreePane", ImVec2(0, 0), false);
            DrawFolderTreeRecursive(m_rootDirectory);
            ImGui::EndChild();

            // Right Pane: Asset Cards Grid
            ImGui::TableSetColumnIndex(1);
            ImGui::BeginChild("AssetGridPane", ImVec2(0, 0), false);
            DrawAssetGrid();

            // Deselect when clicking on empty background area in the asset grid
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
                !m_isAnyCardHovered)
            {
                m_selectedAssetPath.clear();
            }

            // Right-click on empty background area context menu
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
                !m_isAnyCardHovered)
            {
                ImGui::OpenPopup("BackgroundContextMenu");
            }
            if (ImGui::BeginPopup("BackgroundContextMenu"))
            {
                ImGui::TextDisabled("Folder: %s", m_currentDirectory.c_str());
                ImGui::Separator();
                if (ImGui::MenuItem("Show in Explorer"))
                {
                    OpenPathInExplorer(m_currentDirectory, false);
                }
                if (ImGui::MenuItem("Copy Folder Path"))
                {
                    ImGui::SetClipboardText(m_currentDirectory.c_str());
                    CInspectorUI::GetInstance().SetStatusMessage("Copied folder path: " + m_currentDirectory, 2.0f);
                }
                if (ImGui::MenuItem("Refresh"))
                {
                    RefreshCurrentDirectory();
                }
                ImGui::EndPopup();
            }

            ImGui::EndChild();

            ImGui::EndTable();
        }
    }
    ImGui::End();
}


void CContentDrawerUI::OnAssetRenamed(const std::string& oldPath, const std::string& newPath)
{
	std::string oldNorm = oldPath;
	std::string newNorm = newPath;
	for (auto& c : oldNorm) if (c == '\\') c = '/';
	for (auto& c : newNorm) if (c == '\\') c = '/';

	if (m_selectedAssetPath == oldNorm)
	{
		m_selectedAssetPath = newNorm;
	}
	if (m_currentDirectory == oldNorm)
	{
		m_currentDirectory = newNorm;
	}
	else if (m_currentDirectory.rfind(oldNorm + "/", 0) == 0)
	{
		m_currentDirectory = newNorm + m_currentDirectory.substr(oldNorm.length());
	}

	m_pendingRefresh = true;
}

void CContentDrawerUI::OnAssetDeleted(const std::string& deletedPath)
{
	std::string norm = deletedPath;
	for (auto& c : norm) if (c == '\\') c = '/';

	if (m_selectedAssetPath == norm)
	{
		m_selectedAssetPath.clear();
	}
	m_pendingRefresh = true;
}

void CContentDrawerUI::OnAssetRestored(const std::string& restoredPath)
{
	std::string norm = restoredPath;
	for (auto& c : norm) if (c == '\\') c = '/';

	m_selectedAssetPath = norm;
	m_pendingRefresh = true;
}
