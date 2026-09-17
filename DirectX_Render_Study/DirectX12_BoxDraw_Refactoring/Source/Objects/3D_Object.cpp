//===== インクルード =====
#include "3D_Object.h"

//Transform
#include "Transform.h"
//Mesh
#include "Mesh.h"

//Model
#include "Model.h"

#include "TimeManager.h"

C3D_Object::C3D_Object()
	:CObject()
{
	AddComponent<CTransform>();
	AddComponent<CModel>();
}

C3D_Object::C3D_Object(String _Name)
	:C3D_Object()
{
	SetName(_Name);
}

void C3D_Object::Update() 
{
	CModel* model = GetComponent<CModel>();
	if (model)
	{
		model->Update();
	}

	float dt = TimeManager::GetInstance().GetDeltaTime();
	UpdateComponents(dt);
}

void C3D_Object::LateUpdate()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();
	LateUpdateComponents(dt);
}

void C3D_Object::Draw() 
{
	if (!m_isVisible) return;

	CModel* model = GetComponent<CModel>();

	if (model)
	{
		model->Draw();
	}
}

//Transform
void C3D_Object::SetTransform(DirectX::XMFLOAT3 _Position, DirectX::XMFLOAT3 _Scale, DirectX::XMFLOAT3 _Rotation)
{
	CTransform* transform = GetComponent<CTransform>();
	transform->SetPos(_Position);
	transform->SetScale(_Scale);
	transform->SetRotation(_Rotation);
}


//===== Getter,Setter =====

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
