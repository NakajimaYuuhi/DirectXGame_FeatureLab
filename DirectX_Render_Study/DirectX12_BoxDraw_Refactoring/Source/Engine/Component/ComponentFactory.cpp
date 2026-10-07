#include "ComponentFactory.h"
#include "Transform.h"
#include "Model.h"
#include "ModelManager.h"
#include "BoxCollider3D.h"
#include "CapsuleCollider3D.h"
#include "audio.h"
#include "GravityComponent.h"
#include "CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "PlayerControllerComponent.h"
#include "EnemyAIComponent.h"
#include "BulletComponent.h"
#include "CameraComponent.h"
#include "BillboardComponent.h"
#include "ParticleComponent.h"
#include "UVAnimationComponent.h"
#include "ParticleEmitterComponent.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "ButtonComponent.h"
#include "EnemyCounterComponent.h"
#include "FieldComponent.h"
#include "LightComponent.h"
#include <windows.h>

ComponentFactory::ComponentFactory()
{
	InitDefaultComponents();
}

void ComponentFactory::RegisterComponent(const std::string& name, CreatorFunc creator)
{
	m_creators[name] = creator;
}

bool ComponentFactory::IsRegistered(const std::string& name) const
{
	return m_creators.find(name) != m_creators.end();
}

CComponent* ComponentFactory::CreateComponent(const std::string& name, CObject* owner, const nlohmann::json& params)
{
	if (!owner) return nullptr;

	auto it = m_creators.find(name);
	if (it != m_creators.end())
	{
		return it->second(owner, params);
	}

	OutputDebugStringA(("[ComponentFactory] Warning: Unregistered component type: " + name + "\n").c_str());
	return nullptr;
}

void ComponentFactory::InitDefaultComponents()
{
	// 1. Transform Component
	RegisterComponent("Transform", [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		CTransform* transform = owner->GetComponent<CTransform>();
		if (!transform) transform = owner->AddComponent<CTransform>();

		if (transform)
		{
			if (p.contains("Position") && p["Position"].is_array() && p["Position"].size() >= 3)
			{
				transform->SetPos({ p["Position"][0], p["Position"][1], p["Position"][2] });
			}
			if (p.contains("Rotation") && p["Rotation"].is_array() && p["Rotation"].size() >= 3)
			{
				transform->SetRotation({ p["Rotation"][0], p["Rotation"][1], p["Rotation"][2] });
			}
			if (p.contains("Scale") && p["Scale"].is_array() && p["Scale"].size() >= 3)
			{
				transform->SetScale({ p["Scale"][0], p["Scale"][1], p["Scale"][2] });
			}
		}
		return transform;
	});

	// 2. Model Component
	RegisterComponent("Model", [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		std::string modelPath = p.value("ModelPath", "");
		CModel* model = owner->GetComponent<CModel>();
		if (!model) model = owner->AddComponent<CModel>();

		if (model && !modelPath.empty())
		{
			auto sharedModel = ModelManager::GetInstance().GetModel(modelPath);
			if (sharedModel)
			{
				model->CopyFrom(sharedModel);
				model->SetModelPath(modelPath);
			}

			if (p.contains("DefaultAnimation"))
			{
				if (p["DefaultAnimation"].is_number())
				{
					model->PlayAnimation(p["DefaultAnimation"].get<int>());
				}
				else if (p["DefaultAnimation"].is_string())
				{
					model->PlayAnimation(p["DefaultAnimation"].get<std::string>());
				}
			}
			else
			{
				model->PlayAnimation(0);
			}

			std::string rLayerStr = p.value("RenderLayer", "Opaque");
			if (rLayerStr == "Transparent") model->SetRenderLayer(RenderLayer::Transparent);
			else if (rLayerStr == "UI") model->SetRenderLayer(RenderLayer::UI);
			else model->SetRenderLayer(RenderLayer::Opaque);

			if (p.contains("BlendMode"))
			{
				std::string blendStr = p["BlendMode"].get<std::string>();
				if (blendStr == "Additive") model->SetBlendModeAll(BlendMode::Additive);
				else if (blendStr == "Alpha") model->SetBlendModeAll(BlendMode::Alpha);
				else if (blendStr == "Opaque") model->SetBlendModeAll(BlendMode::Opaque);
			}

			if (p.contains("TexturePath"))
			{
				std::string texPath = p["TexturePath"].get<std::string>();
				std::wstring wTexPath(texPath.begin(), texPath.end());
				model->SetMaterialTexture(wTexPath.c_str());
			}
		}
		return model;
	});

	// 3. BoxCollider3D Component
	RegisterComponent("BoxCollider3D", [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		BoxCollider3D* collider = owner->GetComponent<BoxCollider3D>();
		if (!collider) collider = owner->AddComponent<BoxCollider3D>();

		if (collider)
		{
			if (p.contains("Size") && p["Size"].is_array() && p["Size"].size() >= 3)
			{
				collider->SetSize({ p["Size"][0], p["Size"][1], p["Size"][2] });
			}
			if (p.contains("Offset") && p["Offset"].is_array() && p["Offset"].size() >= 3)
			{
				collider->SetOffset({ p["Offset"][0], p["Offset"][1], p["Offset"][2] });
			}
			collider->SetIsTrigger(p.value("IsTrigger", false));
			collider->SetLayer(p.value("Layer", (uint32_t)CollisionLayer::Default));
			collider->SetCollisionMask(p.value("CollisionMask", (uint32_t)CollisionLayer::All));
		}
		return collider;
	});

	// CapsuleCollider3D Component
	RegisterComponent("CapsuleCollider3D", [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		CapsuleCollider3D* collider = owner->GetComponent<CapsuleCollider3D>();
		if (!collider) collider = owner->AddComponent<CapsuleCollider3D>();

		if (collider)
		{
			if (p.contains("Radius"))
			{
				collider->SetRadius(p["Radius"].get<float>());
			}
			if (p.contains("Height"))
			{
				collider->SetHeight(p["Height"].get<float>());
			}
			if (p.contains("Offset") && p["Offset"].is_array() && p["Offset"].size() >= 3)
			{
				collider->SetOffset({ p["Offset"][0], p["Offset"][1], p["Offset"][2] });
			}
			collider->SetIsTrigger(p.value("IsTrigger", false));
			collider->SetLayer(p.value("Layer", (uint32_t)CollisionLayer::Default));
			collider->SetCollisionMask(p.value("CollisionMask", (uint32_t)CollisionLayer::All));
		}
		return collider;
	});

	// 4. Audio Component
	RegisterComponent("Audio", [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		std::string audioPath = p.value("FilePath", "");
		Audio* audio = owner->GetComponent<Audio>();
		if (!audio) audio = owner->AddComponent<Audio>();

		if (audio && !audioPath.empty())
		{
			audio->Load(audioPath.c_str());
		}
		return audio;
	});

	// 5. Gravity Component
	auto gravityCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		float grav = p.value("Gravity", -25.0f);
		float jPow = p.value("JumpPower", 0.0f);
		GravityComponent* gravity = owner->GetComponent<GravityComponent>();
		if (!gravity) gravity = owner->AddComponent<GravityComponent>(grav, jPow);
		else
		{
			gravity->SetGravity(grav);
			gravity->SetJumpPower(jPow);
		}
		return gravity;
	};
	RegisterComponent("Gravity", gravityCreator);
	RegisterComponent("GravityComponent", gravityCreator);

	// 6. Movement Component
	auto movementCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		float spd = p.value("Speed", 0.05f);
		CharacterMovementComponent* moveComp = owner->GetComponent<CharacterMovementComponent>();
		if (!moveComp) moveComp = owner->AddComponent<CharacterMovementComponent>(spd);
		else moveComp->SetSpeed(spd);

		if (moveComp)
		{
			moveComp->SetMinClimbNormalY(p.value("MinClimbNormalY", 0.45f));
			moveComp->SetAutoRotate(p.value("AutoRotate", true));
		}
		return moveComp;
	};
	RegisterComponent("Movement", movementCreator);
	RegisterComponent("CharacterMovementComponent", movementCreator);

	// 7. Health Component
	auto healthCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		int maxHp = p.value("MaxHP", 10);
		float invDur = p.value("InvincibleDuration", 1.5f);
		float blkInt = p.value("BlinkInterval", 0.08f);
		HealthComponent* health = owner->GetComponent<HealthComponent>();
		if (!health) health = owner->AddComponent<HealthComponent>(maxHp, invDur, blkInt);

		if (health)
		{
			if (auto controller = owner->GetComponent<PlayerControllerComponent>())
			{
				health->SetOnDamagedCallback([controller](int, int) {
					if (controller) controller->OnDamaged();
				});
				health->SetOnDieCallback([controller]() {
					if (controller) controller->OnDie();
				});
			}
			if (auto ai = owner->GetComponent<EnemyAIComponent>())
			{
				health->SetOnDamagedCallback([ai](int, int) {
					if (ai) ai->OnDamaged();
				});
				health->SetOnDieCallback([ai]() {
					if (ai) ai->OnDie();
				});
			}
		}
		return health;
	};
	RegisterComponent("Health", healthCreator);
	RegisterComponent("HealthComponent", healthCreator);

	// 8. PlayerController Component
	auto playerCtrlCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		PlayerControllerComponent* controller = owner->GetComponent<PlayerControllerComponent>();
		if (!controller) controller = owner->AddComponent<PlayerControllerComponent>();

		HealthComponent* health = owner->GetComponent<HealthComponent>();
		if (health && controller)
		{
			health->SetOnDamagedCallback([controller](int, int) {
				if (controller) controller->OnDamaged();
			});
			health->SetOnDieCallback([controller]() {
				if (controller) controller->OnDie();
			});
		}
		return controller;
	};
	RegisterComponent("PlayerController", playerCtrlCreator);
	RegisterComponent("PlayerControllerComponent", playerCtrlCreator);

	// 9. EnemyAI Component
	auto enemyAICreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		EnemyAIComponent* ai = owner->GetComponent<EnemyAIComponent>();
		if (!ai) ai = owner->AddComponent<EnemyAIComponent>();

		if (ai)
		{
			ai->SetDeathEffectPrefab(p.value("DeathEffectPrefab", "Explosion"));
			ai->SetDamagedEffectPrefab(p.value("DamagedEffectPrefab", "RandomParticle"));
		}

		HealthComponent* health = owner->GetComponent<HealthComponent>();
		if (health && ai)
		{
			health->SetOnDamagedCallback([ai](int, int) {
				if (ai) ai->OnDamaged();
			});
			health->SetOnDieCallback([ai]() {
				if (ai) ai->OnDie();
			});
		}
		return ai;
	};
	RegisterComponent("EnemyAI", enemyAICreator);
	RegisterComponent("EnemyAIComponent", enemyAICreator);

	// 10. Bullet Component
	auto bulletCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		BulletComponent* bulletComp = owner->GetComponent<BulletComponent>();
		if (!bulletComp) bulletComp = owner->AddComponent<BulletComponent>();

		if (bulletComp)
		{
			bulletComp->SetSpeed(p.value("Speed", 0.04f));
			bulletComp->SetLifeTime(p.value("LifeTime", 5.0f));
			bulletComp->SetDamage(p.value("Damage", 1));
			bulletComp->SetHitEffectPrefab(p.value("HitEffectPrefab", "RandomParticle"));
			if (p.contains("Direction") && p["Direction"].is_array() && p["Direction"].size() >= 3)
			{
				bulletComp->SetDirection({ p["Direction"][0], p["Direction"][1], p["Direction"][2] });
			}
		}
		return bulletComp;
	};
	RegisterComponent("Bullet", bulletCreator);
	RegisterComponent("BulletComponent", bulletCreator);

	// 11. Camera Component
	auto cameraCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		CameraComponent* cameraComp = owner->GetComponent<CameraComponent>();
		if (!cameraComp) cameraComp = owner->AddComponent<CameraComponent>();

		if (cameraComp)
		{
			cameraComp->SetDistance(p.value("Distance", 5.0f));
			cameraComp->SetHeight(p.value("Height", 2.5f));
			cameraComp->SetAngleY(p.value("AngleY", 0.0f));
			cameraComp->SetAngleX(p.value("AngleX", 0.0f));
			cameraComp->SetFov(p.value("Fov", DirectX::XM_PIDIV4));
			cameraComp->SetNearZ(p.value("NearZ", 0.1f));
			cameraComp->SetFarZ(p.value("FarZ", 1000.0f));
			cameraComp->SetFollowSpeed(p.value("FollowSpeed", 12.0f));
			if (p.contains("TargetOffset") && p["TargetOffset"].is_array() && p["TargetOffset"].size() >= 3)
			{
				cameraComp->SetTargetOffset({ p["TargetOffset"][0], p["TargetOffset"][1], p["TargetOffset"][2] });
			}
		}
		return cameraComp;
	};
	RegisterComponent("Camera", cameraCreator);
	RegisterComponent("CameraComponent", cameraCreator);

	// 12. Billboard Component
	auto billboardCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		BillboardComponent* bbComp = owner->GetComponent<BillboardComponent>();
		if (!bbComp) bbComp = owner->AddComponent<BillboardComponent>();
		if (bbComp)
		{
			bbComp->SetLockYAxis(p.value("LockYAxis", false));
		}
		return bbComp;
	};
	RegisterComponent("Billboard", billboardCreator);
	RegisterComponent("BillboardComponent", billboardCreator);

	// 13. Particle Component
	auto particleCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		ParticleComponent* ptComp = owner->GetComponent<ParticleComponent>();
		if (!ptComp) ptComp = owner->AddComponent<ParticleComponent>();
		if (ptComp)
		{
			ptComp->SetSpeed(p.value("Speed", 0.05f));
			ptComp->SetLifeTime(p.value("LifeTime", 1.0f));
			if (p.value("RandomDirection", false))
			{
				ptComp->InitRandomDirection(p.value("Speed", 0.05f), p.value("LifeTime", 1.0f));
			}
			else if (p.contains("Direction") && p["Direction"].is_array() && p["Direction"].size() >= 3)
			{
				ptComp->SetDirection({ p["Direction"][0], p["Direction"][1], p["Direction"][2] });
			}
		}
		return ptComp;
	};
	RegisterComponent("Particle", particleCreator);
	RegisterComponent("ParticleComponent", particleCreator);

	// 14. UVAnimation Component
	auto uvAnimCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		UVAnimationComponent* uvComp = owner->GetComponent<UVAnimationComponent>();
		if (!uvComp) uvComp = owner->AddComponent<UVAnimationComponent>();
		if (uvComp)
		{
			uvComp->SetGrid(p.value("Rows", 1), p.value("Cols", 1));
			if (p.contains("TotalFrames")) uvComp->SetTotalFrames(p["TotalFrames"].get<int>());
			uvComp->SetFrameDuration(p.value("FrameDuration", 0.05f));
			uvComp->SetLoop(p.value("Loop", false));
			uvComp->SetDestroyOnComplete(p.value("DestroyOnComplete", true));
		}
		return uvComp;
	};
	RegisterComponent("UVAnimation", uvAnimCreator);
	RegisterComponent("UVAnimationComponent", uvAnimCreator);

	// 15. ParticleEmitter Component
	auto particleEmitterCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		ParticleEmitterComponent* peComp = owner->GetComponent<ParticleEmitterComponent>();
		if (!peComp) peComp = owner->AddComponent<ParticleEmitterComponent>();
		if (peComp)
		{
			peComp->SetParticlePrefab(p.value("ParticlePrefab", "RandomParticle"));
			peComp->SetBurstCount(p.value("BurstCount", 10));
			peComp->SetSpawnInterval(p.value("SpawnInterval", 0.0f));
			peComp->SetBurstOnStart(p.value("BurstOnStart", true));
			if (p.contains("ParticleScale") && p["ParticleScale"].is_array() && p["ParticleScale"].size() >= 3)
			{
				peComp->SetParticleScale({ p["ParticleScale"][0], p["ParticleScale"][1], p["ParticleScale"][2] });
			}
		}
		return peComp;
	};
	RegisterComponent("ParticleEmitter", particleEmitterCreator);
	RegisterComponent("ParticleEmitterComponent", particleEmitterCreator);

	// 16. CSpriteRenderer Component
	auto spriteCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		CSpriteRenderer* sprite = owner->GetComponent<CSpriteRenderer>();
		if (!sprite) sprite = owner->AddComponent<CSpriteRenderer>();
		if (sprite)
		{
			if (p.contains("TexturePath"))
			{
				std::string texStr = p["TexturePath"].get<std::string>();
				std::wstring texW(texStr.begin(), texStr.end());
				sprite->SetTexture(texW);
			}
			if (p.contains("Size") && p["Size"].is_array() && p["Size"].size() >= 2)
			{
				sprite->SetSize(p["Size"][0], p["Size"][1]);
			}
			if (p.contains("Color") && p["Color"].is_array() && p["Color"].size() >= 4)
			{
				sprite->SetColor({ p["Color"][0], p["Color"][1], p["Color"][2], p["Color"][3] });
			}
		}
		return sprite;
	};
	RegisterComponent("SpriteRenderer", spriteCreator);
	RegisterComponent("CSpriteRenderer", spriteCreator);

	// 17. CTextRenderer Component
	auto textCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		CTextRenderer* textComp = owner->GetComponent<CTextRenderer>();
		if (!textComp) textComp = owner->AddComponent<CTextRenderer>();
		if (textComp)
		{
			if (p.contains("Content"))
			{
				std::string contentStr = p["Content"].get<std::string>();
				std::wstring contentW(contentStr.begin(), contentStr.end());
				textComp->SetText(contentW);
			}
			if (p.contains("FontSize")) textComp->SetFontSize(p["FontSize"].get<float>());
			if (p.contains("FontFamily"))
			{
				std::string fontStr = p["FontFamily"].get<std::string>();
				std::wstring fontW(fontStr.begin(), fontStr.end());
				textComp->SetFontFamily(fontW);
			}
			if (p.contains("Color") && p["Color"].is_array() && p["Color"].size() == 4)
			{
				textComp->SetColor(D2D1::ColorF(p["Color"][0], p["Color"][1], p["Color"][2], p["Color"][3]));
			}
		}
		return textComp;
	};
	RegisterComponent("TextRenderer", textCreator);
	RegisterComponent("CTextRenderer", textCreator);

	// 18. ButtonComponent
	auto buttonCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		ButtonComponent* btnComp = owner->GetComponent<ButtonComponent>();
		if (!btnComp) btnComp = owner->AddComponent<ButtonComponent>();
		if (btnComp)
		{
			if (p.contains("Action"))
			{
				btnComp->SetAction(StringToButtonAction(p["Action"].get<std::string>()));
			}
			std::string up = p.value("NavUp", "");
			std::string down = p.value("NavDown", "");
			std::string left = p.value("NavLeft", "");
			std::string right = p.value("NavRight", "");
			btnComp->SetNavigationNames(up, down, left, right);
		}
		return btnComp;
	};
	RegisterComponent("Button", buttonCreator);
	RegisterComponent("ButtonComponent", buttonCreator);

	// 19. EnemyCounterComponent
	auto enemyCounterCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		EnemyCounterComponent* comp = owner->GetComponent<EnemyCounterComponent>();
		if (!comp) comp = owner->AddComponent<EnemyCounterComponent>();
		if (comp && p.contains("TargetTextName"))
		{
			comp->SetTargetTextName(p["TargetTextName"].get<std::string>());
		}
		return comp;
	};
	RegisterComponent("EnemyCounterComponent", enemyCounterCreator);

	// 19. FieldComponent
	auto fieldCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		FieldComponent* fComp = owner->GetComponent<FieldComponent>();
		if (!fComp) fComp = owner->AddComponent<FieldComponent>();
		if (p.contains("Width")) fComp->SetWidth(p["Width"].get<float>());
		if (p.contains("Depth")) fComp->SetDepth(p["Depth"].get<float>());
		if (p.contains("GridX")) fComp->SetGridX(p["GridX"].get<int>());
		if (p.contains("GridZ")) fComp->SetGridZ(p["GridZ"].get<int>());
		if (p.contains("UVTiling")) fComp->SetUVTiling(p["UVTiling"].get<float>());
		return fComp;
	};
	RegisterComponent("FieldComponent", fieldCreator);
	RegisterComponent("Field", fieldCreator);
	RegisterComponent("EnemyCounter", enemyCounterCreator);

	// 20. LightComponent
	auto lightCreator = [](CObject* owner, const nlohmann::json& p) -> CComponent* {
		LightComponent* lComp = owner->GetComponent<LightComponent>();
		if (!lComp) lComp = owner->AddComponent<LightComponent>();
		if (lComp)
		{
			if (p.contains("Type"))
			{
				if (p["Type"].is_number())
				{
					lComp->SetLightType(static_cast<LightType>(p["Type"].get<int>()));
				}
				else if (p["Type"].is_string())
				{
					std::string tStr = p["Type"].get<std::string>();
					if (tStr == "Point") lComp->SetLightType(LightType::Point);
					else if (tStr == "Spot") lComp->SetLightType(LightType::Spot);
					else lComp->SetLightType(LightType::Directional);
				}
			}
			if (p.contains("Color") && p["Color"].is_array() && p["Color"].size() >= 3)
			{
				lComp->SetColor(p["Color"][0], p["Color"][1], p["Color"][2]);
			}
			if (p.contains("Intensity")) lComp->SetIntensity(p["Intensity"].get<float>());
			if (p.contains("AmbientColor") && p["AmbientColor"].is_array() && p["AmbientColor"].size() >= 3)
			{
				lComp->SetAmbientColor(p["AmbientColor"][0], p["AmbientColor"][1], p["AmbientColor"][2]);
			}
			if (p.contains("SpecularPower")) lComp->SetSpecularPower(p["SpecularPower"].get<float>());
			if (p.contains("CastShadow")) lComp->SetCastShadow(p["CastShadow"].get<bool>());
			if (p.contains("ShadowBias")) lComp->SetShadowBias(p["ShadowBias"].get<float>());
			if (p.contains("ShadowDarkness")) lComp->SetShadowDarkness(p["ShadowDarkness"].get<float>());
			if (p.contains("Range")) lComp->SetRange(p["Range"].get<float>());
			if (p.contains("SpotAngle")) lComp->SetSpotAngle(p["SpotAngle"].get<float>());

			lComp->SyncToLightManager();
		}
		return lComp;
	};
	RegisterComponent("LightComponent", lightCreator);
	RegisterComponent("Light", lightCreator);
}
