#include "Collision.h"
#include "Transform.h"
#include "Object.h"
#include <vector>

bool Collision::CheckCollision(Collider3D* a, Collider3D* b)
{
	if (!a || !b) return false;

	auto* boxA = dynamic_cast<BoxCollider3D*>(a);
	auto* capA = dynamic_cast<CapsuleCollider3D*>(a);
	auto* boxB = dynamic_cast<BoxCollider3D*>(b);
	auto* capB = dynamic_cast<CapsuleCollider3D*>(b);

	if (boxA && boxB) return CheckCollision(boxA, boxB);
	if (capA && capB) return CheckCollision(capA, capB);
	if (capA && boxB) return CheckCollision(capA, boxB);
	if (boxA && capB) return CheckCollision(boxA, capB);

	return false;
}

bool Collision::CalculateHorizontalPenetration(Collider3D* a, Collider3D* b, DirectX::XMFLOAT3& outPushVector)
{
	if (!a || !b) return false;

	auto* boxA = dynamic_cast<BoxCollider3D*>(a);
	auto* capA = dynamic_cast<CapsuleCollider3D*>(a);
	auto* boxB = dynamic_cast<BoxCollider3D*>(b);
	auto* capB = dynamic_cast<CapsuleCollider3D*>(b);

	if (boxA && boxB) return boxA->CalculateHorizontalPenetration(boxB, outPushVector);
	if (capA && capB) return capA->CalculateHorizontalPenetration(capB, outPushVector);
	if (capA && boxB) return capA->CalculateHorizontalPenetration(boxB, outPushVector);
	if (boxA && capB) return boxA->CalculateHorizontalPenetration(boxB, outPushVector);

	return false;
}

bool Collision::CalculatePenetration(Collider3D* a, Collider3D* b, DirectX::XMFLOAT3& outPushVector)
{
	if (!a || !b) return false;

	auto* boxA = dynamic_cast<BoxCollider3D*>(a);
	auto* capA = dynamic_cast<CapsuleCollider3D*>(a);
	auto* boxB = dynamic_cast<BoxCollider3D*>(b);
	auto* capB = dynamic_cast<CapsuleCollider3D*>(b);

	if (boxA && boxB) return boxA->CalculatePenetration(boxB, outPushVector);
	if (capA && capB) return capA->CalculatePenetration(capB, outPushVector);
	if (capA && boxB) return capA->CalculatePenetration(boxB, outPushVector);
	if (boxA && capB) return boxA->CalculatePenetration(boxB, outPushVector);

	return false;
}

static Collider3D* GetColliderFromObject(CObject* obj)
{
	if (!obj) return nullptr;
	Collider3D* col = obj->GetComponent<CapsuleCollider3D>();
	if (!col) col = obj->GetComponent<BoxCollider3D>();
	return col;
}

void Collision::ResolveCollisions(Vector<Vector<UniquePtr<CObject>>>& objectList)
{
	// -------------------------------------------------------------------------
	// Phase 0: Collect all active 3D colliders from scene objects
	// -------------------------------------------------------------------------
	struct ColliderEntry {
		CObject*    owner = nullptr;
		Collider3D* collider = nullptr;
		CTransform* transform = nullptr;
		bool        isStatic = false;
	};
	std::vector<ColliderEntry> colliders;
	colliders.reserve(64);

	for (size_t tag = 0; tag < objectList.size(); ++tag)
	{
		for (auto& obj : objectList[tag])
		{
			if (!obj || obj->GetIsDestroyed()) continue;
			Collider3D* col = GetColliderFromObject(obj.get());
			if (col && col->GetIsValid())
			{
				CTransform* trans = obj->GetComponent<CTransform>();
				uint32_t layer = col->GetLayer();
				bool isStatic = (layer & (CollisionLayer::Terrain | CollisionLayer::Obstacle)) != 0;
				colliders.push_back({ obj.get(), col, trans, isStatic });
			}
		}
	}

	size_t count = colliders.size();
	if (count < 2) return;

	// -------------------------------------------------------------------------
	// Phase 1: Solid Resolution Solver (Iterative push-apart for solid objects)
	// Solves penetration for non-trigger objects (!isTrigger) across multiple
	// iterations to completely resolve multi-body overlap and establish final
	// confirmed coordinates before any trigger/damage logic executes.
	// -------------------------------------------------------------------------
	int iterations = GetInstance().GetSolverIterations();

	for (int iter = 0; iter < iterations; ++iter)
	{
		bool anyPenetrationResolved = false;

		for (size_t i = 0; i < count; ++i)
		{
			auto& a = colliders[i];
			if (a.collider->GetIsTrigger() || !a.transform) continue;

			for (size_t j = i + 1; j < count; ++j)
			{
				auto& b = colliders[j];
				if (b.collider->GetIsTrigger() || !b.transform) continue;

				// Skip if both objects are static (e.g. wall vs terrain)
				if (a.isStatic && b.isStatic) continue;

				// Layer bitmask collision check
				if (!a.collider->CanCollideWith(b.collider->GetLayer()) &&
					!b.collider->CanCollideWith(a.collider->GetLayer()))
				{
					continue;
				}

				DirectX::XMFLOAT3 pushVec = { 0.0f, 0.0f, 0.0f };
				if (CalculateHorizontalPenetration(a.collider, b.collider, pushVec))
				{
					anyPenetrationResolved = true;
					DirectX::XMFLOAT3 posA = a.transform->GetPos();
					DirectX::XMFLOAT3 posB = b.transform->GetPos();

					if (!a.isStatic && !b.isStatic)
					{
						// Both dynamic: distribute displacement equally (50% / 50%)
						posA.x += pushVec.x * 0.5f;
						posA.z += pushVec.z * 0.5f;
						posB.x -= pushVec.x * 0.5f;
						posB.z -= pushVec.z * 0.5f;
					}
					else if (!a.isStatic && b.isStatic)
					{
						// 'a' is dynamic, 'b' is static: push 'a' 100% away from 'b'
						posA.x += pushVec.x;
						posA.z += pushVec.z;
					}
					else if (a.isStatic && !b.isStatic)
					{
						// 'a' is static, 'b' is dynamic: push 'b' 100% away from 'a'
						posB.x -= pushVec.x;
						posB.z -= pushVec.z;
					}

					a.transform->SetPos(posA);
					b.transform->SetPos(posB);
				}
			}
		}

		// If no collisions were found in this iteration, early exit to save CPU cycles
		if (!anyPenetrationResolved) break;
	}

	// -------------------------------------------------------------------------
	// Phase 2: Trigger / Overlap Notification (Events evaluated at final positions)
	// Evaluates intersection with the confirmed positions. If overlapping,
	// dispatches OnCollision(other) events to trigger damage, pickups, etc.
	// -------------------------------------------------------------------------
	for (size_t i = 0; i < count; ++i)
	{
		auto& a = colliders[i];
		if (!a.owner || a.owner->GetIsDestroyed()) continue;

		for (size_t j = i + 1; j < count; ++j)
		{
			auto& b = colliders[j];
			if (!b.owner || b.owner->GetIsDestroyed()) continue;

			// Layer bitmask collision check
			if (!a.collider->CanCollideWith(b.collider->GetLayer()) &&
				!b.collider->CanCollideWith(a.collider->GetLayer()))
			{
				continue;
			}

			if (CheckCollision(a.collider, b.collider))
			{
				a.owner->OnCollision(b.owner);
				b.owner->OnCollision(a.owner);
			}
		}
	}
}
