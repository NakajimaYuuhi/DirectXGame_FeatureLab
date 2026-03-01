//Mesh.h
//メッシュのクラス

//===== インクルード =====
#pragma once

//----- DirectX12関連 -----
#include <d3d12.h>
#include <DirectXMath.h>

//===== クラス定義 =====
class CMesh
{
public:
	void Initialize(ID3D12Device* _Device);
	void Update();
	void Draw(ID3D12GraphicsCommandList* _CommandList);

private:
	//位置、回転、スケール
    DirectX::XMFLOAT3 m_position	= { 0, 0, 0 };
    DirectX::XMFLOAT3 m_rotation	= { 0, 0, 0 };
    DirectX::XMFLOAT3 m_scale		= { 1, 1, 1 };
};

