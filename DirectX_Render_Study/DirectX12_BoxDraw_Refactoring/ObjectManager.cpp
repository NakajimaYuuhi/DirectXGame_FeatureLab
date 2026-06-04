#include "ObjectManager.h"

#include "3D_Object.h"
#include "Player.h"
#include "Bullet.h"

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
            break;
		case ObjectTag::ENEMY_BULLET:
			break;
		case ObjectTag::FIELD:
			//Floor

			break;
		case ObjectTag::EFFECT:
			break;
		case ObjectTag::UI:
			break;
		case ObjectTag::TEXT:
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
