#pragma once
#include "Event.h"
#include "EventData.h"
#include "ContainerAlias.h"



class EventManager
{
public:

	//----- 基本の処理 -----
	void Init(void) {}		//初期化
	void Uninit(void) {}	//終了処理
	void Update(void) {}	//更新処理

	//----- イベントの管理 -----
	//追加
	void AddEvent(Event _event){eventList.push_back(_event);}

	//取得
	const Vector<Event>& GetEventList() const{return eventList;}

	//クリア
	void ClearEvents(){eventList.clear();}

	//削除(インデックス)
	void RemoveEvent(int _index){eventList.erase(eventList.begin() + _index);}

	//削除



	// イベントの検索
	Event* FindEventByID(Events::ID _ID);

private:
	Vector<Event> eventList;

//----- シングルトンの実装に必要 -----
public:
	static EventManager& GetInstance()
	{
		static EventManager Instance;

		//インスタンスを返す
		return Instance;
	}

private:
	//コンストラクタ
	EventManager()=default;

	//デストラクタ
	~EventManager() = default;

	//コピー禁止
	EventManager(const EventManager&) = delete;

	//代入禁止
	EventManager& operator=(const EventManager&) = delete;
};

