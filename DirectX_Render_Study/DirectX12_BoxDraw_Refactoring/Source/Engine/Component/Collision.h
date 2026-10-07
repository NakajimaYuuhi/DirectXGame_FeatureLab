#pragma once
#include "BoxCollider3D.h"
#include "CapsuleCollider3D.h"
#include "ObjectTag.h"
#include "ContainerAlias.h"
#include <algorithm>

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

	// Solver iterations configuration (default: 3)
	int GetSolverIterations() const { return m_solverIterations; }
	void SetSolverIterations(int iterations) { m_solverIterations = (std::max)(1, iterations); }

	// Two-phase collision pipeline:
	// Phase 1: Solid Resolution Solver (iterative push apart for !isTrigger objects)
	// Phase 2: Trigger / Overlap Notification (evaluates at final confirmed positions)
	static void ResolveCollisions(Vector<Vector<UniquePtr<CObject>>>& objectList);

private:
	int m_solverIterations = 3;

public:
	static Collision& GetInstance()
	{
		static Collision Instance;
		return Instance;
	}

private:
	Collision() = default;
	~Collision() = default;

	Collision(const Collision&) = delete;
	Collision& operator=(const Collision&) = delete;
};
