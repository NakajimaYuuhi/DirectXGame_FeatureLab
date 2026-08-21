#pragma once
#include "Scene.h"
#include "SceneEnums.h"
#include "SmartPtrAlias.h"

class SceneManager
{
public:
	void Init(void);	//初期化、最初のシーンの格納
	void Uninit(void);	//終了処理、シーンのUninit
	void Update(void);	//シーンの更新処理を呼び出し
	void Draw(void);	//シーンの表示処理を呼び出し


	//----- シーンの切替 -----
	void PushScene(Scenes::ID _SceneID);
	void PopScene(void);

private:
	UniquePtr<CScene> scene;

	//ゲームの終了フラグ
	bool IsGameEnd = false;

private:
	void ChangeScene(Scenes::ID _SceneID);
	void InstantiateScene(Scenes::ID _SceneID);//シーンをインスタンス化
	void UninitAndPop();
	void ProcessSceneEvents();

//----- Getter,Setter -----
private:
	bool GetIsGameEnd(void){return IsGameEnd;}
	
	//仮実装
	CScene* GetScene(){return scene.get();}

	bool IsExistScene(Scenes::ID _SceneID){return _SceneID != scene->GetID();}

	Scenes::ID GetCurrentSceneID(void){return scene->GetID();}


//----- シングルトンの実装に必要 -----
public:
	static SceneManager& GetInstance()
	{
		static SceneManager Instance;

		//インスタンスを返す
		return Instance;
	}

private:
	//コンストラクタ
	SceneManager();

	//デストラクタ
	~SceneManager();

	//コピー禁止
	SceneManager(const SceneManager&) = delete;

	//代入禁止
	SceneManager& operator=(const SceneManager&) = delete;
};

