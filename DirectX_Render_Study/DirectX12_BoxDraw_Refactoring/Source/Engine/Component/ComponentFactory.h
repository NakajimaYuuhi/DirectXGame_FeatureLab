#pragma once
#include "EngineAPI.h"
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
	using RawCreatorFunc = CComponent* (*)(CObject* owner, const nlohmann::json& params);

	ENGINE_API static ComponentFactory& GetInstance();

	// Register a component creator function by name
	ENGINE_API void RegisterComponent(const std::string& name, CreatorFunc creator);

	// DLL-safe registration using raw C string and function pointer (allocates std::string/map entry on EXE heap)
	ENGINE_API void RegisterRawComponent(const char* name, RawCreatorFunc creator);

	// Create component on an owner object using registered name and JSON parameters
	ENGINE_API CComponent* CreateComponent(const std::string& name, CObject* owner, const nlohmann::json& params);

	// Register all built-in engine components
	ENGINE_API void InitDefaultComponents();

	// Check if a component name is registered
	ENGINE_API bool IsRegistered(const std::string& name) const;

private:
	ComponentFactory();
	~ComponentFactory() = default;

	ComponentFactory(const ComponentFactory&) = delete;
	ComponentFactory& operator=(const ComponentFactory&) = delete;

	std::unordered_map<std::string, CreatorFunc> m_creators;
};
