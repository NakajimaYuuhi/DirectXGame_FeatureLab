#pragma once
#include "EngineAPI.h"
#include "SceneEnums.h"
#include "ObjectTag.h"

#include "Object.h"
#include "RenderLayer.h"

class CameraComponent;
#include "CameraComponent.h"
class Player;
class FieldComponent;
#include "FieldComponent.h"

class ObjectManager
{
public:
	// Instantiate with TypeName and unique ObjectName
	ENGINE_API CObject* Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName, std::string _ObjectName);

	// Overload: default ObjectName to _TypeName
	ENGINE_API CObject* Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName);

	// Add pre-created object instance
	ENGINE_API void AddObject(ObjectTag _Tag, CObject* _Object);

	// èâä˙âªèàóù
	ENGINE_API void Init(Scenes::ID _SceneID);

	// èIóπèàóù
	ENGINE_API void Uninit();

	// çXêVèàóù
	ENGINE_API void Update(Scenes::ID _SceneID);
	ENGINE_API void UpdatePhaseAll(UpdatePhase phase, float deltaTime);

	ENGINE_API void CollisionUpdate(Scenes::ID _SceneID);

	ENGINE_API void Draw(Scenes::ID _SceneID);
	ENGINE_API void DrawByLayer(RenderLayer layer, uint32_t tagMask = 0xFFFFFFFFu);
	ENGINE_API void DrawFiltered(uint32_t tagMask = 0xFFFFFFFFu, int layerFilter = -1);
	ENGINE_API void DrawShadow(const DirectX::XMMATRIX& lightViewProj);

	ENGINE_API void FlushDestroyedObjects();

	// Pending additions to prevent iterator invalidation during Update loops
	ENGINE_API void FlushPendingAddObjects();

private:
	Vector <Vector<UniquePtr<CObject>>> vecObject;
	Vector<std::pair<ObjectTag, UniquePtr<CObject>>> m_pendingAddObjects;

public:
	// Player
	ENGINE_API CObject* GetPlayer();

	// Camera
	ENGINE_API CObject* GetCameraObject();
	ENGINE_API CameraComponent* GetCamera();
	ENGINE_API CameraComponent* GetCameraComponent();

	// Field
	ENGINE_API FieldComponent* GetField();

	// Manager
	ENGINE_API CObject* GetManager(String name);

	// All Objects for ImGui Inspector
	const Vector<Vector<UniquePtr<CObject>>>& GetObjectList() const { return vecObject; }

public:
	ENGINE_API static ObjectManager& GetInstance();

private:
	ObjectManager();
	~ObjectManager();

	ObjectManager(const ObjectManager&) = delete;
	ObjectManager& operator=(const ObjectManager&) = delete;
};
