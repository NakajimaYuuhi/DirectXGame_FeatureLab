#include "BillBoard.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "DX12Manager.h"
#include "Model.h"
#include "ModelManager.h"

//===== メソッド定義 =====
BillBoard::BillBoard(String _Name)
	:C3D_Object(_Name)
{
	//----- タグ -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::BILLBOARD);

	ModelPath = "Assets/Model/Plane.glb";

	//ステージで使うモデルを一覧にしてどこかでロードしておくといいかも
	//モチE  のローチE
	CModel* model = GetComponent<CModel>();


	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);


}

void BillBoard::Update()
{
}

void BillBoard::LateUpdate()
{
	//Transformの取得
	CTransform* transform = GetComponent<CTransform>();

	//カメラの行列を取得
	DX12Manager& dx12Manager = DX12Manager::GetInstance();
	DirectX::XMMATRIX view = dx12Manager.GetView();

	//----- view行列から、カメラの軸を取り出す -----

	// View行列の回転成分を転置して、各軸を取り出す
	// V = [R U F | T] 形式の場合、その転置はカメラの軸そのものになる
	DirectX::XMMATRIX invView = DirectX::XMMatrixTranspose(view);

	// カメラの軸を取得
	DirectX::XMFLOAT3 camRight = { invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2] };
	DirectX::XMFLOAT3 camUp = { invView.r[1].m128_f32[0], invView.r[1].m128_f32[1], invView.r[1].m128_f32[2] };
	DirectX::XMFLOAT3 camFront = { invView.r[2].m128_f32[0], invView.r[2].m128_f32[1], invView.r[2].m128_f32[2] };

	//----- 軸を元に、ビルボードの回転を設定する -----
	DirectX::XMFLOAT3 up = { camFront.x, camFront.y, camFront.z };
	DirectX::XMFLOAT3 front = { camUp.x, camUp.y, camUp.z };

	transform->SetRotationFromUpFront(up, front);

}
