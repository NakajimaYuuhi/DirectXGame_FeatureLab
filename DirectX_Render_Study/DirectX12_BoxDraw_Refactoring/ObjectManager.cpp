#include "ObjectManager.h"

#include "3D_Object.h"
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "BillBoard.h"
#include "RandomParticle.h"
#include "Explosion.h"

#include "Collision.h"

#include "UIObject.h"
#include "TextObject.h"

CObject* ObjectManager::Instantiate(Scene::ID _SceneID, ObjectTag _Tag, std::string _TypeName)
{
    //Todo : Factoryを作る

    //生成
    //Mapとかできれいに分岐させたい
	std::unique_ptr<CObject> tmpObject = std::unique_ptr<CObject>(nullptr);
	CObject* returnObject = nullptr;


    switch (_Tag)
    {
		case ObjectTag::NONE:
			tmpObject = std::make_unique<C3D_Object>("3DObject");
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::FIELD)].push_back(std::move(tmpObject));				//配列に追加

			break;

        case ObjectTag::BACKGROUND:
			break;
		case ObjectTag::UI:
			tmpObject = std::make_unique<CUIObject>(_TypeName);
			returnObject = tmpObject.get();
			vecObject[static_cast<int>(ObjectTag::UI)].push_back(std::move(tmpObject));
			break;
		case ObjectTag::PLAYER:
			tmpObject = std::make_unique<Player>("Player");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::PLAYER)].push_back(std::move(tmpObject));				//配列に追加
			break;
		case ObjectTag::PLAYER_BULLET:



			tmpObject = std::make_unique<Bullet>("Bullet");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::PLAYER_BULLET)].push_back(std::move(tmpObject));		//配列に追加
			break;
		case ObjectTag::ENEMY:
			tmpObject = std::make_unique<Enemy>("Enemy");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::ENEMY)].push_back(std::move(tmpObject));				//配列に追加
            break;
		case ObjectTag::ENEMY_BULLET:
			break;
		case ObjectTag::FIELD:
			//Floor

			break;
		case ObjectTag::BILLBOARD:
			if (_TypeName == "RandomParticle")
			{
				tmpObject = std::make_unique<RandomParticle>("Particle");		//生成
				returnObject = tmpObject.get();							//生ポインタ取得
				vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
				break;
			}
			else if (_TypeName == "Explosion")
			{
				tmpObject = std::make_unique<Explosion>("Explosion");		//生成
				returnObject = tmpObject.get();							//生ポインタ取得
				vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
				break;
			}


			tmpObject = std::make_unique<BillBoard>("BillBoard");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
			break;
		case ObjectTag::EFFECT:
			tmpObject = std::make_unique<Explosion>("Explosion");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::EFFECT)].push_back(std::move(tmpObject));				//配列に追加
			break;
			break;
		case ObjectTag::TEXT:
			tmpObject = std::make_unique<TextObject>("TextObject1");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//配列に追加
			break;
		case ObjectTag::CAMERA:
			break;
		case ObjectTag::FADE:
			break;

    }

    //配列に追加

    return returnObject;
}

void ObjectManager::Uninit()
{
	
}

void ObjectManager::Init(Scene::ID _SceneID)
{
	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			object->Init();
		}
	}


}

void ObjectManager::Update(Scene::ID _SceneID)
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

	//削除処理
	for (auto& vec : vecObject)
	{
		// erase-removeイディオムを使うのが定石だよ
		vec.erase(
			std::remove_if(vec.begin(), vec.end(),
				[](const std::unique_ptr<CObject>& obj) {
					return obj->GetIsDestroyed(); // 削除フラグが立っているか判定
				}),
			vec.end()
		);
	}

	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			object->LateUpdate();
		}
	}
}

void ObjectManager::CollisionUpdate(Scene::ID _SceneID)
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

void ObjectManager::Draw(Scene::ID _SceneID)
{
	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			object->Draw();
		}
	}
}

ObjectManager::ObjectManager()
{
	//Resizeを掛ける
	vecObject.resize(ObjectTag::NUM);
}

ObjectManager::~ObjectManager()
{
}

