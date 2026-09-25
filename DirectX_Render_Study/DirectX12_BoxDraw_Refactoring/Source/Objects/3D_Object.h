//3D_Object.h
//3D?`????I?u?W?F?N?g??g?p????
//Transform???Mesh??f?t?H???g?????????????????

//===== ?C???N???[?h =====
#pragma once
#include "Object.h"
#include <DirectXMath.h>

#include "StringAlias.h"
#include "Transform.h"


//===== ?N???X???` =====
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
    
    //???z??O???f?[?^?????????Á∑??
    //--- ???????p ---
    //Transform
    void SetTransform(DirectX::XMFLOAT3 _Position, DirectX::XMFLOAT3 _Scale, DirectX::XMFLOAT3 _Rotation);
    //Mesh
    //void SetMesh(int MeterialNum);
    //Material
    //void SetMaterial(wstring _TexturePath, XMFloat4 Color);

    //--- ???????????? ---
    DirectX::XMFLOAT3 GetPos();
    void	SetPos(DirectX::XMFLOAT3 _Position);

    DirectX::XMFLOAT3	GetScale();
    void	SetScale(DirectX::XMFLOAT3 _Scale);

    DirectX::XMFLOAT3	GetRotation();
    void	SetRotation(DirectX::XMFLOAT3 _Rotation);

	//Front,Right,Up??x?N?g???èÔ
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

