//3D_Object.h
//3D描画するオブジェクトで使用する
//TransformとかMeshをデフォルトで持っておくようにする

//===== インクルード =====
#pragma once
#include "Object.h"
#include <string>
#include <DirectXMath.h>

//string
using String = std::string;

//===== クラスの定義 =====
class C3D_Object : public CObject
{
public:
    C3D_Object();
    C3D_Object(String _Name);

    ~C3D_Object()
    {

    }

    virtual void Update();
    virtual void Draw();

private:

    //----- Getter,Setter -----
public:
    void SetTransform(DirectX::XMFLOAT3 _Position, DirectX::XMFLOAT3 _Scale, DirectX::XMFLOAT3 _Rotation);

    DirectX::XMFLOAT3 GetPos();
    void	SetPos(DirectX::XMFLOAT3 _Position);

    DirectX::XMFLOAT3	GetScale();
    void	SetScale(DirectX::XMFLOAT3 _Scale);

    DirectX::XMFLOAT3	GetRotation();
    void	SetRotation(DirectX::XMFLOAT3 _Rotation);
};

