#pragma once
#include "3D_Object.h"
#include "StringAlias.h"

class Camera : public C3D_Object
{
public:
	Camera(String _Name);
	~Camera() = default;

	DirectX::XMMATRIX GetView() { return view; }
	DirectX::XMMATRIX GetProj() { return proj; }

private:
	//ˆÊ’u‚ÍTransform‚Å‚Â
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;

};

