#pragma once
#include "EngineAPI.h"
#include "Component.h"
#include <string>

class CTextRenderer;

class EnemyCounterComponent : public CComponent
{
public:
    ENGINE_API EnemyCounterComponent();
    virtual ~EnemyCounterComponent() = default;

    ENGINE_API virtual void Init() override;
    ENGINE_API virtual void Start() override;
    ENGINE_API virtual void Update(float deltaTime) override;

    // 敵生存数の再計算
    ENGINE_API void RecountEnemies();

    // カウント操作
    ENGINE_API void Increment(int num = 1);
    ENGINE_API void Decrement(int num = 1);
    ENGINE_API void ResetCount();

    // 撃破処理（撃破数加算、生存数更新、クリア判定、スコアUI更新）
    ENGINE_API void Defeat(int num = 1);

    // ゲッター
    int GetCount() const { return m_enemyCount; }
    int GetDefeatCount() const { return m_defeatCount; }

    // UI テキスト取得
    ENGINE_API CTextRenderer* GetTextRenderer();

    // UI の対象オブジェクト名（空ならタグから自動探索）
    void SetTargetTextName(const std::string& name) { m_targetTextName = name; }
    const std::string& GetTargetTextName() const { return m_targetTextName; }

private:
    int m_enemyCount = 0;
    int m_defeatCount = 0;
    std::string m_targetTextName = "EnemyCount";
    CTextRenderer* m_textRenderer = nullptr;
};
