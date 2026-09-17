#include "Object.h"
#include "ObjectInfo.h"
#include "Component.h"
#include "TimeManager.h"

CObject::CObject()
	: isValid(true)
{
	AddComponent<CObjectInfo>();
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

void CObject::OnCollision(CObject* _Other)
{
	CollisionComponents(_Other);
}

void CObject::AwakeComponents()
{
	for (auto& c : components)
	{
		if (c && c->GetIsValid())
		{
			c->Awake();
		}
	}
}

void CObject::StartComponents()
{
	for (auto& c : components)
	{
		if (c && c->GetIsValid())
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
		for (auto& c : components)
		{
			if (c && c->GetIsValid() && static_cast<int>(c->GetUpdatePhase()) == phase)
			{
				c->Update(deltaTime);
			}
		}
	}
}

void CObject::LateUpdateComponents(float deltaTime)
{
	for (auto& c : components)
	{
		if (c && c->GetIsValid())
		{
			c->LateUpdate(deltaTime);
		}
	}
}

void CObject::CollisionComponents(CObject* _Other)
{
	for (auto& c : components)
	{
		if (c && c->GetIsValid())
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
