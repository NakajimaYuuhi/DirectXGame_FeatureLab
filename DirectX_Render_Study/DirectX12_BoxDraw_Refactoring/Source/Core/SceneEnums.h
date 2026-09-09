#pragma once

namespace Scenes
{
#define INITIAL_SCENE Scenes::ID::TITLE

	typedef enum
	{
		NONE = -1,
		TITLE,
		GAME,
		RESULT,

		Clear,
		Failed,

		TEST,

		Exit,

		NUM,//ƒV[ƒ“‚ÌÅ‘å”
	}ID;

}