#pragma once
#include "Collider.h"
#include <DirectXMath.h>

class Collider3D :
    public Collider
{
public:
    Collider3D() {}
	virtual ~Collider3D() = default;
   
    //WorldPos??èÔ
    virtual DirectX::XMFLOAT3 GetWorldPos() = 0;

	//?I?t?Z?b?g????
	void SetOffset(DirectX::XMFLOAT3 _Offset) { Offset = _Offset; }
	//?I?t?Z?b?g??èÔ
	DirectX::XMFLOAT3 GetOffset() { return Offset; }

protected:
    //?I?t?Z?b?g
	DirectX::XMFLOAT3 Offset = { 0.0f,0.0f,0.0f };


};

