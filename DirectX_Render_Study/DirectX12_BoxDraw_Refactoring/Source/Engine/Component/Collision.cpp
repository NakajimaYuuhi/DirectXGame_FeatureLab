#include "Collision.h"
#include "Transform.h"
#include "Object.h"
#include <vector>

Collision::Collision()
{
	CollisionOrder = 
	{
		{ ObjectTag::PLAYER, ObjectTag::ENEMY_BULLET },
		{ ObjectTag::ENEMY, ObjectTag::PLAYER_BULLET },
		{ ObjectTag::PLAYER, ObjectTag::ENEMY },
	};
}

void Collision::ResolveCollisions(Vector<Vector<UniquePtr<CObject>>>& objectList)
{
	struct ColliderEntry {
		CObject* owner = nullptr;
		BoxCollider3D* collider = nullptr;
		CTransform* transform = nullptr;
	};
	std::vector<ColliderEntry> colliders;
	colliders.reserve(64);

	for (size_t tag = 0; tag < objectList.size(); ++tag)
	{
		for (auto& obj : objectList[tag])
		{
			if (!obj || obj->GetIsDestroyed()) continue;
			BoxCollider3D* col = obj->GetComponent<BoxCollider3D>();
			if (col && col->GetIsValid())
			{
				CTransform* trans = obj->GetComponent<CTransform>();
				colliders.push_back({ obj.get(), col, trans });
			}
		}
	}

	size_t count = colliders.size();
	if (count < 2) return;

	// Phase 1: Solid Resolution (Physical push apart for non-triggers)
	for (size_t i = 0; i < count; ++i)
	{
		auto& a = colliders[i];
		if (a.collider->GetIsTrigger() || !a.transform) continue;

		for (size_t j = i + 1; j < count; ++j)
		{
			auto& b = colliders[j];
			if (b.collider->GetIsTrigger() || !b.transform) continue;

			if (!a.collider->CanCollideWith(b.collider->GetLayer()) &&
				!b.collider->CanCollideWith(a.collider->GetLayer()))
			{
				continue;
			}

			DirectX::XMFLOAT3 pushVec = { 0.0f, 0.0f, 0.0f };
			if (a.collider->CalculateHorizontalPenetration(b.collider, pushVec))
			{
				DirectX::XMFLOAT3 posA = a.transform->GetPos();
				DirectX::XMFLOAT3 posB = b.transform->GetPos();

				posA.x += pushVec.x * 0.5f;
				posA.z += pushVec.z * 0.5f;

				posB.x -= pushVec.x * 0.5f;
				posB.z -= pushVec.z * 0.5f;

				a.transform->SetPos(posA);
				b.transform->SetPos(posB);
			}
		}
	}

	// Phase 2: Overlap Events (Trigger and Solid collision notifications)
	auto& collisionOrder = GetInstance().GetCollisionOrder();
	for (auto& order : collisionOrder)
	{
		int tagA = static_cast<int>(order[0]);
		int tagB = static_cast<int>(order[1]);

		if (tagA < 0 || tagA >= (int)objectList.size()) continue;
		if (tagB < 0 || tagB >= (int)objectList.size()) continue;

		for (size_t i = 0; i < objectList[tagA].size(); ++i)
		{
			for (size_t j = 0; j < objectList[tagB].size(); ++j)
			{
				CObject* objA = objectList[tagA][i].get();
				CObject* objB = objectList[tagB][j].get();

				if (!objA || !objB || objA->GetIsDestroyed() || objB->GetIsDestroyed()) continue;

				BoxCollider3D* colA = objA->GetComponent<BoxCollider3D>();
				BoxCollider3D* colB = objB->GetComponent<BoxCollider3D>();

				if (colA && colB && colA->GetIsValid() && colB->GetIsValid())
				{
					if (CheckCollision(colA, colB))
					{
						objA->OnCollision(objB);
						objB->OnCollision(objA);
					}
				}
			}
		}
	}
}
