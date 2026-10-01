#pragma once
#include "ObjectManager.h"
#include "Component.h"
#include <vector>

class ComponentManager
{
public:
	static ComponentManager& GetInstance()
	{
		static ComponentManager instance;
		return instance;
	}

	// Find all components of type T across all active scene objects
	template<typename T>
	std::vector<T*> FindComponentsOfType()
	{
		std::vector<T*> results;
		const auto& objectList = ObjectManager::GetInstance().GetObjectList();
		for (const auto& vec : objectList)
		{
			for (const auto& obj : vec)
			{
				if (obj && !obj->GetIsDestroyed())
				{
					auto comps = obj->GetComponentsOfType<T>();
					results.insert(results.end(), comps.begin(), comps.end());
				}
			}
		}
		return results;
	}

	// Batch enable or disable all components of type T across active scene objects
	template<typename T>
	void SetComponentsEnabledOfType(bool enabled)
	{
		auto comps = FindComponentsOfType<T>();
		for (auto* comp : comps)
		{
			if (comp)
			{
				comp->SetEnabled(enabled);
			}
		}
	}

private:
	ComponentManager() = default;
	~ComponentManager() = default;

	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;
};
