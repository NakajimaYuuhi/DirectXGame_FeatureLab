#pragma once
#include <string>
#include <vector>
#include "AssetRenameService.h"
#include "AssetDeleteService.h"
#include "UndoManager.h"
#include <unordered_map>
#include <memory>
#include "imgui.h"

class CTexture;

// =================================================================
// アセットエントリ種別 (Content Drawer)
// =================================================================
enum class AssetEntryType
{
    Folder = 0,
    Scene,
    Prefab,
    Shader,
    Material,
    Model,
    Texture,
    Pipeline,
    Audio,
    Other
};

// =================================================================
// Assets フォルダ全体を階層探索・操作できるコンテンツドロワーUIクラス
// =================================================================
class CContentDrawerUI
{
public:
    static CContentDrawerUI& GetInstance()
    {
        static CContentDrawerUI instance;
        return instance;
    }

    // UIの描画更新処理
    void Draw();

    // 表示・非表示フラグの取得と設定
    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }
    void ToggleVisible() { m_isVisible = !m_isVisible; }

    // 現在のディレクトリ（または全体）を再スキャン
    void RefreshCurrentDirectory();

    // 旧互換API
    void RefreshPrefabList() { RefreshCurrentDirectory(); }

    // 指定ディレクトリへ移動
    void NavigateTo(const std::string& folderPath, bool recordHistory = true);

    // 現在選択中のアセットパス取得・設定
    const std::string& GetSelectedAssetPath() const { return m_selectedAssetPath; }
    void SetSelectedAssetPath(const std::string& path) { m_selectedAssetPath = path; }
    void OnAssetRenamed(const std::string& oldPath, const std::string& newPath);
    void OnAssetDeleted(const std::string& deletedPath);
    void OnAssetRestored(const std::string& restoredPath);

private:
    CContentDrawerUI() = default;
    ~CContentDrawerUI() = default;
    CContentDrawerUI(const CContentDrawerUI&) = delete;
    CContentDrawerUI& operator=(const CContentDrawerUI&) = delete;

    struct AssetEntry
    {
        std::string name;         // 表示名 (拡張子なし or ファイル名)
        std::string fileName;     // 拡張子付きファイル名
        std::string path;         // 正規化済み相対パス (例: "Assets/Shader/StandardShadow.hlsl")
        std::string extension;    // 小文字拡張子 (例: ".hlsl")
        AssetEntryType type = AssetEntryType::Other;
        std::string subInfo;      // タグ名や補足情報 (Prefab Tag, ファイルサイズ等)
    };

    void DrawTopNavigationBar();
    void DrawFolderTreeRecursive(const std::string& folderPath);
    void DrawAssetGrid();
    void HandleAssetDoubleClick(const AssetEntry& entry);

    ImTextureID GetAssetIconTextureID(AssetEntryType type);

    static AssetEntryType ClassifyAsset(const std::string& normalizedPath, bool isDirectory, const std::string& extLower);

    bool m_isVisible = true;
    bool m_isInitialized = false;

    std::string m_rootDirectory = "Assets";
    std::string m_currentDirectory = "Assets";
    std::string m_selectedAssetPath = "";
    std::string m_pendingNavigatePath = "";
    bool m_pendingRefresh = false;
    bool m_isAnyCardHovered = false;

    std::vector<std::string> m_backHistory;
    std::vector<std::string> m_forwardHistory;

    std::vector<AssetEntry> m_entries;
    std::unordered_map<int, std::shared_ptr<CTexture>> m_iconTextures;

    char m_searchFilter[128] = "";
    int m_typeFilterIndex = 0; // 0: All, 1: Folder, 2: Scene, 3: Prefab, 4: Shader, 5: Material, 6: Model, 7: Texture, 8: Pipeline, 9: Audio
    float m_cardWidth = 95.0f;

    std::string m_renamingAssetPath;
    char m_renameInputBuf[128] = "";
    bool m_renameInputFocusNeeded = false;
    bool m_showRenameConfirmPopup = false;
    RenamePlan m_activeRenamePlan;

    bool m_showDeleteConfirmPopup = false;
    DeletePlan m_activeDeletePlan;
};
