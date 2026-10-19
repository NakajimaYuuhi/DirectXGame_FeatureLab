#if defined(GAMELOGIC_DLL_BUILD)

#include <Windows.h>
#include "ComponentFactory.h"
#include "HealthComponent.h"
#include "PlayerControllerComponent.h"
#include "EnemyAIComponent.h"
#include "BulletComponent.h"

namespace
{
    CComponent* CreateHealthComponent(CObject* owner, const nlohmann::json& p)
    {
        int maxHp = p.value("MaxHP", 10);
        float invDur = p.value("InvincibleDuration", 1.5f);
        float blkInt = p.value("BlinkInterval", 0.08f);
        HealthComponent* health = owner->GetComponent<HealthComponent>();
        if (!health) health = owner->AddComponent<HealthComponent>(maxHp, invDur, blkInt);

        if (health)
        {
            if (auto controller = owner->GetComponent<PlayerControllerComponent>())
            {
                health->SetOnDamagedCallback([controller](int, int) {
                    if (controller) controller->OnDamaged();
                });
                health->SetOnDieCallback([controller]() {
                    if (controller) controller->OnDie();
                });
            }
            if (auto ai = owner->GetComponent<EnemyAIComponent>())
            {
                health->SetOnDamagedCallback([ai](int, int) {
                    if (ai) ai->OnDamaged();
                });
                health->SetOnDieCallback([ai]() {
                    if (ai) ai->OnDie();
                });
            }
        }
        return health;
    }

    CComponent* CreatePlayerControllerComponent(CObject* owner, const nlohmann::json& p)
    {
        PlayerControllerComponent* controller = owner->GetComponent<PlayerControllerComponent>();
        if (!controller) controller = owner->AddComponent<PlayerControllerComponent>();

        HealthComponent* health = owner->GetComponent<HealthComponent>();
        if (health && controller)
        {
            health->SetOnDamagedCallback([controller](int, int) {
                if (controller) controller->OnDamaged();
            });
            health->SetOnDieCallback([controller]() {
                if (controller) controller->OnDie();
            });
        }
        return controller;
    }

    CComponent* CreateEnemyAIComponent(CObject* owner, const nlohmann::json& p)
    {
        EnemyAIComponent* ai = owner->GetComponent<EnemyAIComponent>();
        if (!ai) ai = owner->AddComponent<EnemyAIComponent>();

        if (ai)
        {
            if (p.contains("DeathEffectPrefab"))
            {
                ai->SetDeathEffectPrefab(p["DeathEffectPrefab"].get<std::string>());
            }
            if (p.contains("DamagedEffectPrefab"))
            {
                ai->SetDamagedEffectPrefab(p["DamagedEffectPrefab"].get<std::string>());
            }
        }

        HealthComponent* health = owner->GetComponent<HealthComponent>();
        if (health && ai)
        {
            health->SetOnDamagedCallback([ai](int, int) {
                if (ai) ai->OnDamaged();
            });
            health->SetOnDieCallback([ai]() {
                if (ai) ai->OnDie();
            });
        }
        return ai;
    }

    CComponent* CreateBulletComponent(CObject* owner, const nlohmann::json& p)
    {
        BulletComponent* bullet = owner->GetComponent<BulletComponent>();
        if (!bullet) bullet = owner->AddComponent<BulletComponent>();

        if (bullet)
        {
            bullet->SetSpeed(p.value("Speed", 0.04f));
            bullet->SetLifeTime(p.value("LifeTime", 5.0f));
            bullet->SetDamage(p.value("Damage", 1));
            bullet->SetHitEffectPrefab(p.value("HitEffectPrefab", "RandomParticle"));

            std::string targetTagStr = p.value("TargetTag", "ENEMY");
            if (targetTagStr == "PLAYER") bullet->SetTargetTag(ObjectTag::PLAYER);
            else bullet->SetTargetTag(ObjectTag::ENEMY);

            if (p.contains("Direction") && p["Direction"].is_array() && p["Direction"].size() >= 3)
            {
                bullet->SetDirection({ p["Direction"][0], p["Direction"][1], p["Direction"][2] });
            }
        }
        return bullet;
    }
}

extern "C" __declspec(dllexport) void RegisterGameLogicComponents(ComponentFactory* factory)
{
    if (!factory) return;

    factory->RegisterRawComponent("Health", &CreateHealthComponent);
    factory->RegisterRawComponent("HealthComponent", &CreateHealthComponent);

    factory->RegisterRawComponent("PlayerController", &CreatePlayerControllerComponent);
    factory->RegisterRawComponent("PlayerControllerComponent", &CreatePlayerControllerComponent);

    factory->RegisterRawComponent("EnemyAI", &CreateEnemyAIComponent);
    factory->RegisterRawComponent("EnemyAIComponent", &CreateEnemyAIComponent);

    factory->RegisterRawComponent("Bullet", &CreateBulletComponent);
    factory->RegisterRawComponent("BulletComponent", &CreateBulletComponent);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

#endif // GAMELOGIC_DLL_BUILD
