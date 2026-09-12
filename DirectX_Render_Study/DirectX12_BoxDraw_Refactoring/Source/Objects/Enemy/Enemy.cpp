#include "InspectorUI.h"
#include "Enemy.h"
#include "EnemyState.h"
#include "ObjectInfo.h"
#include "Model.h"
#include "ModelManager.h"
#include "Transform.h"
#include "Bullet.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "BoxCollider3D.h"
#include "Explosion.h"
#include "EnemyCounter.h"
#include "TimeManager.h"
#include <cmath>

Enemy::Enemy(String _Name)
	: C3D_Object(_Name)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::ENEMY);

	ModelPath = "Assets/Model/Monk.glb";

	CModel* model = GetComponent<CModel>();
	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);
	model->PlayAnimation("Run");

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetOffset({ 0.0f, 0.75f, 0.0f });
	collider->SetSize({ 0.6f, 1.5f, 0.6f });

	SetScale({ 0.5f, 0.5f, 0.5f });
}

void Enemy::Init()
{
	Awake();
	if (!CInspectorUI::GetInstance().IsEditMode())
	{
		Start();
	}
}

void Enemy::Awake()
{
	if (m_hasAwoken) return;

	HP = MaxHP;
	m_flashTimer = 0.0f;
	SetVisible(true);

	m_hasAwoken = true;
}

void Enemy::Start()
{
	if (m_hasStarted) return;

	EnemyCounter* enemyCounter = (EnemyCounter*)ObjectManager::GetInstance().GetManager("EnemyCounter");
	if (enemyCounter)
	{
		enemyCounter->Instantiated();
	}

	SetScale({ 0.5f, 0.5f, 0.5f });

	m_stateMachine.SetOwner(this);
	m_stateMachine.ChangeState(std::make_shared<EnemyIdleState>());

	m_hasStarted = true;
}

void Enemy::Update()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	if (m_flashTimer > 0.0f)
	{
		m_flashTimer -= dt;
		if (m_flashTimer <= 0.0f)
		{
			m_flashTimer = 0.0f;
			SetVisible(true);
		}
		else
		{
			bool visible = (fmodf(m_flashTimer, BLINK_INTERVAL * 2.0f) >= BLINK_INTERVAL);
			SetVisible(visible);
		}
	}

	CModel* model = GetComponent<CModel>();
	if (model)
	{
		model->UpdateAnimation(dt);
	}

	m_stateMachine.OnUpdate(dt);
}

void Enemy::OnCollision(CObject* _Other)
{
	CObjectInfo* otherInfo = _Other->GetComponent<CObjectInfo>();

	if (otherInfo && otherInfo->GetObjectTag() == ObjectTag::PLAYER_BULLET)
	{
		TakeDamage(1);
	}
}

void Enemy::TakeDamage(int damage)
{
	HP -= damage;

	m_flashTimer = FLASH_DURATION;

	if (HP <= 0)
	{
		m_stateMachine.ChangeState(std::make_shared<EnemyDeadState>());
	}
	else
	{
		m_stateMachine.ChangeState(std::make_shared<EnemyHurtState>());
	}
}

void Enemy::MoveTowards(const DirectX::XMFLOAT3& targetPos, float deltaTime)
{
	DirectX::XMFLOAT3 pos = GetPos();
	float dx = targetPos.x - pos.x;
	float dz = targetPos.z - pos.z;
	float dist = sqrtf(dx * dx + dz * dz);

	if (dist > 0.001f)
	{
		float step = Speed * (deltaTime * 60.0f);
		pos.x += (dx / dist) * step;
		pos.z += (dz / dist) * step;
		SetPos(pos);

		float targetRotY = atan2f(dx, dz);
		SetRotation({ 0.0f, targetRotY, 0.0f });
	}
}

void Enemy::OnDie()
{
	SetIsDestroyed(true);

	EnemyCounter* enemyCounter = (EnemyCounter*)ObjectManager::GetInstance().GetManager("EnemyCounter");
	if (enemyCounter)
	{
		enemyCounter->Defeat();
	}

	C3D_Object* billBoard = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::EFFECT, "Explosion"));
	if (billBoard)
	{
		CTransform* transform = billBoard->GetComponent<CTransform>();
		DirectX::XMFLOAT3 pos = GetPos();
		if (transform)
		{
			transform->SetPos(pos);
			transform->SetScale({ 0.8f, 1.0f, 0.8f });
		}
	}
}
