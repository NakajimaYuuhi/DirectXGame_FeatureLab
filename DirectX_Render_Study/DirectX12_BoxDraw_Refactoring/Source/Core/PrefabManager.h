#pragma once
#include "Object.h"
#include <string>
#include <unordered_map>
#include <functional>

class PrefabManager
{
public:
	using PrefabFactory = std::function<CObject*(const std::string& name)>;

	static PrefabManager& GetInstance()
	{
		static PrefabManager instance;
		return instance;
	}

	void RegisterPrefab(const std::string& typeName, PrefabFactory factory);
	CObject* Instantiate(const std::string& typeName, const std::string& instanceName = "");

	// Register built-in component-based prefabs
	void InitDefaultPrefabs();

	// JSON Prefab methods
	CObject* InstantiateFromJSON(const std::string& jsonPath, const std::string& instanceName = "");
	bool RegisterPrefabJSON(const std::string& typeName, const std::string& jsonPath);

private:
	PrefabManager();
	~PrefabManager() = default;

	PrefabManager(const PrefabManager&) = delete;
	PrefabManager& operator=(const PrefabManager&) = delete;

	std::unordered_map<std::string, PrefabFactory> m_registry;
};
