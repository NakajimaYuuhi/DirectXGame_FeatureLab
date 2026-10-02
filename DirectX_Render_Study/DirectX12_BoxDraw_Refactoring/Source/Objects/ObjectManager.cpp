#include "ObjectManager.h"
#include "Collision.h"
#include "InspectorUI.h"
#include "Model.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"

void ObjectManager::Init(Scenes::ID _SceneID)
{
	FlushPendingAddObjects();
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
	m_pendingAddObjects.clear();
	vecObject.clear();
	vecObject.resize(ObjectTag::NUM);
}

void ObjectManager::Update(Scenes::ID _SceneID)
{
	if (!CInspectorUI::GetInstance().ShouldUpdateGame())
	{
		return;
	}

	// Update前に待機中のオブジェクトを追加
	FlushPendingAddObjects();

	for (size_t tagIdx = 0; tagIdx < vecObject.size(); ++tagIdx)
	{
		auto& vec = vecObject[tagIdx];
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto& object = vec[i];
			if (object && !object->GetIsDestroyed())
			{
				if (!object->GetHasAwoken()) object->Awake();
				if (!object->GetHasStarted()) object->Start();
				object->Update();
			}
		}
	}

	// Update中に生成されたオブジェクトを反映
	FlushPendingAddObjects();

	CollisionUpdate(_SceneID);

	// 衝突判定中に生成されたオブジェクトを反映
	FlushPendingAddObjects();

	for (size_t tagIdx = 0; tagIdx < vecObject.size(); ++tagIdx)
	{
		auto& vec = vecObject[tagIdx];
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto& object = vec[i];
			if (object && !object->GetIsDestroyed())
			{
				object->LateUpdate();
			}
		}
	}

	// LateUpdate中に生成されたオブジェクトを反映
	FlushPendingAddObjects();
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
	FlushPendingAddObjects();
	for (size_t tagIdx = 0; tagIdx < vecObject.size(); ++tagIdx)
	{
		auto& vec = vecObject[tagIdx];
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto& object = vec[i];
			if (object && !object->GetIsDestroyed())
			{
				object->Draw();
			}
		}
	}
}

void ObjectManager::DrawByLayer(RenderLayer layer)
{
	for (size_t tagIdx = 0; tagIdx < vecObject.size(); ++tagIdx)
	{
		auto& vec = vecObject[tagIdx];
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto& object = vec[i];
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