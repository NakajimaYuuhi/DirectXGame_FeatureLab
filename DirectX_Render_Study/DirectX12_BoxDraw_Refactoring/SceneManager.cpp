//===== インクルード =====
#include "SceneManager.h"

#include "ObjectManager.h"
//シーン
#include "Scene.h"
#include "SceneTest.h"
#include "SceneTitle.h"
#include "ButtonEventManager.h"


#include "InputManager.h"

#include "EventManager.h"
#include "Event.h"
#include "EventData_NextScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "DX12Manager.h"



//===== 定数・マクロ定義 =====


//===== 関数の定義 =====

//----- 初期化、終了処理 -----

//コンストラクタ
SceneManager::SceneManager(void)
{
	Init();

	EventManager::GetInstance().Init();

}

//デストラクタ
SceneManager::~SceneManager(void)
{

	//Sceneを出るときにやるからいらない
	//delete m_pSceneStack;
	//m_pSceneStack = nullptr;
}

//初期化処理
void SceneManager::Init()
{
	//最初のシーンの生成
	InstantiateScene(INITIAL_SCENE);//シーンの開始処理があるから、その前に必要なクラスを作成、初期化する必要がある

}

//終了処理
void SceneManager::Uninit(void)
{
	// シーン内のオブジェクトやリソースをすべて破棄する
	UninitAndPop();
	// 現在のシーン自体を破棄する
	if (scene) {
		scene.reset();
	}
}


//----- 更新処理,描画処理 -----

//更新処理
void SceneManager::Update()
{
	//EventManagerのシーンイベント処理
	//シーンの切替、追加、削除をここで行う
	ProcessSceneEvents();

	//Todo : Root処理を入れる
	if (scene)

		//シーンの更新処理
		scene->Update();
}

//描画処理
void SceneManager::Draw(void)
{
	// 破棄されたオブジェクトの遅延削除（GPUの描画完了待機後に実行されるため安全）
	ObjectManager::GetInstance().FlushDestroyedObjects();

	//Todo : Root処理を入れる
	if (scene)
		scene->Draw();
}

//----- シーンの管理 -----

//シーンの生成
void SceneManager::InstantiateScene(Scenes::ID _SceneID)
{
	//TODO : Factoryでやるのがベスト
	//TODO : Mapを使うのがベスト

	//一旦ここに描いただけ(後で消す)
	CScene* AdditionalScene = nullptr;

	//条件で分けてシーンの作成
	switch (_SceneID)
	{
	case Scenes::ID::TEST:
		scene = std::make_unique<CSceneTest>();
		break;

	case Scenes::ID::TITLE:
		scene = std::make_unique<SceneTitle>();
		break;

	}

	//シーンの初期化
	scene->Init();

}

//シーンの切替
void SceneManager::ChangeScene(Scenes::ID _SceneID)
{
	//----- ゲームの終了時 -----
	if (_SceneID == Scenes::ID::Exit)
	{
		//終了処理 (中のシーンも空にしてくれる)(Uninitも呼んでる


		//終了フラグを立てる
		IsGameEnd = true;

		return;
	}

	//----- シーンの移動 -----
	//シーンの終了、削除
	UninitAndPop();

	//シーンを追加する
	InstantiateScene(_SceneID);

}

void SceneManager::PushScene(Scenes::ID _SceneID)
{
	//シーンを追加する
	InstantiateScene(_SceneID);
}

void SceneManager::PopScene(void)
{
	//シーンの終了、削除
	UninitAndPop();
}

//シーンの終了、ポップ
void SceneManager::UninitAndPop(void)
{
	DX12Manager::GetInstance().WaitForPendingOperations();
	ObjectManager::GetInstance().Uninit();
	TextureManager::GetInstance().Clear();
	ModelManager::GetInstance().Clear();
	DX12Manager::GetInstance().ResetSrvNextIndex();

	// Clear UI selection state of previous scene
	ButtonEventManager::GetInstance().ClearSelectedGameObject();
}

//シーンイベントの処理
void SceneManager::ProcessSceneEvents()
{

	// Scene関連のイベントID一覧
	static const Events::ID sceneEventIDs[] = {
		Events::ID::ChangeScene,
		Events::ID::PushScene,
		Events::ID::PopScene
	};


	// --- Scene関連イベントをまとめて検索 ---
	Event* pEvent = nullptr;// イベントポインタ

	//処理対象のイベントが有るか確認
	for (auto id : sceneEventIDs)
	{
		pEvent = EventManager::GetInstance().FindEventByID(id);
		if (pEvent) break;
	}

	// イベントなし
	if (!pEvent) return;

	switch (pEvent->GetEventID())
	{
		case Events::ID::ChangeScene:

			//シーンの切替
			ChangeScene(
				(static_cast<EventData_NextScene*>(pEvent->GetEventData())->GetNextScene())
			);
			break;
	}

	EventManager::GetInstance().ClearEvents();
}



//----- Getter -----

//終了フラグ
