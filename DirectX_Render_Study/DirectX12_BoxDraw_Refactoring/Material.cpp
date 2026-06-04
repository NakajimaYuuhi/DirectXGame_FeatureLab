#include "Material.h"

#include "DX12Manager.h"
#include "TextureManager.h"

//FilePathを取っておくかは要検討
//キャッシュのヒットチェックで使うかもしれない
CMaterial::CMaterial(wstring _FilePath, XMFLOAT4 _Color)
	:m_Color(_Color)
{
    LoadTexture(_FilePath);
}

void CMaterial::LoadTexture(wstring _FilePath)
{
    //デバイス、コマンドリストの取得
    ID3D12Device* device = CDX12Manager::GetInstance().GetDevice();
    ID3D12GraphicsCommandList* cmdList = CDX12Manager::GetInstance().GetCommandLIst();

    // 0. GPUが処理中の場合、コマンドアロケータをリセットするとDevice Removedになるため待機
    CDX12Manager::GetInstance().ForceWait();

    // 1. コマンドリストを開く
    CDX12Manager::GetInstance().GetCommandAllocator()->Reset();
    cmdList->Reset(CDX12Manager::GetInstance().GetCommandAllocator(), nullptr);



    //テクスチャのロード、SRVの作成
    m_Texture = TextureManager::GetInstance().GetTexture(device, cmdList, _FilePath.c_str(), 0);


    // 2.コマンド実行
    cmdList->Close();
    ID3D12CommandList* list[] = { cmdList };
    CDX12Manager::GetInstance().GetCommandQueue()->ExecuteCommandLists(1, list);

    // 3.ここでGPUがコピーを終えるまで、CPUをストップさせる！
    CDX12Manager::GetInstance().ForceWait();
}
