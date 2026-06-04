//Transform.h
//  Position,Rotation,Scaleの管理
//  Up,FrontもGetできるようにする
//  TODO : Up,Front,Rightから、角度を再設定できるとベスト
//  TODO : クォータニオンに対応できると良い

//===== インクルード =====
#pragma once
#include <DirectXMath.h>
#include "Component.h"

//===== クラスの定義 =====
class CTransform :public CComponent
{
public:
	//コンストラクタ
	CTransform
	(
		DirectX::XMFLOAT3 _Position = { 0.0f, 0.0f, 0.0f },
		DirectX::XMFLOAT3 _Rotation = { 0.0f, 0.0f, 0.0f },
		DirectX::XMFLOAT3 _Scale	= { 1.0f, 1.0f, 1.0f }
	)
		: CComponent("Transform")//基本はコンポーネントの種類
		, m_Position(_Position)	//位置
		, m_Rotation(_Rotation)	//回転
		, m_Scale	(_Scale)	//スケール
	{
	}

	//デストラクタ
	~CTransform() = default;

	//----- 便利なGetter -----
	//Frontの取得
	DirectX::XMFLOAT3 GetFront()
	{
		//回転行列の作成
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		//回転行列からFrontを取り出す
		DirectX::XMFLOAT3 front;
		front = { rotMat.r[2].m128_f32[0], rotMat.r[2].m128_f32[1], rotMat.r[2].m128_f32[2] };

		return front;
	}

	//Upの取得
	DirectX::XMFLOAT3 GetUp()
	{
		//回転行列の作成
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		//回転行列からFrontを取り出す
		DirectX::XMFLOAT3 up;
		up = { rotMat.r[1].m128_f32[0], rotMat.r[1].m128_f32[1], rotMat.r[1].m128_f32[2] };

		return up;
	}

	DirectX::XMFLOAT3 GetRight()
	{
		//回転行列の作成
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		//回転行列からFrontを取り出す
		DirectX::XMFLOAT3 right;
		right = { rotMat.r[0].m128_f32[0], rotMat.r[0].m128_f32[1], rotMat.r[0].m128_f32[2] };
		return right;
	}

	//World行列の取得
	DirectX::XMMATRIX GetWorld();

	//TODO:Up,Front,Rightから、角度を再設定
	void SetRotationFromUpFront(DirectX::XMFLOAT3 _Up, DirectX::XMFLOAT3 _Front)
	{
		//Rightの計算
		DirectX::XMFLOAT3 right;
		right.x = _Up.y * _Front.z - _Up.z * _Front.y;
		right.y = _Up.z * _Front.x - _Up.x * _Front.z;
		right.z = _Up.x * _Front.y - _Up.y * _Front.x;
		//回転行列の作成
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixIdentity();
		rotMat.r[0] = DirectX::XMVectorSet(right.x, right.y, right.z, 0.0f);
		rotMat.r[1] = DirectX::XMVectorSet(_Up.x, _Up.y, _Up.z, 0.0f);
		rotMat.r[2] = DirectX::XMVectorSet(_Front.x, _Front.y, _Front.z, 0.0f);
		//回転行列からオイラー角を取り出す
		m_Rotation.y = atan2f(rotMat.r[2].m128_f32[0], rotMat.r[2].m128_f32[2]);
		m_Rotation.x = asinf(-rotMat.r[2].m128_f32[1]);
		m_Rotation.z = atan2f(rotMat.r[1].m128_f32[1], rotMat.r[0].m128_f32[1]);
	}

private:
    //位置、回転、スケール
    DirectX::XMFLOAT3 m_Position    = { 0.0f, 0.0f, 0.0f };    //位置
    DirectX::XMFLOAT3 m_Rotation    = { 0.0f, 0.0f, 0.0f };    //回転
    DirectX::XMFLOAT3 m_Scale       = { 1.0f, 1.0f, 1.0f };    //スケール

	//TODO: クォータニオンに対応

	//----- 普通のGetter,Setter -----
public:
	DirectX::XMFLOAT3 GetPos() { return m_Position; }
	void	SetPos(DirectX::XMFLOAT3 _Position) { m_Position = _Position; }

	DirectX::XMFLOAT3	GetScale() { return m_Scale; }
	void	SetScale(DirectX::XMFLOAT3 _Scale) { m_Scale = _Scale; }

	DirectX::XMFLOAT3	GetRotation() { return m_Rotation; }
	void	SetRotation(DirectX::XMFLOAT3 _Rotation) { m_Rotation = _Rotation; }
};

