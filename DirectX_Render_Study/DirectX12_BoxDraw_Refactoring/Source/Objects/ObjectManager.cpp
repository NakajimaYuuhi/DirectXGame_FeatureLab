#include "ObjectManager.h"
#include "Collision.h"
#include "InspectorUI.h"
#include "Model.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"

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
	Collision::ResolveCollisions(vecObject);
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

void ObjectManager::DrawByLayer(RenderLayer layer)
{
	for (auto& vec : vecObject)
	{
		for (auto& object : vec)
		{
			if (!object || object->GetIsDestroyed() || !object->GetIsVisible()) continue;

			CModel* model = object->GetComponent<CModel>();
			if (model && model->GetRenderLayer() == layer)
			{
				object->Draw();
				continue;
			}

			CSpriteRenderer* sprite = object->GetComponent<CSpriteRenderer>();
			if (sprite && sprite->GetRenderLayer() == layer)
			{
				object->Draw();
				continue;
			}

			CTextRenderer* text = object->GetComponent<CTextRenderer>();
			if (text && text->GetRenderLayer() == layer)
			{
				object->Draw();
				continue;
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