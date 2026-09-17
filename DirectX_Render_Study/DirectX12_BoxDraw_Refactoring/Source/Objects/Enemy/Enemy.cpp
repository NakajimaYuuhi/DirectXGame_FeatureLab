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
#include "Field.h"
#include "GravityComponent.h"
#include "HealthComponent.h"
#include "CharacterMovementComponent.h"
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

	// Add Gravity, Health, and Movement components
	m_gravityComp = AddComponent<GravityComponent>(-25.0f, 0.0f);
	m_healthComp = AddComponent<HealthComponent>(3, 0.3f, 0.06f);
	m_movementComp = AddComponent<CharacterMovementComponent>(Speed);

	m_healthComp->SetOnDamagedCallback([this](int curHp, int maxHp) {
		m_stateMachine.ChangeState(std::make_shared<EnemyHurtState>());
	});
	m_healthComp->SetOnDieCallback([this]() {
		m_stateMachine.ChangeState(std::make_shared<EnemyDeadState>());
	});
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

	AwakeComponents();
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

	StartComponents();

	m_stateMachine.SetOwner(this);
	m_stateMachine.ChangeState(std::make_shared<EnemyIdleState>());

	m_hasStarted = true;
}

void Enemy::SnapToGround()
{
	if (m_gravityComp)
	{
		m_gravityComp->SnapToGround();
	}
}

bool Enemy::IsGrounded() const
{
	return m_gravityComp ? m_gravityComp->IsGrounded() : false;
}

int Enemy::GetHP() const
{
	return m_healthComp ? m_healthComp->GetHP() : 0;
}

void Enemy::TakeDamage(int damage)
{
	if (m_healthComp)
	{
		m_healthComp->TakeDamage(damage);
	}
}

void Enemy::Update()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	CModel* model = GetComponent<CModel>();
	if (model)
	{
		model->UpdateAnimation(dt);
	}

	m_stateMachine.OnUpdate(dt);

	// Automatically updates components phase by phase (Physics/Gravity -> Health etc.)
	UpdateComponents(dt);
}

void Enemy::OnCollision(CObject* _Other)
{
	CollisionComponents(_Other);

	CObjectInfo* otherInfo = _Other->GetComponent<CObjectInfo>();

	if (otherInfo && otherInfo->GetObjectTag() == ObjectTag::PLAYER_BULLET)
	{
		TakeDamage(1);
	}
}

void Enemy::MoveTowards(const DirectX::XMFLOAT3& targetPos, float deltaTime)
{
	if (m_movementComp)
	{
		m_movementComp->MoveTowards(targetPos, deltaTime);
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
