#include "Explosion.h"
#include "Model.h"
#include "ObjectInfo.h"

Explosion::Explosion(String _Name)
	:BillBoard(_Name)
{

	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::EFFECT);

	CModel* model = GetComponent<CModel>();

	model->SetBlendMode(BlendMode::Additive);

	model->SetMaterialTexture(L"Assets/Texture/explosion.png");

	CTransform* billboardTransform = GetComponent<CTransform>();
	billboardTransform->SetUVOffset({ 0.0f, 0.0f });
	billboardTransform->SetUVScale({ 1.0f / 4.0f, 1.0f / 4.0f });


}

void Explosion::Update()
{
	//寿命が尽きたら消える
	if (LifeTime <= 0)
	{
		SetIsDestroyed(true);
		return;
	}

	//更新間隔ごとにフレームを進める
	if (LifeTime % UpdateInterval == 0)
	{
		Frame++;
	}

	//アニメーションの更新
	if (Frame >= MaxFrame)
	{
		Frame = MaxFrame-1;
	}

	//UVオフセットの更新
	CTransform* billboardTransform = GetComponent<CTransform>();
	int currentRow = Frame / 4; // 1行に4フレーム
	int currentCol = Frame % 4; // 列はフレーム番号の余り
	billboardTransform->SetUVOffset({ currentCol * 0.25f, currentRow * 0.25f });

	//寿命減少
	LifeTime--;
}