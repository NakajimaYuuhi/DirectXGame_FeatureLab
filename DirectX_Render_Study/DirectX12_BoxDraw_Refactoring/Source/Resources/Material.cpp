#include "Material.h"

#include "DX12Manager.h"
#include "TextureManager.h"

//FilePathを取っておくかは要検討
//キャッシュのヒットチェックで使うかもしれない
CMaterial::CMaterial(wstring _FilePath, XMFLOAT4 _Color, wstring shaderFile, string vsEntry, string psEntry, BlendMode blendMode)
	: m_Color(_Color), m_ShaderFile(shaderFile), m_VsEntry(vsEntry), m_PsEntry(psEntry), m_BlendMode(blendMode)
{
    LoadTexture(_FilePath);
}

void CMaterial::LoadTexture(wstring _FilePath)
{
    //デバイス、コマンドリストの取得
    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
    ID3D12GraphicsCommandList* cmdList = DX12Manager::GetInstance().GetCommandList();

    // 0. GPUが処理中の場合、コマンドアロケータをリセットするとDevice Removedになるため待機
    DX12Manager::GetInstance().ForceWait();

    // 1. コマンドリストを開く
    DX12Manager::GetInstance().GetCommandAllocator()->Reset();
    cmdList->Reset(DX12Manager::GetInstance().GetCommandAllocator(), nullptr);



    //テクスチャのロード、SRVの作成
    m_Texture = TextureManager::GetInstance().GetTexture(device, cmdList, _FilePath.c_str());


    // 2.コマンド実行
    cmdList->Close();
    ID3D12CommandList* list[] = { cmdList };
    DX12Manager::GetInstance().GetCommandQueue()->ExecuteCommandLists(1, list);

    // 3.ここでGPUがコピーを終えるまで、CPUをストップさせる！
    DX12Manager::GetInstance().ForceWait();
}
