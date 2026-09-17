#include "PrefabManager.h"
#include "3D_Object.h"
#include "Transform.h"
#include "Model.h"
#include "ModelManager.h"
#include "BoxCollider3D.h"
#include "ObjectInfo.h"
#include "GravityComponent.h"
#include "CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "PlayerControllerComponent.h"
#include "EnemyAIComponent.h"
#include "audio.h"

PrefabManager::PrefabManager()
{
	InitDefaultPrefabs();
}

void PrefabManager::RegisterPrefab(const std::string& typeName, PrefabFactory factory)
{
	m_registry[typeName] = factory;
}

CObject* PrefabManager::Instantiate(const std::string& typeName, const std::string& instanceName)
{
	auto it = m_registry.find(typeName);
	if (it != m_registry.end())
	{
		std::string finalName = instanceName.empty() ? typeName : instanceName;
		return it->second(finalName);
	}
	return nullptr;
}

void PrefabManager::InitDefaultPrefabs()
{
	// 1. Wizard Player Prefab Recipe
	RegisterPrefab("PlayerPrefab", [](const std::string& name) -> CObject* {
		auto obj = new C3D_Object(name);
		
		CObjectInfo* info = obj->GetComponent<CObjectInfo>();
		if (info) info->SetObjectTag(ObjectTag::PLAYER);

		obj->SetScale({ 0.5f, 0.5f, 0.5f });

		// Model
		CModel* model = obj->GetComponent<CModel>();
		if (model)
		{
			auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/Wizard.glb");
			model->CopyFrom(sharedModel);
			model->PlayAnimation("Idle");
		}

		// Collider
		BoxCollider3D* collider = obj->AddComponent<BoxCollider3D>();
		collider->SetOffset({ 0.0f, 0.75f, 0.0f });
		collider->SetSize({ 0.6f, 1.5f, 0.6f });

		// Audio
		Audio* audio = obj->AddComponent<Audio>();
		audio->Load("Assets/Audio/SE/Fire1.wav");

		// Core components
		obj->AddComponent<GravityComponent>(-25.0f, 8.5f);
		obj->AddComponent<CharacterMovementComponent>(0.1f);
		HealthComponent* health = obj->AddComponent<HealthComponent>(10, 1.5f, 0.08f);
		PlayerControllerComponent* controller = obj->AddComponent<PlayerControllerComponent>();

		// Bind health callbacks to controller
		health->SetOnDamagedCallback([controller](int, int) {
			if (controller) controller->OnDamaged();
		});
		health->SetOnDieCallback([controller]() {
			if (controller) controller->OnDie();
		});

		return obj;
	});

	// 2. Monk Enemy Prefab Recipe
	RegisterPrefab("EnemyPrefab", [](const std::string& name) -> CObject* {
		auto obj = new C3D_Object(name);

		CObjectInfo* info = obj->GetComponent<CObjectInfo>();
		if (info) info->SetObjectTag(ObjectTag::ENEMY);

		obj->SetScale({ 0.5f, 0.5f, 0.5f });

		// Model
		CModel* model = obj->GetComponent<CModel>();
		if (model)
		{
			auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/Monk.glb");
			model->CopyFrom(sharedModel);
			model->PlayAnimation("Idle");
		}

		// Collider
		BoxCollider3D* collider = obj->AddComponent<BoxCollider3D>();
		collider->SetOffset({ 0.0f, 0.75f, 0.0f });
		collider->SetSize({ 0.6f, 1.5f, 0.6f });

		// Core components
		obj->AddComponent<GravityComponent>(-25.0f, 0.0f);
		obj->AddComponent<CharacterMovementComponent>(0.05f);
		HealthComponent* health = obj->AddComponent<HealthComponent>(3, 0.3f, 0.06f);
		EnemyAIComponent* ai = obj->AddComponent<EnemyAIComponent>();

		// Bind health callbacks to AI
		health->SetOnDamagedCallback([ai](int, int) {
			if (ai) ai->OnDamaged();
		});
		health->SetOnDieCallback([ai]() {
			if (ai) ai->OnDie();
		});

		return obj;
	});
}
