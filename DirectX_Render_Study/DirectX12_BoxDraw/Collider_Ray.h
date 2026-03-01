#pragma once
#include <DirectXMath.h>
#include "Collider.h"

class CMesh;

class CCollider_Ray :public CCollider
{
public:
	CCollider_Ray(CMesh* _pMesh):CCollider(_pMesh)
	{
	}

	//レイの始点と方向を設定する関数
	void SetRay(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& direction)
	{
		m_Start = start;
		m_Direction = direction;
	}

	//Rayの更新
	void Update();

	//レイの始点を取得する関数
	const DirectX::XMFLOAT3& GetStart() const
	{
		return m_Start;
	}
	//レイの方向を取得する関数
	const DirectX::XMFLOAT3& GetDirection() const
	{
		return m_Direction;
	}

private:
	//Rayのオフセット
	DirectX::XMFLOAT3 m_OffsetPos = { 0.0f, 0.0f, 0.0f };
	
	//レイの始点
	DirectX::XMFLOAT3 m_Start = { 0.0f, 0.0f, 0.0f };
	//レイの方向
	DirectX::XMFLOAT3 m_Direction = { 0.0f, -1.0f, 0.0f };
};

