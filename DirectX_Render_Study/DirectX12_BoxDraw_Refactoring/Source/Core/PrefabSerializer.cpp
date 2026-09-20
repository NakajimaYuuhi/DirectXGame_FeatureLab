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
#include "Source/External/json.hpp"
#include <fstream>
#include <filesystem>
#include <windows.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

bool PrefabSerializer::SavePrefab(const std::string& filepath, CObject* obj)
{
    if (!obj) return false;

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
        case ObjectTag::PLAYER:        tagStr = "PLAYER"; break;
        case ObjectTag::ENEMY:         tagStr = "ENEMY"; break;
        case ObjectTag::PLAYER_BULLET: tagStr = "PLAYER_BULLET"; break;
        case ObjectTag::ENEMY_BULLET:  tagStr = "ENEMY_BULLET"; break;
        case ObjectTag::BACKGROUND:    tagStr = "BACKGROUND"; break;
        case ObjectTag::FIELD:         tagStr = "FIELD"; break;
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
        DirectX::XMFLOAT3 scale = transform->GetScale();
        comps["Transform"]["Scale"] = { scale.x, scale.y, scale.z };
    }

    // 2. Model
    CModel* model = obj->GetComponent<CModel>();
    if (model)
    {
        std::string modelPath = model->GetModelPath();
        if (modelPath.empty()) modelPath = "Assets/Model/Wizard.glb";
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
        comps["EnemyAI"] = json::object();
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
