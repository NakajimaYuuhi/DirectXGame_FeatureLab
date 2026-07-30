#pragma once
#include "ISelectable.h"

//------------------------------------------------------------------------------
// CEventManager
// UIのフォーカス管理と、入力に応じたナビゲーション処理を行うシングルトン
//------------------------------------------------------------------------------
class ButtonEventManager
{
public:
    static ButtonEventManager& GetInstance();

    // 現在フォーカスされているUIを設定する
    void SetSelectedGameObject(ISelectable* newSelected);

    // 現在のフォーカス対象を取得
    ISelectable* GetSelectedGameObject() const { return m_currentSelected; }

    // 毎フレームの更新（入力によるナビゲーション処理など）
    void Update();

    // 驕ｸ謚樒憾諷九ｒ繧ｯ繝ｪ繧｢縺吶ｋ
    void ClearSelectedGameObject() { m_currentSelected = nullptr; }

private:
    ButtonEventManager();
    ~ButtonEventManager() = default;
    ButtonEventManager(const ButtonEventManager&) = delete;
    ButtonEventManager& operator=(const ButtonEventManager&) = delete;

    ISelectable* m_currentSelected;
};
