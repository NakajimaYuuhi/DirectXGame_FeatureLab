#include "Collision.h"

Collision::Collision()
{
	//中身の設定
	//外部ファイルから読み込めたらいいのかな
	CollisionOrder = 
	{
		{ ObjectTag::PLAYER, ObjectTag::ENEMY_BULLET },
		{ ObjectTag::ENEMY, ObjectTag::PLAYER_BULLET },
	};
}
