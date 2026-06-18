#include "EventManager.h"

// イベントの検索
Event* EventManager::FindEventByID(Events::ID _ID)
{
	//一致した最初のイベントを返す
	for (Event& event : eventList)
	{
		if (event.GetEventID() == _ID)
		{
			return &event;
		}
	}
	return nullptr; // 見つからなかった場合
}