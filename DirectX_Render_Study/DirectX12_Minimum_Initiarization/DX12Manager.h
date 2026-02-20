//===== インクルード =====
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

//===== クラス定義 =====
class CDX12Manager
{
public:
	//インスタンス取得
    static CDX12Manager& GetInstance();

	// <初期化、終了処理>
    bool Initialize(HWND hwnd);
    void Finalize();

private:
	//DirectX 12関連のメンバ変数
    ComPtr<IDXGIFactory6> m_factory;
    ComPtr<ID3D12Device>  m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;


    //シングルトン実装
private:
    CDX12Manager() = default;
    ~CDX12Manager() = default;

    CDX12Manager(const CDX12Manager&) = delete;
    CDX12Manager& operator=(const CDX12Manager&) = delete;

};

