#pragma once
#include "Object.h"
#include "Component.h"
#include "Source/External/json.hpp"
#include <string>
#include <unordered_map>
#include <functional>

class ComponentFactory
{
public:
	using CreatorFunc = std::function<CComponent*(CObject* owner, const nlohmann::json& params)>;

	static ComponentFactory& GetInstance()
	{
		static ComponentFactory instance;
		return instance;
	}

	// Register a component creator function by name
	void RegisterComponent(const std::string& name, CreatorFunc creator);

	// Create component on an owner object using registered name and JSON parameters
	CComponent* CreateComponent(const std::string& name, CObject* owner, const nlohmann::json& params);

	// Register all built-in engine components
	void InitDefaultComponents();

	// Check if a component name is registered
	bool IsRegistered(const std::string& name) const;

private:
	ComponentFactory();
	~ComponentFactory() = default;

	ComponentFactory(const ComponentFactory&) = delete;
	ComponentFactory& operator=(const ComponentFactory&) = delete;

	std::unordered_map<std::string, CreatorFunc> m_creators;
};
