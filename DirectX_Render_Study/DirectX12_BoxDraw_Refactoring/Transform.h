//Transform.h
//  Position,Rotation,Scaleの管理
//  Up,FrontもGetできるようにする
//  TODO : Up,Front,Rightから、角度を再設定できるとベスト
//  TODO : クォータニオンに対応できると良い

//===== インクルード =====
#pragma once
#include <DirectXMath.h>

//===== クラスの定義 =====
class CTransform
{
public:
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

	//World行列の取得
	DirectX::XMMATRIX GetWorld();

	//TODO:Up,Front,Rightから、角度を再設定

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

