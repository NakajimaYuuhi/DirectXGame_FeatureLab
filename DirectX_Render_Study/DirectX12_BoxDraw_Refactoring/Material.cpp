#include "Material.h"

#include "DX12Manager.h"

//FilePathを取っておくかは要検討
//キャッシュのヒットチェックで使うかもしれない
CMaterial::CMaterial(wstring _FilePath, XMFLOAT4 _Color)
	:m_Color(_Color)
{
	m_Texture = std::make_shared<CTexture>();

    
    LoadTexture(_FilePath);
	
}

void CMaterial::LoadTexture(wstring _FilePath)
{
    //理想は、コマンド開いた状態で行う
    //毎回、コマンドの開閉、GPUの終了待ちをしてると大変
    //コマンドが開いていなければ、開く、とかまでできたら最高

    //--一旦キャッシュを考慮せずにロードする

    //デバイス、コマンドリストの取得
    ID3D12Device* device = CDX12Manager::GetInstance().GetDevice();
    ID3D12GraphicsCommandList* cmdList = CDX12Manager::GetInstance().GetCommandLIst();


    // 1. コマンドリストを開く
    CDX12Manager::GetInstance().GetCommandAllocator()->Reset();
    cmdList->Reset(CDX12Manager::GetInstance().GetCommandAllocator(), nullptr);



    //テクスチャのロード、SRVの作成
    m_Texture->LoadTexture(device, cmdList, _FilePath.c_str(), 0);
    m_Texture->CreateSRV(device);


    // 2.コマンド実行
    cmdList->Close();
    ID3D12CommandList* list[] = { cmdList };
    CDX12Manager::GetInstance().GetCommandQueue()->ExecuteCommandLists(1, list);

    // 3.ここでGPUがコピーを終えるまで、CPUをストップさせる！
    CDX12Manager::GetInstance().ForceWait();
}
