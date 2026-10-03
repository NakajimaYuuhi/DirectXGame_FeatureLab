#include "PrefabManager.h"
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
#include "BulletComponent.h"
#include "CameraComponent.h"
#include "audio.h"
#include "ComponentFactory.h"
#include "Source/External/json.hpp"
#include <fstream>
#include <filesystem>

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
		CObject* obj = it->second(finalName);
		if (obj)
		{
			CObjectInfo* info = obj->GetComponent<CObjectInfo>();
			if (info && info->GetPrefabName().empty())
			{
				info->SetPrefabName(typeName);
			}
		}
		return obj;
	}
	return nullptr;
}

void PrefabManager::InitDefaultPrefabs()
{
	// 1. Wizard Player Prefab Recipe
	RegisterPrefab("PlayerPrefab", [](const std::string& name) -> CObject* {
		auto obj = new CObject(name);
		
		CObjectInfo* info = obj->GetComponent<CObjectInfo>();
		if (info) info->SetObjectTag(ObjectTag::PLAYER);

		CTransform* transform = obj->GetComponent<CTransform>();
		if (transform) transform->SetScale({ 0.5f, 0.5f, 0.5f });

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
		auto obj = new CObject(name);

		CObjectInfo* info = obj->GetComponent<CObjectInfo>();
		if (info) info->SetObjectTag(ObjectTag::ENEMY);

		CTransform* transform = obj->GetComponent<CTransform>();
		if (transform) transform->SetScale({ 0.5f, 0.5f, 0.5f });

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

	// Automatically scan and register all JSON prefabs in Assets/Prefabs
	RegisterPrefabsInDirectory("Assets/Prefabs");
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

	auto obj = new CObject(finalName);

	// Tag
	std::string tagStr = j.value("Tag", "NONE");
	CObjectInfo* info = obj->GetComponent<CObjectInfo>();
	if (info)
	{
		info->SetPrefabName(prefabName);
		if (tagStr == "BACKGROUND") info->SetObjectTag(ObjectTag::BACKGROUND);
		else if (tagStr == "PLAYER") info->SetObjectTag(ObjectTag::PLAYER);
		else if (tagStr == "PLAYER_BULLET") info->SetObjectTag(ObjectTag::PLAYER_BULLET);
		else if (tagStr == "ENEMY") info->SetObjectTag(ObjectTag::ENEMY);
		else if (tagStr == "ENEMY_BULLET") info->SetObjectTag(ObjectTag::ENEMY_BULLET);
		else if (tagStr == "FIELD") info->SetObjectTag(ObjectTag::FIELD);
		else if (tagStr == "TRIANGLE") info->SetObjectTag(ObjectTag::TRIANGLE);
		else if (tagStr == "BILLBOARD") info->SetObjectTag(ObjectTag::BILLBOARD);
		else if (tagStr == "EFFECT") info->SetObjectTag(ObjectTag::EFFECT);
		else if (tagStr == "UI") info->SetObjectTag(ObjectTag::UI);
		else if (tagStr == "TEXT") info->SetObjectTag(ObjectTag::TEXT);
		else if (tagStr == "CAMERA") info->SetObjectTag(ObjectTag::CAMERA);
		else if (tagStr == "FADE") info->SetObjectTag(ObjectTag::FADE);
		else if (tagStr == "MANAGER") info->SetObjectTag(ObjectTag::MANAGER);
		else info->SetObjectTag(ObjectTag::NONE);
	}

	if (j.contains("Components") && j["Components"].is_object())
	{
		for (const auto& [compName, compJson] : j["Components"].items())
		{
			ComponentFactory::GetInstance().CreateComponent(compName, obj, compJson);
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

void PrefabManager::RegisterPrefabsInDirectory(const std::string& directoryPath)
{
	namespace fs = std::filesystem;
	if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) return;

	for (const auto& entry : fs::directory_iterator(directoryPath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".json")
		{
			std::string jsonPath = entry.path().string();
			std::string stemName = entry.path().stem().string();

			std::ifstream file(jsonPath);
			if (file.is_open())
			{
				nlohmann::json j;
				try
				{
					file >> j;
					std::string prefabName = j.value("PrefabName", stemName);
					RegisterPrefabJSON(prefabName, jsonPath);
					if (prefabName != stemName)
					{
						RegisterPrefabJSON(stemName, jsonPath);
					}
				}
				catch (...)
				{
					RegisterPrefabJSON(stemName, jsonPath);
				}
			}
		}
	}
}

bool PrefabManager::HasPrefab(const std::string& typeName) const
{
	return m_registry.find(typeName) != m_registry.end();
}
