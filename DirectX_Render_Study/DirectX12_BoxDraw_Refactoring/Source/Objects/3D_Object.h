//3D_Object.h
//3D描画するオブジェクトで使用する
//TransformとかMeshをデフォルトで持っておくようにする

//===== インクルード =====
#pragma once
#include "Object.h"
#include <DirectXMath.h>

#include "StringAlias.h"
#include "Transform.h"


//===== クラスの定義 =====
class C3D_Object : public CObject
{
public:
    C3D_Object();
    C3D_Object(String _Name);

    virtual ~C3D_Object()
    {

    }

    virtual void Update();
    virtual void LateUpdate();
    virtual void Draw();

private:

    //----- Getter,Setter -----
public:
    
    //理想は外部データで持っていること
    //--- 初期化用 ---
    //Transform
    void SetTransform(DirectX::XMFLOAT3 _Position, DirectX::XMFLOAT3 _Scale, DirectX::XMFLOAT3 _Rotation);
    //Mesh
    //void SetMesh(int MeterialNum);
    //Material
    //void SetMaterial(wstring _TexturePath, XMFloat4 Color);

    //--- 利便性のため追加 ---
    DirectX::XMFLOAT3 GetPos();
    void	SetPos(DirectX::XMFLOAT3 _Position);

    DirectX::XMFLOAT3	GetScale();
    void	SetScale(DirectX::XMFLOAT3 _Scale);

    DirectX::XMFLOAT3	GetRotation();
    void	SetRotation(DirectX::XMFLOAT3 _Rotation);

	//Front,Right,Upのベクトル取得
	DirectX::XMFLOAT3 GetFront()
    {
        CTransform* transform = GetComponent<CTransform>();
        return transform->GetFront();
	}
    DirectX::XMFLOAT3 GetRight() {
        CTransform* transform = GetComponent<CTransform>();
		return transform->GetRight();
    }
    DirectX::XMFLOAT3 GetUp() {
        CTransform* transform = GetComponent<CTransform>();
		return transform->GetUp();
    }


	
};

