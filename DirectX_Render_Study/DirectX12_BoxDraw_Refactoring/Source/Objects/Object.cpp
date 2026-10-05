#include "Object.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Component.h"
#include "TimeManager.h"
#include "Source/UI/RectTransform.h"

CObject::CObject()
	: isValid(true)
{
	AddComponent<CObjectInfo>();
	AddComponent<CTransform>();
}

CObject::CObject(String _Name)
	: CObject()
{
	SetName(_Name);
}

CObject::~CObject() = default;

void CObject::Init()
{
	for (auto& c : components)
	{
		if (c) c->Init();
	}
}

void CObject::Awake()
{
	if (m_hasAwoken) return;
	AwakeComponents();
	m_hasAwoken = true;
}

void CObject::Start()
{
	if (m_hasStarted) return;
	StartComponents();
	m_hasStarted = true;
}

void CObject::Update()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();
	UpdateComponents(dt);
}

void CObject::LateUpdate()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();
	LateUpdateComponents(dt);
}

void CObject::Draw()
{
	if (!m_isVisible || !isValid || IsDestroyed) return;
	for (auto& c : components)
	{
		if (c && c->IsEnabled())
		{
			c->Draw();
		}
	}
}

void CObject::DrawByLayer(RenderLayer layer)
{
	if (!m_isVisible || !isValid || IsDestroyed) return;
	for (auto& c : components)
	{
		if (c && c->IsEnabled() && c->GetRenderLayer() == layer)
		{
			c->Draw();
		}
	}
}

void CObject::OnCollision(CObject* _Other)
{
	CollisionComponents(_Other);
}

void CObject::AwakeComponents()
{
	for (auto& c : components)
	{
		if (c && c->IsEnabled())
		{
			c->Awake();
		}
	}
}

void CObject::StartComponents()
{
	for (auto& c : components)
	{
		if (c && c->IsEnabled())
		{
			c->Start();
		}
	}
}

void CObject::UpdateComponents(float deltaTime)
{
	// Execute components phase by phase (Input -> AI -> Movement -> Physics -> Animation -> PostPhysics)
	for (int phase = 0; phase < static_cast<int>(UpdatePhase::COUNT); ++phase)
	{
		UpdateComponentsByPhase(static_cast<UpdatePhase>(phase), deltaTime);
	}
}

void CObject::UpdateComponentsByPhase(UpdatePhase phase, float deltaTime)
{
	if (!isValid || IsDestroyed) return;
	for (auto& c : components)
	{
		if (c && c->IsEnabled() && c->GetUpdatePhase() == phase)
		{
			c->Update(deltaTime);
		}
	}
}

void CObject::LateUpdateComponents(float deltaTime)
{
	for (auto& c : components)
	{
		if (c && c->IsEnabled())
		{
			c->LateUpdate(deltaTime);
		}
	}
}

void CObject::CollisionComponents(CObject* _Other)
{
	for (auto& c : components)
	{
		if (c && c->IsEnabled())
		{
			c->OnCollision(_Other);
		}
	}
}

void CObject::SetName(String _ObjectName)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	if (objectInfo)
	{
		objectInfo->SetObjectName(_ObjectName);
	}
}

String CObject::GetName() const
{
	const CObjectInfo* objectInfo = const_cast<CObject*>(this)->GetComponent<CObjectInfo>();
	return objectInfo ? objectInfo->GetObjectName() : "";
}

CRectTransform* CObject::GetRectTransform()
{
	return GetComponent<CRectTransform>();
}
