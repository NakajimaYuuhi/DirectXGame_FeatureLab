#pragma once
#include "BoxCollider3D.h"
#include "ObjectTag.h"
#include "ContainerAlias.h"

class Collision
{
public:

	//衝突判定
	static bool CheckCollision(BoxCollider3D* colliderA, BoxCollider3D* colliderB)
	{
		DirectX::XMFLOAT3 posA = colliderA->GetWorldPos();
		DirectX::XMFLOAT3 sizeA = colliderA->GetSize();
		DirectX::XMFLOAT3 posB = colliderB->GetWorldPos();
		DirectX::XMFLOAT3 sizeB = colliderB->GetSize();
		// AABB同士の衝突判定
		if (abs(posA.x - posB.x) < (sizeA.x / 2 + sizeB.x / 2) &&
			abs(posA.y - posB.y) < (sizeA.y / 2 + sizeB.y / 2) &&
			abs(posA.z - posB.z) < (sizeA.z / 2 + sizeB.z / 2))
		{
			return true; // 衝突している
		}
		return false; // 衝突していない
	}

	Vector<Vector<ObjectTag>>& GetCollisionOrder() { return CollisionOrder; }

private:
	//Collisionの順番
	Vector<Vector<ObjectTag>> CollisionOrder;

	//----- シングルトンの実装に必要 -----
public:
	static Collision& GetInstance()
	{
		static Collision Instance;

		//インスタンスを返す
		return Instance;
	}

private:
	//コンストラクタ
	Collision();

	//デストラクタ
	~Collision(){}

	//コピー禁止
	Collision(const Collision&) = delete;

	//代入禁止
	Collision& operator=(const Collision&) = delete;
};

