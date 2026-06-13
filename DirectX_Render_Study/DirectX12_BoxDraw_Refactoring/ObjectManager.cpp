#include "ObjectManager.h"

#include "3D_Object.h"
#include "Skydome.h"
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
    //Todo : Factory繧剃ｽ懊ｋ

    //逕滓・
    //Map縺ｨ縺九〒縺阪ｌ縺・↓蛻・ｲ舌＆縺帙◆縺・
	std::unique_ptr<CObject> tmpObject = std::unique_ptr<CObject>(nullptr);
	CObject* returnObject = nullptr;


    switch (_Tag)
    {
		case ObjectTag::NONE:
			tmpObject = std::make_unique<C3D_Object>("3DObject");
			returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
			vecObject[static_cast<int>(ObjectTag::FIELD)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉

			break;

        case ObjectTag::BACKGROUND:
			if (_TypeName == "Skydome")
			{
				tmpObject = std::make_unique<Skydome>("Skydome");
				returnObject = tmpObject.get();
				vecObject[static_cast<int>(ObjectTag::BACKGROUND)].push_back(std::move(tmpObject));
			}
			break;
		case ObjectTag::UI:
			tmpObject = std::make_unique<CUIObject>(_TypeName);
			returnObject = tmpObject.get();
			vecObject[static_cast<int>(ObjectTag::UI)].push_back(std::move(tmpObject));
			break;
		case ObjectTag::PLAYER:
			tmpObject = std::make_unique<Player>("Player");		//逕滓・
			returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
			vecObject[static_cast<int>(ObjectTag::PLAYER)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉
			break;
		case ObjectTag::PLAYER_BULLET:



			tmpObject = std::make_unique<Bullet>("Bullet");		//逕滓・
			returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
			vecObject[static_cast<int>(ObjectTag::PLAYER_BULLET)].push_back(std::move(tmpObject));		//驟榊・縺ｫ霑ｽ蜉
			break;
		case ObjectTag::ENEMY:
			tmpObject = std::make_unique<Enemy>("Enemy");		//逕滓・
			returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
			vecObject[static_cast<int>(ObjectTag::ENEMY)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉
            break;
		case ObjectTag::ENEMY_BULLET:
			break;
		case ObjectTag::FIELD:
			//Floor

			break;
		case ObjectTag::BILLBOARD:
			if (_TypeName == "RandomParticle")
			{
				tmpObject = std::make_unique<RandomParticle>("Particle");		//逕滓・
				returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
				vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉
				break;
			}
			else if (_TypeName == "Explosion")
			{
				tmpObject = std::make_unique<Explosion>("Explosion");		//逕滓・
				returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
				vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉
				break;
			}


			tmpObject = std::make_unique<BillBoard>("BillBoard");		//逕滓・
			returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉
			break;
		case ObjectTag::EFFECT:
			tmpObject = std::make_unique<Explosion>("Explosion");		//逕滓・
			returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
			vecObject[static_cast<int>(ObjectTag::EFFECT)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉
			break;
			break;
		case ObjectTag::TEXT:
			tmpObject = std::make_unique<TextObject>("TextObject1");		//逕滓・
			returnObject = tmpObject.get();							//逕溘・繧､繝ｳ繧ｿ蜿門ｾ・
			vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//驟榊・縺ｫ霑ｽ蜉
			break;
		case ObjectTag::CAMERA:
			break;
		case ObjectTag::FADE:
			break;

    }

    //驟榊・縺ｫ霑ｽ蜉

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

	//Collision縺ｮ譖ｴ譁ｰ
	CollisionUpdate(_SceneID);

	//蜑企勁蜃ｦ逅・
	for (auto& vec : vecObject)
	{
		// erase-remove繧､繝・ぅ繧ｪ繝繧剃ｽｿ縺・・縺悟ｮ夂浹縺繧・
		vec.erase(
			std::remove_if(vec.begin(), vec.end(),
				[](const std::unique_ptr<CObject>& obj) {
					return obj->GetIsDestroyed(); // 蜑企勁繝輔Λ繧ｰ縺檎ｫ九▲縺ｦ縺・ｋ縺句愛螳・
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
	//Collision縺ｮ譖ｴ譁ｰ
	//CollisionOrder繧貞叙蠕・
	Vector<Vector<ObjectTag>>& CollisionOrder = Collision::GetInstance().GetCollisionOrder();

	//CollisionOrder縺ｮ鬆・分縺ｧ陦晉ｪ∝愛螳・
	for (auto& order : CollisionOrder)
	{
		//order縺ｮ鬆・分縺ｧ陦晉ｪ∝愛螳・
		for (size_t i = 0; i < vecObject[static_cast<int>(order[0])].size(); i++)
		{
			for (size_t j = 0; j < vecObject[static_cast<int>(order[1])].size(); j++)
			{
				//陦晉ｪ∝愛螳・
				BoxCollider3D* colliderA = vecObject[static_cast<int>(order[0])][i]->GetComponent<BoxCollider3D>();
				BoxCollider3D* colliderB = vecObject[static_cast<int>(order[1])][j]->GetComponent<BoxCollider3D>();
				if (colliderA && colliderB)
				{
					if (Collision::CheckCollision(colliderA, colliderB))
					{
						//陦晉ｪ√＠縺ｦ縺・ｋ縺ｨ縺阪・蜃ｦ逅・
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
	//Resize繧呈寺縺代ｋ
	vecObject.resize(ObjectTag::NUM);
}

ObjectManager::~ObjectManager()
{
}

