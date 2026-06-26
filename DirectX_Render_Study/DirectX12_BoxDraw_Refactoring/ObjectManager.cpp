//ヘッダ
#include "ObjectManager.h"

//衝突
#include "Collision.h"


//===== Getter =====
//初期化処理
void ObjectManager::Init(Scenes::ID _SceneID)
{
	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			object->Init();
		}
	}


}

//終了処理
void ObjectManager::Uninit()
{
	vecObject.clear();
	vecObject.resize(ObjectTag::NUM);
}


//更新処理
void ObjectManager::Update(Scenes::ID _SceneID)
{
	for(auto& vec : vecObject)
	{
		for(auto& object : vec)
		{
			object->Update();
		}
	}

	//Collisionの更新
	CollisionUpdate(_SceneID);

	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			if (!object->GetIsDestroyed()) {
				object->LateUpdate();
			}
		}
	}
}

//削除処理
void ObjectManager::FlushDestroyedObjects()
{
	for (auto& vec : vecObject)
	{
		vec.erase(
			std::remove_if(vec.begin(), vec.end(),
				[](const std::unique_ptr<CObject>& obj) {
					return obj->GetIsDestroyed();
				}),
			vec.end()
		);
	}
}

//衝突処理
void ObjectManager::CollisionUpdate(Scenes::ID _SceneID)
{
	//Collisionの更新
	//CollisionOrderを取得
	Vector<Vector<ObjectTag>>& CollisionOrder = Collision::GetInstance().GetCollisionOrder();

	//CollisionOrderの順番で衝突判定
	for (auto& order : CollisionOrder)
	{
		//orderの順番で衝突判定
		for (size_t i = 0; i < vecObject[static_cast<int>(order[0])].size(); i++)
		{
			for (size_t j = 0; j < vecObject[static_cast<int>(order[1])].size(); j++)
			{
				//衝突判定
				BoxCollider3D* colliderA = vecObject[static_cast<int>(order[0])][i]->GetComponent<BoxCollider3D>();
				BoxCollider3D* colliderB = vecObject[static_cast<int>(order[1])][j]->GetComponent<BoxCollider3D>();
				if (colliderA && colliderB)
				{
					if (Collision::CheckCollision(colliderA, colliderB))
					{
						//衝突しているときの処理
						vecObject[static_cast<int>(order[0])][i]->OnCollision(vecObject[static_cast<int>(order[1])][j].get());
						vecObject[static_cast<int>(order[1])][j]->OnCollision(vecObject[static_cast<int>(order[0])][i].get());
					}
				}
			}
		}
	}
}

//描画処理
void ObjectManager::Draw(Scenes::ID _SceneID)
{
	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			if (!object->GetIsDestroyed()) {
				object->Draw();
			}
		}
	}
}



//コンストラクタ
ObjectManager::ObjectManager()
{
	//Resizeを掛ける
	vecObject.resize(ObjectTag::NUM);
}

//デストラクタ
ObjectManager::~ObjectManager()
{
}

