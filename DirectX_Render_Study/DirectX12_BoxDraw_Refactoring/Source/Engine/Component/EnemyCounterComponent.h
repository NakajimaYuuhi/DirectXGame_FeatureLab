#pragma once
#include "Component.h"
#include <string>

class CTextRenderer;

class EnemyCounterComponent : public CComponent
{
public:
    EnemyCounterComponent();
    virtual ~EnemyCounterComponent() = default;

    virtual void Init() override;
    virtual void Start() override;
    virtual void Update(float deltaTime) override;

    // 敵生存数の再計算
    void RecountEnemies();

    // カウント操作
    void Increment(int num = 1);
    void Decrement(int num = 1);
    void ResetCount();

    // 撃破処理（撃破数加算、生存数更新、クリア判定、スコアUI更新）
    void Defeat(int num = 1);

    // ゲッター
    int GetCount() const { return m_enemyCount; }
    int GetDefeatCount() const { return m_defeatCount; }

    // UI テキスト取得
    CTextRenderer* GetTextRenderer();

    // UI の対象オブジェクト名（空ならタグから自動探索）
    void SetTargetTextName(const std::string& name) { m_targetTextName = name; }
    const std::string& GetTargetTextName() const { return m_targetTextName; }

private:
    int m_enemyCount = 0;
    int m_defeatCount = 0;
    std::string m_targetTextName = "EnemyCount";
    CTextRenderer* m_textRenderer = nullptr;
};
