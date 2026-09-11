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

		FADE,

		Exit,

		NUM,//ÉVÅ[ÉìÇÃç≈ëÂêî
	}ID;

}