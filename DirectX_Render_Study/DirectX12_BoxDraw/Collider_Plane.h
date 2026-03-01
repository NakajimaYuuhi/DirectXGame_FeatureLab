#pragma once
#include <DirectXMath.h>
#include "Collider.h"

class CMesh;

class CCollider_Plane : public CCollider
{
public:

    CCollider_Plane(CMesh* _pMesh) : CCollider(_pMesh)
    {
	}


    //法線と位置を設定する関数
    void SetPlane(const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& pos)
    {
        m_Normal = normal;
        m_Pos = pos;
    }

    //法線を取得する関数
    //TODO:参照で返すと良い
    const DirectX::XMFLOAT3& GetNormal() const
    {
        return m_Normal;
    }
    //位置を取得する関数
    const DirectX::XMFLOAT3& GetPos() const
    {
        return m_Pos;
	}

    void SetNormal(const DirectX::XMFLOAT3& normal)
    {
        m_Normal = normal;
	}

    void SetPos(const DirectX::XMFLOAT3& pos)
    {
        m_Pos = pos;
	}

private:
    DirectX::XMFLOAT3 m_Normal    = {0.0f, 1.0f, 0.0f};
    DirectX::XMFLOAT3 m_Pos       = {0.0f, 0.0f, 0.0f};

};

