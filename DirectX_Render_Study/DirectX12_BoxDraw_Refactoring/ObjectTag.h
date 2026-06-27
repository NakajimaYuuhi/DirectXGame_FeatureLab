#pragma once

namespace Object
{
	typedef enum
	{
		NONE = -1,

		BACKGROUND,
		PLAYER,
		PLAYER_BULLET,

		ENEMY,
		ENEMY_BULLET,

		FIELD,
		TRIANGLE,

		BILLBOARD,

		EFFECT,
		UI,
		TEXT,
		CAMERA,
		FADE,

		MANAGER,

		NUM,//オブジェクトのタグの最大数

	}objectTag;

}

using ObjectTag = Object::objectTag;