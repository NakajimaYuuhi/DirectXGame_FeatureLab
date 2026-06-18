#include "Skydome.h"
#include "Model.h"
#include "ModelManager.h"
#include "Transform.h"
#include "ObjectInfo.h"

Skydome::Skydome(String _Name)
    : C3D_Object(_Name)
{
    // Set Tag
    CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
    if (objectInfo) {
        objectInfo->SetObjectTag(ObjectTag::BACKGROUND);
    }

    // Initialize Model
    CModel* model = GetComponent<CModel>();
    if (model) {
        auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/cube.glb");
        model->CopyFrom(sharedModel);

        // Apply skydome texture
        model->SetMaterialTexture(L"Assets/Texture/skydome.jpg", 0);
    }

    // Set inside-out scaling so we can see the cube from the inside
    CTransform* transform = GetComponent<CTransform>();
    if (transform) {
        transform->SetScale({ -100.0f, 100.0f, 100.0f });
    }
}

Skydome::~Skydome()
{
}

void Skydome::Update()
{
    C3D_Object::Update();
}
