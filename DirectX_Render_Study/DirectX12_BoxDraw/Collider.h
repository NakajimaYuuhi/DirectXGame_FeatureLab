#pragma once

class CMesh;

class CCollider
{
public:
	CCollider(CMesh* _pMesh);

	virtual void Update() {}

protected:
	//コライダーを持っているメッシュ
	CMesh* m_pMesh = nullptr;
};

