//===== インクルード =====
#pragma once
#include "EventEnums.h"
#include "EventData.h"


//===== クラス定義 =====
class Event
{
public:
	//コンストラクタ
	Event() = default;
	//仮想デストラクタ
	virtual ~Event() = default;

	//----- Getter,Setter -----
	//EventID
	Events::ID GetEventID() const{return iD;}
	void SetEventID		(Events::ID eventID)	{iD = eventID;}

	//EventData
	EventData* GetEventData() {return &eventData;}
	void SetEventData(EventData _EventData) { eventData = _EventData; }	

private:
	Events::ID iD;
	EventData eventData;
};

