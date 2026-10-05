#pragma once
#include "SceneEnums.h"
#include "ObjectTag.h"

#include "Object.h"
#include "RenderLayer.h"

class CameraComponent;
#include "CameraComponent.h"
class CameraComponent;
class Player;
class FieldComponent;
#include "FieldComponent.h"

class ObjectManager
{
public:



	//生成処理	// Instantiate with TypeName and unique ObjectName
	CObject* Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName, std::string _ObjectName);

	// Overload: default ObjectName to _TypeName
	CObject* Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName);

	// Add pre-created object instance
	void AddObject(ObjectTag _Tag, CObject* _Object);

	//初期化処理
	void Init(Scenes::ID _SceneID);

	//終了処理
	void Uninit();

	//更新処理
	void Update(Scenes::ID _SceneID);
	void UpdatePhaseAll(UpdatePhase phase, float deltaTime);

	void CollisionUpdate(Scenes::ID _SceneID);	//Collisionの更新

	void Draw(Scenes::ID _SceneID);
	void DrawByLayer(RenderLayer layer);
	void DrawShadow(const DirectX::XMMATRIX& lightViewProj);

	void FlushDestroyedObjects();

	// Pending additions to prevent iterator invalidation during Update loops
	void FlushPendingAddObjects();

private:
	//一旦配列は1つ(2次元)
	Vector <Vector<UniquePtr<CObject>>> vecObject;	//オブジェクトの配列
	Vector<std::pair<ObjectTag, UniquePtr<CObject>>> m_pendingAddObjects;


public:

	//Player
	CObject* GetPlayer();

	//Camera
	CObject* GetCameraObject();
	CameraComponent* GetCamera();
	CameraComponent* GetCameraComponent();

	//Field
	FieldComponent* GetField();

	//Manager
	CObject* GetManager(String name);

	//All Objects for ImGui Inspector
	const Vector<Vector<UniquePtr<CObject>>>& GetObjectList() const { return vecObject; }

//----- シングルトンの実装に必要 -----
public:
	static ObjectManager& GetInstance()
	{
		static ObjectManager Instance;

		//インスタンスを返す
		return Instance;
	}

private:
	//コンストラクタ
	ObjectManager();

	//デストラクタ
	~ObjectManager();

	//コピー禁止
	ObjectManager(const ObjectManager&) = delete;

	//代入禁止
	ObjectManager& operator=(const ObjectManager&) = delete;
};

