#include "EnemyCounterComponent.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "TextRenderer.h"
#include "EventManager.h"
#include "EventData_NextScene.h"
#include <windows.h>

EnemyCounterComponent::EnemyCounterComponent()
    : CComponent("EnemyCounterComponent")
    , m_enemyCount(0)
    , m_defeatCount(0)
    , m_targetTextName("EnemyCount")
    , m_textRenderer(nullptr)
{
}

void EnemyCounterComponent::Init()
{
    RecountEnemies();

    CTextRenderer* tr = GetTextRenderer();
    if (tr)
    {
        tr->SetText(L"Score : " + std::to_wstring(m_defeatCount));
    }
}

void EnemyCounterComponent::Start()
{
    RecountEnemies();

    CTextRenderer* tr = GetTextRenderer();
    if (tr)
    {
        tr->SetText(L"Score : " + std::to_wstring(m_defeatCount));
    }
}

void EnemyCounterComponent::Update(float deltaTime)
{
}

CTextRenderer* EnemyCounterComponent::GetTextRenderer()
{
    if (m_textRenderer && m_textRenderer->GetOwner() && !m_textRenderer->GetOwner()->GetIsDestroyed())
    {
        return m_textRenderer;
    }
    m_textRenderer = nullptr;

    const auto& objectList = ObjectManager::GetInstance().GetObjectList();

    // 1. 指定された名前（m_targetTextName）のオブジェクトから探す
    if (!m_targetTextName.empty())
    {
        for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
        {
            for (const auto& obj : objectList[tagIdx])
            {
                if (!obj || obj->GetIsDestroyed()) continue;
                CObjectInfo* info = obj->GetComponent<CObjectInfo>();
                if (info && info->GetObjectName() == m_targetTextName)
                {
                    auto tr = obj->GetComponent<CTextRenderer>();
                    if (tr)
                    {
                        m_textRenderer = tr;
                        return m_textRenderer;
                    }
                }
            }
        }
    }

    // 2. フォールバック: ObjectTag::TEXT から探す
    if (static_cast<size_t>(ObjectTag::TEXT) < objectList.size())
    {
        for (const auto& obj : objectList[static_cast<size_t>(ObjectTag::TEXT)])
        {
            if (!obj || obj->GetIsDestroyed()) continue;
            auto tr = obj->GetComponent<CTextRenderer>();
            if (tr)
            {
                m_textRenderer = tr;
                return m_textRenderer;
            }
        }
    }

    return nullptr;
}

void EnemyCounterComponent::RecountEnemies()
{
    int activeCount = 0;
    const auto& objectList = ObjectManager::GetInstance().GetObjectList();
    if (static_cast<size_t>(ObjectTag::ENEMY) < objectList.size())
    {
        for (const auto& obj : objectList[static_cast<size_t>(ObjectTag::ENEMY)])
        {
            if (obj && !obj->GetIsDestroyed())
            {
                activeCount++;
            }
        }
    }
    m_enemyCount = activeCount;
}

void EnemyCounterComponent::Increment(int num)
{
    m_enemyCount += num;
}

void EnemyCounterComponent::Decrement(int num)
{
    m_enemyCount -= num;
    if (m_enemyCount < 0) m_enemyCount = 0;
}

void EnemyCounterComponent::ResetCount()
{
    m_defeatCount = 0;
    RecountEnemies();

    CTextRenderer* tr = GetTextRenderer();
    if (tr)
    {
        tr->SetText(L"Score : " + std::to_wstring(m_defeatCount));
    }
}

void EnemyCounterComponent::Defeat(int num)
{
    m_defeatCount += num;

    // 生存敵の数を再計算
    RecountEnemies();

    // スコアUIテキスト更新
    CTextRenderer* tr = GetTextRenderer();
    if (tr)
    {
        tr->SetText(L"Score : " + std::to_wstring(m_defeatCount));
    }

    // 生存敵が 0 以下ならクリアシーンへ遷移
    if (m_enemyCount <= 0)
    {
        Event event;
        EventData_NextScene* eventData = new EventData_NextScene(Scenes::ID::Clear);
        event.SetEventData(eventData);
        event.SetEventID(Events::ID::ChangeScene);
        EventManager::GetInstance().AddEvent(event);
    }
}
