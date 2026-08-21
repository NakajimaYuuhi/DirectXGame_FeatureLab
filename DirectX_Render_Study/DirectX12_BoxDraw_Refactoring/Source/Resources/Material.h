//一旦Texture,Colorを持つ
//頂点ごとに色を持つ

//Material
#pragma once
#include <DirectXMath.h>
#include "StringAlias.h"	//文字列
#include "SmartPtrAlias.h"	//スマートポインタ

#include "Texture.h"
#include "BasicSettings.h"

//===== エイリアス =====
using XMFLOAT4 = DirectX::XMFLOAT4;

using Color = XMFLOAT4;

using pTexture = SharedPtr<CTexture>;



class CMaterial
{
public:
	CMaterial(wstring _FilePath, XMFLOAT4 _Color, wstring shaderFile = L"Assets/Shader/Triangle.hlsl", string vsEntry = "VSMain", string psEntry = "PSMain", BlendMode blendMode = BlendMode::Opaque);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle()
	{
		return m_Texture->GetGpuHandle();
	}

	//仮で置いておく
	void LoadTexture(wstring _FilePath);

private:
	//Texture
	pTexture m_Texture;
	//color
	Color m_Color;

	// Shader Settings
	wstring m_ShaderFile;
	string m_VsEntry;
	string m_PsEntry;
	BlendMode m_BlendMode;

public:
	wstring GetShaderFile() const { return m_ShaderFile; }
	string GetVsEntry() const { return m_VsEntry; }
	string GetPsEntry() const { return m_PsEntry; }
	BlendMode GetBlendMode() const { return m_BlendMode; }
};
