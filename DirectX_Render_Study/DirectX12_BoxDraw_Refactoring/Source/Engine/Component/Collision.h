#pragma once
#include "BoxCollider3D.h"
#include "CapsuleCollider3D.h"
#include "ObjectTag.h"
#include "ContainerAlias.h"

class Collision
{
public:
	// Intersection checks (Box vs Box, Capsule vs Capsule, Capsule vs Box)
	static bool CheckCollision(BoxCollider3D* a, BoxCollider3D* b)
	{
		if (!a || !b) return false;
		return a->CheckCollision(b);
	}

	static bool CheckCollision(CapsuleCollider3D* a, CapsuleCollider3D* b)
	{
		if (!a || !b) return false;
		return a->CheckCollision(b);
	}

	static bool CheckCollision(CapsuleCollider3D* a, BoxCollider3D* b)
	{
		if (!a || !b) return false;
		return a->CheckCollision(b);
	}

	static bool CheckCollision(BoxCollider3D* a, CapsuleCollider3D* b)
	{
		if (!a || !b) return false;
		return a->CheckCollision(b);
	}

	// Dynamic dispatch for base Collider3D pointers
	static bool CheckCollision(Collider3D* a, Collider3D* b);

	// Penetration depth and separation vector dispatch
	static bool CalculateHorizontalPenetration(Collider3D* a, Collider3D* b, DirectX::XMFLOAT3& outPushVector);
	static bool CalculatePenetration(Collider3D* a, Collider3D* b, DirectX::XMFLOAT3& outPushVector);

	Vector<Vector<ObjectTag>>& GetCollisionOrder() { return CollisionOrder; }

	// Two-phase collision pipeline:
	// Phase 1: Solid Resolution (!isTrigger vs !isTrigger) - pushes objects apart
	// Phase 2: Trigger / Overlap Notification - calls OnCollision
	static void ResolveCollisions(Vector<Vector<UniquePtr<CObject>>>& objectList);

private:
	Vector<Vector<ObjectTag>> CollisionOrder;

public:
	static Collision& GetInstance()
	{
		static Collision Instance;
		return Instance;
	}

private:
	Collision();
	~Collision() = default;

	Collision(const Collision&) = delete;
	Collision& operator=(const Collision&) = delete;
};
