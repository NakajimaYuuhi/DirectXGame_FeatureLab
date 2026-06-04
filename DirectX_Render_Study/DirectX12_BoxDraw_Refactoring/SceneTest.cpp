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

    ////Transform‚Ì“o˜^
    //box->SetTransform({ 0.0f, 2.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
    //
    ////Model‚ÌŽæ“¾

    ////Material‚Ì“o˜^
    //UINT mat_num = model->RegisterMatarial(L"Assets/Texture/Sample2.jpg", {1.0f,1.0f,1.0f,1.0f});
    //
    ////Mesh‚Ì“o˜^
    //model->RegisterMesh(mat_num);

    //m_VecObject.push_back(std::move(box));


    ////--- Ground ---
    //auto ground = std::make_unique<C3D_Object>("Ground");

    //model = ground->GetComponent<CModel>();

    //model->CreateTmpBoneData();

    //ground->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, 0.0f });


    ////Model‚ÌŽæ“¾

    ////Material‚Ì“o˜^
    //mat_num = model->RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

    ////Mesh‚Ì“o˜^
    //model->RegisterMesh(mat_num);

    ////”z—ñ‚É’Ç‰Á
    //m_VecObject.push_back(std::move(ground));

    ////--- Slope ---
    //auto slope = std::make_unique<C3D_Object>("Slope");

    //model = slope->GetComponent<CModel>();


    //model->CreateTmpBoneData();

    //slope->SetTransform({ 0.0f, 0.0f, 10.0f }, { 20.0f, 0.1f, 20.0f }, { -0.8f, 0.0f, 0.0f });


    ////Model‚ÌŽæ“¾

    ////Material‚Ì“o˜^
    //mat_num = model->RegisterMatarial(L"Assets/Texture/Sample3.jpg", { 1.0f,1.0f,1.0f,1.0f });

    ////Mesh‚Ì“o˜^
    //model->RegisterMesh(mat_num);

    ////”z—ñ‚É’Ç‰Á
    //m_VecObject.push_back(std::move(slope));


    //----- Player -----
    //¡‚ÌŠ‰½‚à–³‚µ
    C3D_Object* player = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::PLAYER, "Player"));

    //Žè‘O‚ðˆê’UŒü‚¢‚Ä‚à‚ç‚¤
    player->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });


    //----- Model -----

    //auto model_obj = std::make_unique<C3D_Object>("Model_obj");

    //CModel* model = model_obj->GetComponent<CModel>();

    ////model_obj->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, -1.6f });

    //model->ModelLoad("Assets/Model/OffensiveIdle.glb");


    ////”z—ñ‚É’Ç‰Á
    //m_VecObject.push_back(std::move(model_obj));


    //----- Todo : PlaneƒNƒ‰ƒX‚É‚·‚é -----
    //C3D_Object* Plane = (C3D_Object*) (ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::NONE, "Plane"));

    //CModel* Plane_Model = Plane->GetComponent<CModel>();

    //Plane->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, 0.0f });

    //Plane_Model->ModelLoad("Assets/Model/Plane.glb");

	//Bullet‚Ì¶¬
    C3D_Object* bullet = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::PLAYER_BULLET, "Bullet"));

    bullet->SetTransform({ 0.0f, 1.0f, 0.0f }, { 0.1f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f });

    CModel* Bullet_Model = bullet->GetComponent<CModel>();

    Bullet_Model->ModelLoad("Assets/Model/cube.glb");


}



CSceneTest::~CSceneTest() = default;

void CSceneTest::Init()
{
    //‚»‚ê‚¼‚ê‚ÌƒIƒuƒWƒFƒNƒg‚ð‰Šú‰»
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

    //ƒ‚ƒfƒ‹‚É‚æ‚Á‚ÄDraw‚·‚é
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
