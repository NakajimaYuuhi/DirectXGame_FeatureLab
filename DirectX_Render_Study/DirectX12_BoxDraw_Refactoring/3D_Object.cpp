//===== インクルード =====
#include "3D_Object.h"

//Transform
#include "Transform.h"
//Mesh
#include "Mesh.h"

C3D_Object::C3D_Object()
	:CObject()
{
	//Transformコンポーネントの作成
	AddComponent<CTransform>();

	//Meshもここ？
	AddComponent<CMesh>();

}

C3D_Object::C3D_Object(String _Name)
	:C3D_Object()
{
	SetName(_Name);
}

void C3D_Object::Update() 
{

	//CTransform* transform = GetComponent<CTransform>();

	//transform->Update();

	CMesh* mesh = GetComponent<CMesh>();

	mesh->Update();

}

void C3D_Object::Draw() 
{
	CMesh* mesh = GetComponent<CMesh>();

	mesh->Draw();
}

//Transform
void C3D_Object::SetTransform(DirectX::XMFLOAT3 _Position, DirectX::XMFLOAT3 _Scale, DirectX::XMFLOAT3 _Rotation)
{
	CTransform* transform = GetComponent<CTransform>();
	transform->SetPos(_Position);
	transform->SetScale(_Scale);
	transform->SetRotation(_Rotation);
}

//Pos
DirectX::XMFLOAT3 C3D_Object::GetPos() 
{ 
	CTransform* transform = GetComponent<CTransform>();
	return transform->GetPos();
}

void C3D_Object::SetPos(DirectX::XMFLOAT3 _Position)
{ 
	CTransform* transform = GetComponent<CTransform>();
	transform->SetPos(_Position);
}

//Scale
DirectX::XMFLOAT3 C3D_Object::GetScale() 
{ 
	CTransform* transform = GetComponent<CTransform>();
	return transform->GetScale();
}

void C3D_Object::SetScale(DirectX::XMFLOAT3 _Scale)
{
	CTransform* transform = GetComponent<CTransform>();
	transform->SetScale(_Scale);
}

//Rotation
DirectX::XMFLOAT3 C3D_Object::GetRotation() 
{ 
	CTransform* transform = GetComponent<CTransform>();
	return transform->GetRotation();
}

void C3D_Object::SetRotation(DirectX::XMFLOAT3 _Rotation)
{ 
	CTransform* transform = GetComponent<CTransform>();
	transform->SetRotation(_Rotation);
}
