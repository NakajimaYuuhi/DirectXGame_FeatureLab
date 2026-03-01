#pragma once

class CCollider_Ray;
class CCollider_Plane;

class CCollision
{
public:
	//インスタンス取得
	static CCollision& GetInstance()
	{
		static CCollision m_Instance;
		return m_Instance;
	}

	bool CheckCollision
	(
		const CCollider_Ray& ray,
		const CCollider_Plane& plane,
		float& outT
	);

	//シングルトン実装
private:
	CCollision() = default;
	~CCollision() = default;

	CCollision(const CCollision&) = delete;
	CCollision& operator=(const CCollision&) = delete;
};

