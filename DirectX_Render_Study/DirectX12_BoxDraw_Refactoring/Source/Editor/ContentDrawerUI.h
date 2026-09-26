#pragma once
#include <string>
#include <vector>

// =================================================================
// プレハブ(Prefab)一覧の閲覧・生成・編集を行うコンテンツドロワーUIクラス
// =================================================================
class CContentDrawerUI
{
public:
    // シングルトンインスタンスの取得
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

    // Assets/Prefabs フォルダからプレハブ一覧を再読み込み
    void RefreshPrefabList();

private:
    CContentDrawerUI() = default;
    ~CContentDrawerUI() = default;
    CContentDrawerUI(const CContentDrawerUI&) = delete;
    CContentDrawerUI& operator=(const CContentDrawerUI&) = delete;

    // プレハブの個別の情報を保持する構造体
    struct PrefabItem
    {
        std::string name; // プレハブ名（ファイル名）
        std::string path; // ファイルパス
        std::string tag;  // タグ情報
    };

    bool m_isVisible = true;                 // ウィンドウの表示状態フラグ
    std::vector<PrefabItem> m_prefabItems;   // 読込済みプレハブ情報リスト
    bool m_isInitialized = false;            // 初期構築済みフラグ
    char m_searchFilter[128] = "";           // 検索フィルターバッファ
};