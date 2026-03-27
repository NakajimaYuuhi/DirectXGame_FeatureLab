//一旦Texture,Colorを持つ
//頂点ごとに色を持つ

//Material
#pragma once
#include <DirectXMath.h>
#include "StringAlias.h"	//文字列
#include "SmartPtrAlias.h"	//スマートポインタ

#include "Texture.h"

//===== エイリアス =====
using XMFLOAT4 = DirectX::XMFLOAT4;

using Color = XMFLOAT4;

using pTexture = SharedPtr<CTexture>;



class CMaterial
{
public:
	CMaterial(wstring _FilePath, XMFLOAT4 _Color);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle()
	{
		return m_Texture->GetGpuHandle();
	}

private:
	//Texture
	pTexture m_Texture;
	//color
	Color m_Color;


	//仮で置いておく
	void LoadTexture(wstring _FilePath);
};

