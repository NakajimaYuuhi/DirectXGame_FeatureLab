#include "TimeManager.h"
#include "ObjectManager.h"
#include "Collision.h"
#include "InspectorUI.h"
#include "Model.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "Source/Util/Profiler.h"
#include "Source/Util/TweenManager.h"

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
	TweenManager::GetInstance().KillAll();
	m_pendingAddObjects.clear();
	vecObject.clear();
	vecObject.resize(ObjectTag::NUM);
}

void ObjectManager::Update(Scenes::ID _SceneID)
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	// 1. フレーム開始時の保留オブジェクト反映（常に実行）
	FlushPendingAddObjects();

	// 2. Tween アニメーションの更新（UI演出・エディタ操作等を含むため常に実行）
	{
		PROFILE_SCOPE("Update::Tween");
		TweenManager::GetInstance().Update(dt);
	}

	if (!CInspectorUI::GetInstance().ShouldUpdateGame())
	{
		return;
	}

	// 3. 未実行オブジェクト・コンポーネントの初期化（Awake / Start）を一括確定
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
			}
		}
	}

	// 3. Phase 0: Input (全オブジェクトの入力受付・操作)
	{
		PROFILE_SCOPE("Update::Input");
		UpdatePhaseAll(UpdatePhase::Input, dt);
	}

	// 4. Phase 1: AI (全エネミーの思考・状態遷移)
	{
		PROFILE_SCOPE("Update::AI");
		UpdatePhaseAll(UpdatePhase::AI, dt);
	}

	// 5. Phase 2: Movement (全キャラの移動・速度計算)
	{
		PROFILE_SCOPE("Update::Movement");
		UpdatePhaseAll(UpdatePhase::Movement, dt);
	}

	// 6. Phase 3: Physics (重力適用、外力計算など)
	{
		PROFILE_SCOPE("Update::Physics");
		UpdatePhaseAll(UpdatePhase::Physics, dt);
	}

	// 7. 衝突判定・押し戻し解決（全オブジェクトの位置が物理的に確定）
	{
		PROFILE_SCOPE("Update::Collision");
		CollisionUpdate(_SceneID);
	}

	// 8. Phase 4: Animation (確定した移動・姿勢に基づくボーン更新・UVアニメ)
	{
		PROFILE_SCOPE("Update::Animation");
		UpdatePhaseAll(UpdatePhase::Animation, dt);
	}

	// 9. Phase 5: PostPhysics (押し戻し確定後のプレイヤー位置をカメラが追従・ビルボード)
	{
		PROFILE_SCOPE("Update::PostPhysics");
		UpdatePhaseAll(UpdatePhase::PostPhysics, dt);
	}

	// 10. LateUpdate (全オブジェクトの最終補正)
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

	// 11. フレーム終了時の保留オブジェクト反映 & 破棄オブジェクトの安全な回収
	FlushPendingAddObjects();
	FlushDestroyedObjects();
}

void ObjectManager::UpdatePhaseAll(UpdatePhase phase, float deltaTime)
{
	for (size_t tagIdx = 0; tagIdx < vecObject.size(); ++tagIdx)
	{
		auto& vec = vecObject[tagIdx];
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto& object = vec[i];
			if (object && !object->GetIsDestroyed())
			{
				object->UpdateComponentsByPhase(phase, deltaTime);
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
	FlushPendingAddObjects();

	// レイヤー順に整列描画（不透明 -> 透過 -> デバッグ -> UI）
	{
		PROFILE_SCOPE("Render::Opaque");
		DrawByLayer(RenderLayer::Opaque);
	}
	{
		PROFILE_SCOPE("Render::Transparent");
		DrawByLayer(RenderLayer::Transparent);
	}
	{
		PROFILE_SCOPE("Render::Debug");
		DrawByLayer(RenderLayer::Debug);
	}
	{
		PROFILE_SCOPE("Render::UI");
		DrawByLayer(RenderLayer::UI);
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
			if (object && !object->GetIsDestroyed())
			{
				object->DrawByLayer(layer);
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
