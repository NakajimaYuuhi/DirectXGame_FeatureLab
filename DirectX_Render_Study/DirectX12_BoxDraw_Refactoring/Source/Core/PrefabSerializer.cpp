#include "PrefabSerializer.h"
#include "Object.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Model.h"
#include "BoxCollider3D.h"
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
#include "Source/External/json.hpp"
#include <fstream>
#include <filesystem>
#include <windows.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

bool PrefabSerializer::SavePrefab(const std::string& filepath, CObject* obj)
{
    if (!obj) return false;

    // Ensure parent directory exists
    try
    {
        fs::path p(filepath);
        if (p.has_parent_path())
        {
            fs::create_directories(p.parent_path());
        }
    }
    catch (...) {}

    json root;

    // Prefab Name & Tag
    std::string prefabName = fs::path(filepath).stem().string();
    CObjectInfo* info = obj->GetComponent<CObjectInfo>();
    std::string tagStr = "NONE";

    if (info)
    {
        ObjectTag tag = info->GetObjectTag();
        switch (tag)
        {
        case ObjectTag::BACKGROUND:    tagStr = "BACKGROUND"; break;
        case ObjectTag::PLAYER:        tagStr = "PLAYER"; break;
        case ObjectTag::PLAYER_BULLET: tagStr = "PLAYER_BULLET"; break;
        case ObjectTag::ENEMY:         tagStr = "ENEMY"; break;
        case ObjectTag::ENEMY_BULLET:  tagStr = "ENEMY_BULLET"; break;
        case ObjectTag::FIELD:         tagStr = "FIELD"; break;
        case ObjectTag::TRIANGLE:      tagStr = "TRIANGLE"; break;
        case ObjectTag::BILLBOARD:     tagStr = "BILLBOARD"; break;
        case ObjectTag::EFFECT:        tagStr = "EFFECT"; break;
        case ObjectTag::UI:            tagStr = "UI"; break;
        case ObjectTag::TEXT:          tagStr = "TEXT"; break;
        case ObjectTag::CAMERA:        tagStr = "CAMERA"; break;
        case ObjectTag::FADE:          tagStr = "FADE"; break;
        case ObjectTag::MANAGER:       tagStr = "MANAGER"; break;
        default:                       tagStr = "NONE"; break;
        }
    }

    root["PrefabName"] = prefabName;
    root["Tag"] = tagStr;

    json comps = json::object();

    // 1. Transform
    CTransform* transform = obj->GetComponent<CTransform>();
    if (transform)
    {
        DirectX::XMFLOAT3 pos = transform->GetPos();
        DirectX::XMFLOAT3 rot = transform->GetRotation();
        DirectX::XMFLOAT3 scale = transform->GetScale();
        comps["Transform"]["Position"] = { pos.x, pos.y, pos.z };
        comps["Transform"]["Rotation"] = { rot.x, rot.y, rot.z };
        comps["Transform"]["Scale"] = { scale.x, scale.y, scale.z };
    }

    // 2. Model
    CModel* model = obj->GetComponent<CModel>();
    if (model)
    {
        std::string modelPath = model->GetModelPath();
        comps["Model"]["ModelPath"] = modelPath;
        comps["Model"]["DefaultAnimation"] = "Idle";
        RenderLayer rLayer = model->GetRenderLayer();
        if (rLayer == RenderLayer::Transparent) comps["Model"]["RenderLayer"] = "Transparent";
        else if (rLayer == RenderLayer::UI) comps["Model"]["RenderLayer"] = "UI";
        else comps["Model"]["RenderLayer"] = "Opaque";
    }

    // 3. BoxCollider3D
    BoxCollider3D* collider = obj->GetComponent<BoxCollider3D>();
    if (collider)
    {
        DirectX::XMFLOAT3 size = collider->GetSize();
        DirectX::XMFLOAT3 offset = collider->GetOffset();
        comps["BoxCollider3D"]["Size"] = { size.x, size.y, size.z };
        comps["BoxCollider3D"]["Offset"] = { offset.x, offset.y, offset.z };
        comps["BoxCollider3D"]["IsTrigger"] = collider->GetIsTrigger();
        comps["BoxCollider3D"]["Layer"] = collider->GetLayer();
        comps["BoxCollider3D"]["CollisionMask"] = collider->GetCollisionMask();
    }

    // 4. Audio
    Audio* audio = obj->GetComponent<Audio>();
    if (audio)
    {
        comps["Audio"]["FilePath"] = "Assets/Audio/SE/Fire1.wav";
    }

    // 5. Gravity
    GravityComponent* gravity = obj->GetComponent<GravityComponent>();
    if (gravity)
    {
        comps["Gravity"]["Gravity"] = gravity->GetGravity();
        comps["Gravity"]["JumpPower"] = gravity->GetJumpPower();
    }

    // 6. Movement
    CharacterMovementComponent* movement = obj->GetComponent<CharacterMovementComponent>();
    if (movement)
    {
        comps["Movement"]["Speed"] = movement->GetSpeed();
        comps["Movement"]["MinClimbNormalY"] = movement->GetMinClimbNormalY();
        comps["Movement"]["AutoRotate"] = movement->GetAutoRotate();
    }

    // 7. Health
    HealthComponent* health = obj->GetComponent<HealthComponent>();
    if (health)
    {
        comps["Health"]["MaxHP"] = health->GetMaxHP();
        comps["Health"]["InvincibleDuration"] = health->GetInvincibleDuration();
        comps["Health"]["BlinkInterval"] = health->GetBlinkInterval();
    }

    // 8. PlayerController
    PlayerControllerComponent* controller = obj->GetComponent<PlayerControllerComponent>();
    if (controller)
    {
        comps["PlayerController"] = json::object();
    }

    // 9. EnemyAI
    EnemyAIComponent* ai = obj->GetComponent<EnemyAIComponent>();
    if (ai)
    {
        comps["EnemyAI"]["DeathEffectPrefab"] = ai->GetDeathEffectPrefab();
        comps["EnemyAI"]["DamagedEffectPrefab"] = ai->GetDamagedEffectPrefab();
    }

    // 10. BulletComponent
    BulletComponent* bulletComp = obj->GetComponent<BulletComponent>();
    if (bulletComp)
    {
        comps["BulletComponent"]["Speed"] = bulletComp->GetSpeed();
        comps["BulletComponent"]["LifeTime"] = bulletComp->GetLifeTime();
        comps["BulletComponent"]["Damage"] = bulletComp->GetDamage();
        comps["BulletComponent"]["HitEffectPrefab"] = bulletComp->GetHitEffectPrefab();
        DirectX::XMFLOAT3 dir = bulletComp->GetDirection();
        comps["BulletComponent"]["Direction"] = { dir.x, dir.y, dir.z };
    }

    // 11. CameraComponent
    CameraComponent* cameraComp = obj->GetComponent<CameraComponent>();
    if (cameraComp)
    {
        comps["CameraComponent"]["Distance"] = cameraComp->GetDistance();
        comps["CameraComponent"]["Height"] = cameraComp->GetHeight();
        comps["CameraComponent"]["AngleY"] = cameraComp->GetAngleY();
        comps["CameraComponent"]["AngleX"] = cameraComp->GetAngleX();
        comps["CameraComponent"]["Fov"] = cameraComp->GetFov();
        comps["CameraComponent"]["NearZ"] = cameraComp->GetNearZ();
        comps["CameraComponent"]["FarZ"] = cameraComp->GetFarZ();
        comps["CameraComponent"]["FollowSpeed"] = cameraComp->GetFollowSpeed();
        DirectX::XMFLOAT3 offset = cameraComp->GetTargetOffset();
        comps["CameraComponent"]["TargetOffset"] = { offset.x, offset.y, offset.z };
    }

    // 12. BillboardComponent
    BillboardComponent* bbComp = obj->GetComponent<BillboardComponent>();
    if (bbComp)
    {
        comps["BillboardComponent"]["LockYAxis"] = bbComp->GetLockYAxis();
    }

    // 13. ParticleComponent
    ParticleComponent* ptComp = obj->GetComponent<ParticleComponent>();
    if (ptComp)
    {
        comps["ParticleComponent"]["Speed"] = ptComp->GetSpeed();
        comps["ParticleComponent"]["LifeTime"] = ptComp->GetLifeTime();
        comps["ParticleComponent"]["RandomDirection"] = ptComp->IsRandomDirection();
        DirectX::XMFLOAT3 dir = ptComp->GetDirection();
        comps["ParticleComponent"]["Direction"] = { dir.x, dir.y, dir.z };
    }

    // 14. UVAnimationComponent
    UVAnimationComponent* uvComp = obj->GetComponent<UVAnimationComponent>();
    if (uvComp)
    {
        comps["UVAnimationComponent"]["Rows"] = uvComp->GetRows();
        comps["UVAnimationComponent"]["Cols"] = uvComp->GetCols();
        comps["UVAnimationComponent"]["TotalFrames"] = uvComp->GetTotalFrames();
        comps["UVAnimationComponent"]["FrameDuration"] = uvComp->GetFrameDuration();
        comps["UVAnimationComponent"]["Loop"] = uvComp->IsLoop();
        comps["UVAnimationComponent"]["DestroyOnComplete"] = uvComp->IsDestroyOnComplete();
    }

    // 15. ParticleEmitterComponent
    ParticleEmitterComponent* peComp = obj->GetComponent<ParticleEmitterComponent>();
    if (peComp)
    {
        comps["ParticleEmitterComponent"]["ParticlePrefab"] = peComp->GetParticlePrefab();
        comps["ParticleEmitterComponent"]["BurstCount"] = peComp->GetBurstCount();
        comps["ParticleEmitterComponent"]["SpawnInterval"] = peComp->GetSpawnInterval();
        comps["ParticleEmitterComponent"]["BurstOnStart"] = peComp->GetBurstOnStart();
        DirectX::XMFLOAT3 pScale = peComp->GetParticleScale();
        comps["ParticleEmitterComponent"]["ParticleScale"] = { pScale.x, pScale.y, pScale.z };
    }

    root["Components"] = comps;

    std::ofstream outFile(filepath);
    if (!outFile.is_open())
    {
        OutputDebugStringA(("[PrefabSerializer] Failed to open file for writing: " + filepath + "\n").c_str());
        return false;
    }

    outFile << root.dump(2);
    outFile.close();

    OutputDebugStringA(("[PrefabSerializer] Successfully saved prefab to: " + filepath + "\n").c_str());
    return true;
}
