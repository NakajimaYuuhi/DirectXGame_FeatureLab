#pragma once

// =================================================================
// エディタ上部のツールバーUI（プレイ/編集制御、ギズモ切替、シーン保存等）
// =================================================================
class CEditorToolbarUI
{
public:
    // シングルトンインスタンスの取得
    static CEditorToolbarUI& GetInstance()
    {
        static CEditorToolbarUI instance;
        return instance;
    }

    // ツールバーUIの描画処理
    void Draw();

    // 表示・非表示フラグの取得と設定
    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }

private:
    CEditorToolbarUI() = default;
    ~CEditorToolbarUI() = default;
    CEditorToolbarUI(const CEditorToolbarUI&) = delete;
    CEditorToolbarUI& operator=(const CEditorToolbarUI&) = delete;

    bool m_isVisible = true; // ツールバー表示フラグ
};
