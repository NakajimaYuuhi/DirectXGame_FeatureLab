#pragma once
#include <DirectXMath.h>


class CCollider_Ray;
class CCollider_Plane;
class CMesh;

class CCollision
{
public:
	//インスタンス取得
	static CCollision& GetInstance()
	{
		static CCollision m_Instance;
		return m_Instance;
	}

	//bool CheckCollision
	//(
	//	const CCollider_Ray& ray,
	//	const CCollider_Plane& plane,
	//	float& outT
	//);

	bool CheckCollision
	(
		const CCollider_Ray& ray,
		const CCollider_Plane& plane,
		float& outT,
		DirectX::XMFLOAT3& _CollisionPoint
	);

	//衝突点,オブジェクト
	bool CheckPointInQuad(DirectX::XMFLOAT3 _CollisionPoint, CMesh* _Mesh);


	//シングルトン実装
private:
	CCollision() = default;
	~CCollision() = default;

	CCollision(const CCollision&) = delete;
	CCollision& operator=(const CCollision&) = delete;
};

