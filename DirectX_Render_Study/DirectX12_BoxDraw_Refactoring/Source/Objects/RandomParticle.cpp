#include "RandomParticle.h"

RandomParticle::RandomParticle(String _Name)
	:BillBoard(_Name)
{
	//•ûŒü‚Íƒ‰ƒ“ƒ_ƒ€‚É
	Direction.x = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f; // -1.0f ~ 1.0f
	Direction.y = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f; // -1.0f ~ 1.0f
	Direction.z = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f; // -1.0f ~ 1.0f
}

void RandomParticle::Update()
{
	//õ–½‚ªs‚«‚½‚çÁ‚¦‚é
	if (LifeTime <= 0)
	{
		SetIsDestroyed(true);
		return;
	}
	//ˆÚ“®
	CTransform* transform = GetComponent<CTransform>();
	DirectX::XMFLOAT3 pos = transform->GetPos();
	pos.x += Direction.x * Speed;
	pos.y += Direction.y * Speed;
	pos.z += Direction.z * Speed;
	transform->SetPos(pos);
	//õ–½Œ¸­
	LifeTime--;
}
