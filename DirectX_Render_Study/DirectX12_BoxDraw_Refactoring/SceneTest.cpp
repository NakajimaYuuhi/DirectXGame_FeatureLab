#include "SceneTest.h"
#include "3D_Object.h"

#include "Object.h"

#include "Model.h"

#include "ObjectManager.h"

CSceneTest::CSceneTest()
{
    ////--- Box ---
    //auto box = std::make_unique<C3D_Object>("Box");


    //CModel* model = box->GetComponent<CModel>();

    //model->CreateTmpBoneData();

    ////TransformÇÃìoò^
    //box->SetTransform({ 0.0f, 2.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
    //
    ////ModelÇÃéÊìæ

    ////MaterialÇÃìoò^
    //UINT mat_num = model->RegisterMatarial(L"Assets/Texture/Sample2.jpg", {1.0f,1.0f,1.0f,1.0f});
    //
    ////MeshÇÃìoò^
    //model->RegisterMesh(mat_num);

    //m_VecObject.push_back(std::move(box));


    ////--- Ground ---
    //auto ground = std::make_unique<C3D_Object>("Ground");

    //model = ground->GetComponent<CModel>();

    //model->CreateTmpBoneData();

    //ground->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, 0.0f });


    ////ModelÇÃéÊìæ

    ////MaterialÇÃìoò^
    //mat_num = model->RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

    ////MeshÇÃìoò^
    //model->RegisterMesh(mat_num);

    ////îzóÒÇ…í«â¡
    //m_VecObject.push_back(std::move(ground));

    ////--- Slope ---
    //auto slope = std::make_unique<C3D_Object>("Slope");

    //model = slope->GetComponent<CModel>();


    //model->CreateTmpBoneData();

    //slope->SetTransform({ 0.0f, 0.0f, 10.0f }, { 20.0f, 0.1f, 20.0f }, { -0.8f, 0.0f, 0.0f });


    ////ModelÇÃéÊìæ

    ////MaterialÇÃìoò^
    //mat_num = model->RegisterMatarial(L"Assets/Texture/Sample3.jpg", { 1.0f,1.0f,1.0f,1.0f });

    ////MeshÇÃìoò^
    //model->RegisterMesh(mat_num);

    ////îzóÒÇ…í«â¡
    //m_VecObject.push_back(std::move(slope));


    //----- Player -----
    //ç°ÇÃèäâΩÇ‡ñ≥Çµ
	auto player = ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::PLAYER, "Player");


    //----- Model -----

    //auto model_obj = std::make_unique<C3D_Object>("Model_obj");

    //CModel* model = model_obj->GetComponent<CModel>();

    ////model_obj->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, -1.6f });

    //model->ModelLoad("Assets/Model/OffensiveIdle.glb");


    ////îzóÒÇ…í«â¡
    //m_VecObject.push_back(std::move(model_obj));


    //----- Todo : PlaneÉNÉâÉXÇ…Ç∑ÇÈ -----
    C3D_Object* Plane = (C3D_Object*) (ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::NONE, "Plane"));

    CModel* Plane_Model = Plane->GetComponent<CModel>();

    Plane->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, 0.0f });

    Plane_Model->ModelLoad("Assets/Model/Plane.glb");

}



CSceneTest::~CSceneTest() = default;

void CSceneTest::Init()
{
    //ÇªÇÍÇºÇÍÇÃÉIÉuÉWÉFÉNÉgÇèâä˙âª
    for (auto& object : m_VecObject)
    {
        object->Init();
    }
}

void CSceneTest::Update() 
{
	ObjectManager::GetInstance().Update(Scene::ID::NONE);

    for (auto& object : m_VecObject)
    {
        object->Update();
    }

    for (auto& object : m_VecObject)
    {
        object->LateUpdate();
    }
}

void CSceneTest::Draw() 
{

    //ÉÇÉfÉãÇ…ÇÊÇ¡ÇƒDrawÇ∑ÇÈ
    for (auto& object : m_VecObject)
    {
        object->Draw();
    }
	ObjectManager::GetInstance().Draw(Scene::ID::NONE);
}

void CSceneTest::AddObject(UniquePtr<CObject> _Object)
{
    m_VecObject.push_back(std::move(_Object));
}
