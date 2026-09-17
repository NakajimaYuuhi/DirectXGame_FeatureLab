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

	HP = MaxHP;
	m_invincibleTimer = 0.0f;
	SetVisible(true);

	m_hasAwoken = true;
}

void Player::Start()
{
	if (m_hasStarted) return;

	m_camera = ObjectManager::GetInstance().GetCamera();
	m_field = ObjectManager::GetInstance().GetField();

	SetScale({ 0.5f, 0.5f, 0.5f });

	SnapToGround();

	m_stateMachine.SetOwner(this);
	m_stateMachine.ChangeState(std::make_shared<PlayerIdleState>());

	m_hasStarted = true;
}

void Player::SnapToGround()
{
	if (!m_field)
	{
		m_field = ObjectManager::GetInstance().GetField();
	}
	if (m_field)
	{
		DirectX::XMFLOAT3 pos = GetPos();
		float groundY = 0.0f;
		if (m_field->GetHeight(pos.x, pos.z, groundY))
		{
			pos.y = groundY;
			SetPos(pos);
			m_verticalVelocity = 0.0f;
			m_isGrounded = true;
		}
	}
}

void Player::ApplyGravity(float deltaTime)
{
	if (!m_field)
	{
		m_field = ObjectManager::GetInstance().GetField();
	}

	DirectX::XMFLOAT3 pos = GetPos();
	float groundY = 0.0f;
	bool hasGround = (m_field != nullptr) && m_field->GetHeight(pos.x, pos.z, groundY);

	if (!hasGround)
	{
		groundY = 0.0f;
	}

	// Apply gravity acceleration
	m_verticalVelocity += GRAVITY * deltaTime;
	if (m_verticalVelocity < TERMINAL_VELOCITY)
	{
		m_verticalVelocity = TERMINAL_VELOCITY;
	}

	float nextY = pos.y + m_verticalVelocity * deltaTime;

	// Ground check
	if (m_isGrounded && m_verticalVelocity <= 0.0f)
	{
		// While grounded, snap to slopes smoothly within step down limit
		float diff = pos.y - groundY;
		if (diff >= -0.1f && diff <= STEP_DOWN_LIMIT)
		{
			pos.y = groundY;
			m_verticalVelocity = 0.0f;
			m_isGrounded = true;
		}
		else if (nextY <= groundY)
		{
			pos.y = groundY;
			m_verticalVelocity = 0.0f;
			m_isGrounded = true;
		}
		else
		{
			// Step off a high ledge -> fall
			pos.y = nextY;
			m_isGrounded = false;
		}
	}
	else
	{
		// Airborne: fall and land
		if (nextY <= groundY)
		{
			pos.y = groundY;
			m_verticalVelocity = 0.0f;
			m_isGrounded = true;
		}
		else
		{
			pos.y = nextY;
			m_isGrounded = false;
		}
	}

	SetPos(pos);
}

void Player::Update()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	// Jump input
	if (m_isGrounded && CInputManager::GetInstance().IsKeyTrigger(VK_SPACE))
	{
		m_verticalVelocity = JUMP_POWER;
		m_isGrounded = false;
	}

	// Apply gravity
	ApplyGravity(dt);

	if (m_invincibleTimer > 0.0f)
	{
		m_invincibleTimer -= dt;
		if (m_invincibleTimer <= 0.0f)
		{
			m_invincibleTimer = 0.0f;
			SetVisible(true);
		}
		else
		{
			bool visible = (fmodf(m_invincibleTimer, BLINK_INTERVAL * 2.0f) >= BLINK_INTERVAL);
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
	DirectX::XMFLOAT3 right = { c, 0.0f, s };

	DirectX::XMFLOAT3 pos = GetPos();
	DirectX::XMFLOAT3 movement = { 0.0f, 0.0f, 0.0f };

	float frameSpeed = Speed * (deltaTime * 60.0f);

	if (CInputManager::GetInstance().IsKeyPress('W'))
	{
		movement.x += forward.x * frameSpeed;
		movement.z += forward.z * frameSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('S'))
	{
		movement.x -= forward.x * frameSpeed;
		movement.z -= forward.z * frameSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('D'))
	{
		movement.x += right.x * frameSpeed;
		movement.z += right.z * frameSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('A'))
	{
		movement.x -= right.x * frameSpeed;
		movement.z -= right.z * frameSpeed;
	}

	if (movement.x != 0.0f || movement.z != 0.0f)
	{
		DirectX::XMFLOAT3 targetPos = { pos.x + movement.x, pos.y, pos.z + movement.z };

		if (!m_field)
		{
			m_field = ObjectManager::GetInstance().GetField();
		}

		if (m_field)
		{
			float groundY = 0.0f;
			DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };

			if (m_field->GetHeight(targetPos.x, targetPos.z, groundY))
			{
				m_field->GetNormal(targetPos.x, targetPos.z, normal);

				// 登坂制限（傾斜が急すぎる崖＝法線Ny < 0.45、約63度以上は登れない）
				const float MIN_CLIMB_NORMAL_Y = 0.45f;
				if (normal.y >= MIN_CLIMB_NORMAL_Y)
				{
					if (m_isGrounded)
					{
						targetPos.y = groundY;
					}
					SetPos(targetPos);
				}
				else
				{
					// 急斜面の場合は法線の水平成分に沿って滑らせる
					float dot = movement.x * normal.x + movement.z * normal.z;
					if (dot < 0.0f)
					{
						float slideX = movement.x - normal.x * dot;
						float slideZ = movement.z - normal.z * dot;
						DirectX::XMFLOAT3 slidePos = { pos.x + slideX, pos.y, pos.z + slideZ };
						float slideY = 0.0f;
						if (m_field->GetHeight(slidePos.x, slidePos.z, slideY))
						{
							if (m_isGrounded)
							{
								slidePos.y = slideY;
							}
							SetPos(slidePos);
						}
					}
				}
			}
			else
			{
				SetPos(targetPos);
			}
		}
		else
		{
			SetPos(targetPos);
		}

		float targetRotY = atan2f(movement.x, movement.z);
		SetRotation({ 0.0f, targetRotY, 0.0f });
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
	if (HP <= 0 || IsInvincible()) return;

	HP -= damage;
	if (HP < 0)
	{
		HP = 0;
	}

	m_invincibleTimer = INVINCIBLE_DURATION;

	OutputDebugStringA(("Player Took Damage! Current HP: " + std::to_string(HP) + "\n").c_str());

	if (HP <= 0)
	{
		m_stateMachine.ChangeState(std::make_shared<PlayerDeadState>());
	}
	else
	{
		m_stateMachine.ChangeState(std::make_shared<PlayerHurtState>());
	}
}

void Player::OnDie()
{
	SceneManager::GetInstance().ChangeSceneWithFade(Scenes::ID::Failed, 0.5f);
}
