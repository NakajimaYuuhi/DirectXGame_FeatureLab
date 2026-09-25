#include "Explosion.h"
#include "Model.h"
#include "ObjectInfo.h"

Explosion::Explosion(String _Name)
	:BillBoard(_Name)
{

	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::EFFECT);

	CModel* model = GetComponent<CModel>();

	// ?????G?t?F?N?g????Z????(Additive)??`????
	model->SetBlendModeAll(BlendMode::Additive);

	model->SetMaterialTexture(L"Assets/Texture/explosion.png");

	CTransform* billboardTransform = GetComponent<CTransform>();
	billboardTransform->SetUVOffset({ 0.0f, 0.0f });
	billboardTransform->SetUVScale({ 1.0f / 4.0f, 1.0f / 4.0f });


}

void Explosion::Update()
{
	//???????s???????????
	if (LifeTime <= 0)
	{
		SetIsDestroyed(true);
		return;
	}

	//?X?V??u?????t???[????i???
	if (LifeTime % UpdateInterval == 0)
	{
		Frame++;
	}

	//?A?j???[?V??????X?V
	if (Frame >= MaxFrame)
	{
		Frame = MaxFrame-1;
	}

	//UV?I?t?Z?b?g??X?V
	CTransform* billboardTransform = GetComponent<CTransform>();
	int currentRow = Frame / 4; // 1?s??4?t???[??
	int currentCol = Frame % 4; // ???t???[???????]??
	billboardTransform->SetUVOffset({ currentCol * 0.25f, currentRow * 0.25f });

	//????????
	LifeTime--;
}