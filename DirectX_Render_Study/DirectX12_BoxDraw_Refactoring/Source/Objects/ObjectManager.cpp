#include "ObjectManager.h"
#include "Collision.h"
#include "InspectorUI.h"

void ObjectManager::Init(Scenes::ID _SceneID)
{
	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			if (object)
			{
				object->Init();
			}
		}
	}
}

void ObjectManager::Uninit()
{
	vecObject.clear();
	vecObject.resize(ObjectTag::NUM);
}

void ObjectManager::Update(Scenes::ID _SceneID)
{
	if (!CInspectorUI::GetInstance().ShouldUpdateGame())
	{
		return;
	}

	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			if (object && !object->GetIsDestroyed())
			{
				if (!object->GetHasAwoken()) object->Awake();
				if (!object->GetHasStarted()) object->Start();
				object->Update();
			}
		}
	}

	CollisionUpdate(_SceneID);

	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			if (object && !object->GetIsDestroyed())
			{
				object->LateUpdate();
			}
		}
	}
}

void ObjectManager::FlushDestroyedObjects()
{
	for (auto& vec : vecObject)
	{
		vec.erase(
			std::remove_if(vec.begin(), vec.end(),
				[](const std::unique_ptr<CObject>& obj) {
					return !obj || obj->GetIsDestroyed();
				}),
			vec.end()
		);
	}
}

void ObjectManager::CollisionUpdate(Scenes::ID _SceneID)
{
	Vector<Vector<ObjectTag>>& CollisionOrder = Collision::GetInstance().GetCollisionOrder();

	for (auto& order : CollisionOrder)
	{
		int tagA = static_cast<int>(order[0]);
		int tagB = static_cast<int>(order[1]);

		for (size_t i = 0; i < vecObject[tagA].size(); i++)
		{
			for (size_t j = 0; j < vecObject[tagB].size(); j++)
			{
				CObject* objA = vecObject[tagA][i].get();
				CObject* objB = vecObject[tagB][j].get();

				if (!objA || !objB || objA->GetIsDestroyed() || objB->GetIsDestroyed()) continue;

				BoxCollider3D* colliderA = objA->GetComponent<BoxCollider3D>();
				BoxCollider3D* colliderB = objB->GetComponent<BoxCollider3D>();

				if (colliderA && colliderB)
				{
					if (Collision::CheckCollision(colliderA, colliderB))
					{
						objA->OnCollision(objB);
						objB->OnCollision(objA);
					}
				}
			}
		}
	}
}

void ObjectManager::Draw(Scenes::ID _SceneID)
{
	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			if (object && !object->GetIsDestroyed())
			{
				object->Draw();
			}
		}
	}
}

ObjectManager::ObjectManager()
{
	vecObject.resize(ObjectTag::NUM);
}

ObjectManager::~ObjectManager()
{
}