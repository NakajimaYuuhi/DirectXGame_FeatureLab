#pragma once
#include <string>
#include "EventManager.h"
#include "EventData_NextScene.h"
#include "SceneEnums.h"

enum class ButtonAction
{
    None = 0,
    ChangeScene_Test,
    ChangeScene_Title,
    ChangeScene_Clear,
    ChangeScene_Failed,
    ExitGame
};

inline const char* ButtonActionToString(ButtonAction action)
{
    switch (action)
    {
    case ButtonAction::ChangeScene_Test:   return "ChangeScene_Test";
    case ButtonAction::ChangeScene_Title:  return "ChangeScene_Title";
    case ButtonAction::ChangeScene_Clear:  return "ChangeScene_Clear";
    case ButtonAction::ChangeScene_Failed: return "ChangeScene_Failed";
    case ButtonAction::ExitGame:          return "ExitGame";
    default:                               return "None";
    }
}

inline ButtonAction StringToButtonAction(const std::string& str)
{
    if (str == "ChangeScene_Test")   return ButtonAction::ChangeScene_Test;
    if (str == "ChangeScene_Title")  return ButtonAction::ChangeScene_Title;
    if (str == "ChangeScene_Clear")  return ButtonAction::ChangeScene_Clear;
    if (str == "ChangeScene_Failed") return ButtonAction::ChangeScene_Failed;
    if (str == "ExitGame")          return ButtonAction::ExitGame;
    return ButtonAction::None;
}

inline void ExecuteButtonAction(ButtonAction action)
{
    Event event;
    EventData_NextScene* eventData = nullptr;

    switch (action)
    {
    case ButtonAction::ChangeScene_Test:
        eventData = new EventData_NextScene(Scenes::ID::TEST);
        break;
    case ButtonAction::ChangeScene_Title:
        eventData = new EventData_NextScene(Scenes::ID::TITLE);
        break;
    case ButtonAction::ChangeScene_Clear:
        eventData = new EventData_NextScene(Scenes::ID::Clear);
        break;
    case ButtonAction::ChangeScene_Failed:
        eventData = new EventData_NextScene(Scenes::ID::Failed);
        break;
    case ButtonAction::ExitGame:
        eventData = new EventData_NextScene(Scenes::ID::Exit);
        break;
    default:
        return;
    }

    if (eventData)
    {
        event.SetEventData(eventData);
        event.SetEventID(Events::ID::ChangeScene);
        EventManager::GetInstance().AddEvent(event);
    }
}
