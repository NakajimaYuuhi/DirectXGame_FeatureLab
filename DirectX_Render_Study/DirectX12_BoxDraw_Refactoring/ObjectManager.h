#pragma once
#include "SceneEnums.h"
#include "ObjectTag.h"

#include "Object.h"

class Camera;
class Player;

class ObjectManager
{
public:



	//生成処理
	CObject* Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName);

	//初期化処理
	void Init(Scenes::ID _SceneID);

	//終了処理
	void Uninit();

	//更新処理
	void Update(Scenes::ID _SceneID);

	void CollisionUpdate(Scenes::ID _SceneID);	//Collisionの更新

	void Draw(Scenes::ID _SceneID);			//`

	void FlushDestroyedObjects();

private:
	//一旦配列は1つ(2次元)
	Vector <Vector<UniquePtr<CObject>>> vecObject;	//オブジェクトの配列


public:

	//Player
	Player* GetPlayer();

	//Camera
	Camera* GetCamera();

	//Manager
	CObject* GetManager(String name);

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

