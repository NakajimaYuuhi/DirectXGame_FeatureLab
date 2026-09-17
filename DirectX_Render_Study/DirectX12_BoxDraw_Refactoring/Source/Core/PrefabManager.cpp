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
#include "Source/External/json.hpp"
#include <fstream>

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

	// Register JSON Prefabs if available
	RegisterPrefabJSON("PlayerJSON", "Assets/Prefabs/Player.json");
	RegisterPrefabJSON("EnemyJSON", "Assets/Prefabs/Enemy.json");
}

CObject* PrefabManager::InstantiateFromJSON(const std::string& jsonPath, const std::string& instanceName)
{
	std::ifstream file(jsonPath);
	if (!file.is_open())
	{
		return nullptr;
	}

	nlohmann::json j;
	try
	{
		file >> j;
	}
	catch (...)
	{
		return nullptr;
	}

	std::string prefabName = j.value("PrefabName", "PrefabObject");
	std::string finalName = instanceName.empty() ? prefabName : instanceName;

	auto obj = new C3D_Object(finalName);

	// Tag
	std::string tagStr = j.value("Tag", "NONE");
	CObjectInfo* info = obj->GetComponent<CObjectInfo>();
	if (info)
	{
		if (tagStr == "PLAYER") info->SetObjectTag(ObjectTag::PLAYER);
		else if (tagStr == "ENEMY") info->SetObjectTag(ObjectTag::ENEMY);
		else if (tagStr == "PLAYER_BULLET") info->SetObjectTag(ObjectTag::PLAYER_BULLET);
		else if (tagStr == "ENEMY_BULLET") info->SetObjectTag(ObjectTag::ENEMY_BULLET);
	}

	if (!j.contains("Components")) return obj;
	const auto& comps = j["Components"];

	// Transform
	if (comps.contains("Transform"))
	{
		const auto& t = comps["Transform"];
		if (t.contains("Scale") && t["Scale"].is_array() && t["Scale"].size() >= 3)
		{
			obj->SetScale({ t["Scale"][0], t["Scale"][1], t["Scale"][2] });
		}
	}

	// Model
	if (comps.contains("Model"))
	{
		const auto& m = comps["Model"];
		std::string modelPath = m.value("ModelPath", "");
		if (!modelPath.empty())
		{
			CModel* model = obj->GetComponent<CModel>();
			if (model)
			{
				auto sharedModel = ModelManager::GetInstance().GetModel(modelPath);
				model->CopyFrom(sharedModel);
				std::string defAnim = m.value("DefaultAnimation", "Idle");
				model->PlayAnimation(defAnim);

				std::string rLayerStr = m.value("RenderLayer", "Opaque");
				if (rLayerStr == "Transparent") model->SetRenderLayer(RenderLayer::Transparent);
				else if (rLayerStr == "UI") model->SetRenderLayer(RenderLayer::UI);
				else model->SetRenderLayer(RenderLayer::Opaque);
			}
		}
	}

	// BoxCollider3D
	if (comps.contains("BoxCollider3D"))
	{
		const auto& c = comps["BoxCollider3D"];
		BoxCollider3D* collider = obj->AddComponent<BoxCollider3D>();

		if (c.contains("Size") && c["Size"].is_array() && c["Size"].size() >= 3)
		{
			collider->SetSize({ c["Size"][0], c["Size"][1], c["Size"][2] });
		}
		if (c.contains("Offset") && c["Offset"].is_array() && c["Offset"].size() >= 3)
		{
			collider->SetOffset({ c["Offset"][0], c["Offset"][1], c["Offset"][2] });
		}
		collider->SetIsTrigger(c.value("IsTrigger", false));
		collider->SetLayer(c.value("Layer", (uint32_t)CollisionLayer::Default));
		collider->SetCollisionMask(c.value("CollisionMask", (uint32_t)CollisionLayer::All));
	}

	// Audio
	if (comps.contains("Audio"))
	{
		const auto& a = comps["Audio"];
		std::string audioPath = a.value("FilePath", "");
		if (!audioPath.empty())
		{
			Audio* audio = obj->AddComponent<Audio>();
			audio->Load(audioPath.c_str());
		}
	}

	// Gravity
	if (comps.contains("Gravity"))
	{
		const auto& g = comps["Gravity"];
		float grav = g.value("Gravity", -25.0f);
		float jPow = g.value("JumpPower", 0.0f);
		obj->AddComponent<GravityComponent>(grav, jPow);
	}

	// Movement
	if (comps.contains("Movement"))
	{
		const auto& mv = comps["Movement"];
		float spd = mv.value("Speed", 0.05f);
		auto moveComp = obj->AddComponent<CharacterMovementComponent>(spd);
		moveComp->SetMinClimbNormalY(mv.value("MinClimbNormalY", 0.45f));
		moveComp->SetAutoRotate(mv.value("AutoRotate", true));
	}

	// Health
	HealthComponent* health = nullptr;
	if (comps.contains("Health"))
	{
		const auto& h = comps["Health"];
		int maxHp = h.value("MaxHP", 10);
		float invDur = h.value("InvincibleDuration", 1.5f);
		float blkInt = h.value("BlinkInterval", 0.08f);
		health = obj->AddComponent<HealthComponent>(maxHp, invDur, blkInt);
	}

	// PlayerController
	if (comps.contains("PlayerController"))
	{
		auto controller = obj->AddComponent<PlayerControllerComponent>();
		if (health)
		{
			health->SetOnDamagedCallback([controller](int, int) {
				if (controller) controller->OnDamaged();
			});
			health->SetOnDieCallback([controller]() {
				if (controller) controller->OnDie();
			});
		}
	}

	// EnemyAI
	if (comps.contains("EnemyAI"))
	{
		auto ai = obj->AddComponent<EnemyAIComponent>();
		if (health)
		{
			health->SetOnDamagedCallback([ai](int, int) {
				if (ai) ai->OnDamaged();
			});
			health->SetOnDieCallback([ai]() {
				if (ai) ai->OnDie();
			});
		}
	}

	return obj;
}

bool PrefabManager::RegisterPrefabJSON(const std::string& typeName, const std::string& jsonPath)
{
	RegisterPrefab(typeName, [this, jsonPath](const std::string& name) -> CObject* {
		return InstantiateFromJSON(jsonPath, name);
	});
	return true;
}
