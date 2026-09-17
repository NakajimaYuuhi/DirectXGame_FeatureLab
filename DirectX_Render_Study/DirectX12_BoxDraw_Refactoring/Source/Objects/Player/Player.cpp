#include "InspectorUI.h"
#include "Player.h"
#include "PlayerState.h"
#include "Camera.h"
#include "Model.h"
#include "ModelManager.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Bullet.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "BoxCollider3D.h"
#include "TimeManager.h"
#include "audio.h"
#include "Field.h"
#include "GravityComponent.h"
#include "HealthComponent.h"
#include "CharacterMovementComponent.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include <cmath>

Player::Player(String _Name)
	: C3D_Object(_Name)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::PLAYER);

	ModelPath = "Assets/Model/Wizard.glb";

	CModel* model = GetComponent<CModel>();
	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);
	model->PlayAnimation("Run");

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetOffset({ 0.0f, 0.75f, 0.0f });
	collider->SetSize({ 0.6f, 1.5f, 0.6f });

	Audio* audio = AddComponent<Audio>();
	audio->Load("Assets/Audio/SE/Fire1.wav");

	SetScale({ 0.5f, 0.5f, 0.5f });

	// Add Movement, Gravity, and Health components
	m_movementComp = AddComponent<CharacterMovementComponent>(0.1f);
	m_gravityComp  = AddComponent<GravityComponent>(-25.0f, 8.5f);
	m_healthComp   = AddComponent<HealthComponent>(10, 1.5f, 0.08f);

	m_healthComp->SetOnDamagedCallback([this](int curHp, int maxHp) {
		m_stateMachine.ChangeState(std::make_shared<PlayerHurtState>());
	});
	m_healthComp->SetOnDieCallback([this]() {
		m_stateMachine.ChangeState(std::make_shared<PlayerDeadState>());
	});
}

void Player::Init()
{
	Awake();
	if (!CInspectorUI::GetInstance().IsEditMode())
	{
		Start();
	}
}

void Player::Awake()
{
	if (m_hasAwoken) return;

	AwakeComponents();
	SetVisible(true);

	m_hasAwoken = true;
}

void Player::Start()
{
	if (m_hasStarted) return;

	m_camera = ObjectManager::GetInstance().GetCamera();
	SetScale({ 0.5f, 0.5f, 0.5f });

	StartComponents();

	m_stateMachine.SetOwner(this);
	m_stateMachine.ChangeState(std::make_shared<PlayerIdleState>());

	m_hasStarted = true;
}

void Player::SnapToGround()
{
	if (m_gravityComp)
	{
		m_gravityComp->SnapToGround();
	}
}

bool Player::IsGrounded() const
{
	return m_gravityComp ? m_gravityComp->IsGrounded() : false;
}

int Player::GetHP() const
{
	return m_healthComp ? m_healthComp->GetHP() : 0;
}

int Player::GetMaxHP() const
{
	return m_healthComp ? m_healthComp->GetMaxHP() : 0;
}

bool Player::IsDead() const
{
	return m_healthComp ? m_healthComp->IsDead() : false;
}

bool Player::IsInvincible() const
{
	return m_healthComp ? m_healthComp->IsInvincible() : false;
}

void Player::Update()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	// Jump input -> delegated to GravityComponent
	if (m_gravityComp && m_gravityComp->IsGrounded() && CInputManager::GetInstance().IsKeyTrigger(VK_SPACE))
	{
		m_gravityComp->Jump();
	}

	CModel* model = GetComponent<CModel>();
	if (model)
	{
		model->UpdateAnimation(dt);
	}

	m_stateMachine.OnUpdate(dt);

	// Automatically updates components phase by phase (Physics/Gravity -> Health etc.)
	UpdateComponents(dt);
}

bool Player::HasMoveInput() const
{
	int moveX = 0;
	int moveZ = 0;
	if (CInputManager::GetInstance().IsKeyPress('D')) moveX += 1;
	if (CInputManager::GetInstance().IsKeyPress('A')) moveX -= 1;
	if (CInputManager::GetInstance().IsKeyPress('W')) moveZ += 1;
	if (CInputManager::GetInstance().IsKeyPress('S')) moveZ -= 1;

	return (moveX != 0 || moveZ != 0);
}

void Player::ProcessMovement(float deltaTime)
{
	if (!m_camera)
	{
		m_camera = ObjectManager::GetInstance().GetCamera();
		if (!m_camera) return;
	}

	float angleY = m_camera->GetAngleY();
	float s = sinf(angleY);
	float c = cosf(angleY);

	DirectX::XMFLOAT3 forward = { -s, 0.0f, c };
	DirectX::XMFLOAT3 right   = {  c, 0.0f, s };
	DirectX::XMFLOAT3 dir     = { 0.0f, 0.0f, 0.0f };

	if (CInputManager::GetInstance().IsKeyPress('W'))
	{
		dir.x += forward.x;
		dir.z += forward.z;
	}
	if (CInputManager::GetInstance().IsKeyPress('S'))
	{
		dir.x -= forward.x;
		dir.z -= forward.z;
	}
	if (CInputManager::GetInstance().IsKeyPress('D'))
	{
		dir.x += right.x;
		dir.z += right.z;
	}
	if (CInputManager::GetInstance().IsKeyPress('A'))
	{
		dir.x -= right.x;
		dir.z -= right.z;
	}

	float len = sqrtf(dir.x * dir.x + dir.z * dir.z);
	if (len > 0.001f)
	{
		dir.x /= len;
		dir.z /= len;
	}

	if (m_movementComp)
	{
		m_movementComp->MoveDirection(dir, deltaTime);
	}
}

void Player::PerformAttack()
{
	Audio* audio = GetComponent<Audio>();

	Bullet* bullet = (Bullet*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::GAME, ObjectTag::PLAYER_BULLET, "Bullet"));
	if (bullet)
	{
		DirectX::XMFLOAT3 bulletPos = GetPos();
		DirectX::XMFLOAT3 front = GetFront();
		bullet->SetTransform(
			{ bulletPos.x + front.x * 0.6f, bulletPos.y + 0.8f, bulletPos.z + front.z * 0.6f },
			{ 0.15f, 0.15f, 0.15f },
			{ 0.0f, 0.0f, 0.0f }
		);

		CModel* Bullet_Model = bullet->GetComponent<CModel>();
		if (Bullet_Model)
		{
			auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/cube.glb");
			Bullet_Model->CopyFrom(sharedModel);
		}

		bullet->SetDirection(front);
	}

	if (audio)
	{
		audio->Play();
	}
}

void Player::TakeDamage(int damage)
{
	if (m_healthComp)
	{
		m_healthComp->TakeDamage(damage);
	}
}

void Player::OnDie()
{
	SceneManager::GetInstance().ChangeSceneWithFade(Scenes::ID::Failed, 0.5f);
}
