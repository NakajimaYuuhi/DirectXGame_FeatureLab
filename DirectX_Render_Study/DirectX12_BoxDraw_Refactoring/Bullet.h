#pragma once
#include "3D_Object.h"

#include "StringAlias.h"

#include <DirectXMath.h>

class Bullet : public C3D_Object
{
public:
	Bullet(String _Name);
	~Bullet() = default;

	//入力の更新
	virtual void Update();

	void SetDirection(DirectX::XMFLOAT3 _Direction)
	{
		Direction = _Direction;
	}

protected:
	String ModelPath;
	//方向ベクトル
	DirectX::XMFLOAT3 Direction = {1.0f, 0.0f, 0.0f};
	float Speed = 0.01f;
	

};

