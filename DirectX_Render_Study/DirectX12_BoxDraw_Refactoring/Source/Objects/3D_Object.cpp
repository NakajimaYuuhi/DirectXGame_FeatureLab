#include "3D_Object.h"
#include "Transform.h"
#include "Mesh.h"
#include "Model.h"
#include "TimeManager.h"

C3D_Object::C3D_Object()
	:CObject()
{
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

void C3D_Object::SetTransform(DirectX::XMFLOAT3 _Position, DirectX::XMFLOAT3 _Scale, DirectX::XMFLOAT3 _Rotation)
{
	CTransform* transform = GetComponent<CTransform>();
	if (transform)
	{
		transform->SetPos(_Position);
		transform->SetScale(_Scale);
		transform->SetRotation(_Rotation);
	}
}

DirectX::XMFLOAT3 C3D_Object::GetPos() 
{ 
	CTransform* transform = GetComponent<CTransform>();
	return transform ? transform->GetPos() : DirectX::XMFLOAT3(0,0,0);
}

void C3D_Object::SetPos(DirectX::XMFLOAT3 _Position)
{ 
	CTransform* transform = GetComponent<CTransform>();
	if (transform) transform->SetPos(_Position);
}

DirectX::XMFLOAT3 C3D_Object::GetScale() 
{ 
	CTransform* transform = GetComponent<CTransform>();
	return transform ? transform->GetScale() : DirectX::XMFLOAT3(1,1,1);
}

void C3D_Object::SetScale(DirectX::XMFLOAT3 _Scale)
{
	CTransform* transform = GetComponent<CTransform>();
	if (transform) transform->SetScale(_Scale);
}

DirectX::XMFLOAT3 C3D_Object::GetRotation() 
{ 
	CTransform* transform = GetComponent<CTransform>();
	return transform ? transform->GetRotation() : DirectX::XMFLOAT3(0,0,0);
}

void C3D_Object::SetRotation(DirectX::XMFLOAT3 _Rotation)
{ 
	CTransform* transform = GetComponent<CTransform>();
	if (transform) transform->SetRotation(_Rotation);
}
