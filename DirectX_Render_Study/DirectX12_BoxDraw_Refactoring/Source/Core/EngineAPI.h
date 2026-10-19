#pragma once

#if defined(GAMELOGIC_DLL_BUILD)
    #define ENGINE_API __declspec(dllimport)
#else
    #define ENGINE_API __declspec(dllexport)
#endif
