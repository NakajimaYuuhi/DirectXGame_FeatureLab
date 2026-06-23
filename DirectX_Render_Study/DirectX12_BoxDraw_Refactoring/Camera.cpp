#include "Camera.h"
#include "ObjectInfo.h"
#include "BasicSettings.h"

Camera::Camera(String _Name)
	:C3D_Object(_Name)
{

	//----- É^ÉO -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::CAMERA);

	//View
	view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0, 2.5, -5, 1),
		DirectX::XMVectorSet(0, 2, 0, 1),
		DirectX::XMVectorSet(0, 1, 0, 0));


	//Proj
	proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		(float)SCREEN_WIDTH / SCREEN_HEIGHT,
		0.1f,
		1000.0f);
}
