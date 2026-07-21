#pragma once
#include <d3d12.h>

class RenderTexture {
public:
    // 初期化時にサイズ、フォーマット、および登録先の SRVヒープ/RTVヒープ の情報を渡す想定
    RenderTexture(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT format);
    ~RenderTexture();

    // 状態遷移バリアを張る（現在の状態と nextState が同じなら何もしない）
    void Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES nextState);

    // リソースとハンドルの取得
    ID3D12Resource* GetResource() const { return m_pResource; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_rtvHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const { return m_srvHandle; }

private:
    ID3D12Resource* m_pResource = nullptr;
    D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;

    // 各種ハンドル
    // ※設計方針に則り、SRVは既存のメインヒープに登録、RTVは専用の固定ヒープから割り当てる想定
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvHandle{};

    // (必要に応じて) ヒープ内のインデックス等も保持
};