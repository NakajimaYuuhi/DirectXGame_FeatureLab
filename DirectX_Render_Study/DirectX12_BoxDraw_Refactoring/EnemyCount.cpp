#include "EnemyCount.h"


EnemyCount::EnemyCount(String _Name)
{
	SetName(_Name);


}

void EnemyCount::UpdateText(int num_)
{
	SetText(BeginText + std::to_wstring(num_));
}
