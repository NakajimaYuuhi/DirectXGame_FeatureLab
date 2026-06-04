#pragma once
#include "Collider.h"
#include <DirectXMath.h>

class Collider3D :
    public Collider
{
public:
    Collider3D() {}
	virtual ~Collider3D() = default;
   
    //WorldPosの取得
    virtual DirectX::XMFLOAT3 GetWorldPos() = 0;

	//オフセットの設定
	void SetOffset(DirectX::XMFLOAT3 _Offset) { Offset = _Offset; }
	//オフセットの取得
	DirectX::XMFLOAT3 GetOffset() { return Offset; }

protected:
    //オフセット
	DirectX::XMFLOAT3 Offset = { 0.0f,0.0f,0.0f };


};

