#pragma once

namespace Object
{
	typedef enum
	{
		NONE = -1,

		BACKGROUND,
		PLAYER,

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

		NUM,//オブジェクトのタグの最大数

	}objectTag;

}

using ObjectTag = Object::objectTag;