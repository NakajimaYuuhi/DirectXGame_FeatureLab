#pragma once
#include "SceneEnums.h"
#include "ObjectTag.h"

#include "Object.h"

class ObjectManager
{
public:

	//生成処理
	CObject* Instantiate(Scene::ID _SceneID, ObjectTag _Tag, std::string _TypeName);

	//初期化処理
	void Init();

	//終了処理
	void Uninit();

	//更新処理
	void Update(Scene::ID _SceneID);

	void Draw(Scene::ID _SceneID);			//描画

private:
	Vector<UniquePtr<CObject>> vecObject;	//オブジェクトの配列

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

