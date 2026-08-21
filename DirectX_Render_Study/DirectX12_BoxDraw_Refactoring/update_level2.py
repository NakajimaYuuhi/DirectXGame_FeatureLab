import re

# ----------------------------------------------------
# 1. Material.h の更新
# ----------------------------------------------------
with open('Material.h', 'r', encoding='shift_jis', errors='ignore') as f:
    mat_h = f.read()

if '#include "BasicSettings.h"' not in mat_h:
    mat_h = mat_h.replace('#include "Texture.h"', '#include "Texture.h"\n#include "BasicSettings.h"')

mat_h = re.sub(
    r'CMaterial\(wstring _FilePath, XMFLOAT4 _Color\);',
    r'CMaterial(wstring _FilePath, XMFLOAT4 _Color, wstring shaderFile = L"Triangle.hlsl", string vsEntry = "VSMain", string psEntry = "PSMain", BlendMode blendMode = BlendMode::Normal);',
    mat_h
)

if 'wstring m_ShaderFile;' not in mat_h:
    add_members = '''
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
'''
    mat_h = mat_h.replace('Color m_Color;\n};', 'Color m_Color;\n' + add_members + '};')

with open('Material.h', 'w', encoding='shift_jis') as f:
    f.write(mat_h)

# ----------------------------------------------------
# 2. Material.cpp の更新
# ----------------------------------------------------
with open('Material.cpp', 'r', encoding='shift_jis', errors='ignore') as f:
    mat_cpp = f.read()

mat_cpp = re.sub(
    r'CMaterial::CMaterial\(wstring _FilePath, XMFLOAT4 _Color\)\s*:\s*m_Color\(_Color\)',
    r'CMaterial::CMaterial(wstring _FilePath, XMFLOAT4 _Color, wstring shaderFile, string vsEntry, string psEntry, BlendMode blendMode)\n\t: m_Color(_Color), m_ShaderFile(shaderFile), m_VsEntry(vsEntry), m_PsEntry(psEntry), m_BlendMode(blendMode)',
    mat_cpp
)

with open('Material.cpp', 'w', encoding='shift_jis') as f:
    f.write(mat_cpp)


# ----------------------------------------------------
# 3. PSOManager.h の更新
# ----------------------------------------------------
with open('PSOManager.h', 'r', encoding='shift_jis', errors='ignore') as f:
    pso_h = f.read()

if '#include "Material.h"' not in pso_h:
    pso_h = pso_h.replace('#pragma once', '#pragma once\n#include "Material.h"\n#include <string>\n#include <unordered_map>')

if 'GetPSO' not in pso_h:
    add_method = '''
	// 動的PSO生成
	ID3D12PipelineState* GetPSO(CMaterial* material, ID3D12RootSignature* rootSig);

private:
	// PSOキャッシュ: キーは "シェーダーパス_ブレンドモード" などの一意の文字列
	std::unordered_map<std::wstring, ComPtr<ID3D12PipelineState>> m_psoCache;
'''
    pso_h = pso_h.replace('void Init(ID3D12Device* device);', 'void Init(ID3D12Device* device);\n' + add_method)

    # 既存の m_meshPipelineState 等はそのまま残しておく（後方互換性のため、または不要なら後で消す）

with open('PSOManager.h', 'w', encoding='shift_jis') as f:
    f.write(pso_h)


# ----------------------------------------------------
# 4. PSOManager.cpp の更新
# ----------------------------------------------------
with open('PSOManager.cpp', 'r', encoding='shift_jis', errors='ignore') as f:
    pso_cpp = f.read()

if 'PSOManager::GetPSO' not in pso_cpp:
    get_pso_impl = r'''
ID3D12PipelineState* PSOManager::GetPSO(CMaterial* material, ID3D12RootSignature* rootSig)
{
    if (!material) return nullptr;

    // キャッシュキーの作成 (シェーダー名 + エントリ + ブレンドモード)
    std::wstring key = material->GetShaderFile() + L"_" + 
                       std::wstring(material->GetVsEntry().begin(), material->GetVsEntry().end()) + L"_" + 
                       std::to_wstring(static_cast<int>(material->GetBlendMode()));

    // キャッシュヒット
    if (m_psoCache.find(key) != m_psoCache.end())
    {
        return m_psoCache[key].Get();
    }

    // なければ作る
    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
    
    auto vs = ShaderManager::GetInstance().GetShader(material->GetShaderFile().c_str(), material->GetVsEntry().c_str(), "vs_5_0");
    auto ps = ShaderManager::GetInstance().GetShader(material->GetShaderFile().c_str(), material->GetPsEntry().c_str(), "ps_5_0");

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    PSOBuilder psoBuilder;
    psoBuilder.SetRootSignature(rootSig)
              .SetInputLayout(inputLayout, _countof(inputLayout))
              .SetShaders(vs->GetBytecode(), ps->GetBytecode())
              .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
              .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT);

    // ブレンドモードに応じた設定
    if (material->GetBlendMode() == BlendMode::Additive)
    {
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_DEPTH_STENCIL_DESC depthDesc = {};
        depthDesc.DepthEnable = TRUE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Z書き込みなし
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        
        psoBuilder.SetBlendState(blendDesc).SetDepthStencilState(depthDesc);
    }

    ComPtr<ID3D12PipelineState> newPso;
    psoBuilder.Build(device, &newPso);
    
    m_psoCache[key] = newPso;
    return newPso.Get();
}
'''
    pso_cpp += get_pso_impl

with open('PSOManager.cpp', 'w', encoding='shift_jis') as f:
    f.write(pso_cpp)


# ----------------------------------------------------
# 5. Mesh.cpp の更新
# ----------------------------------------------------
try:
    with open('Mesh.cpp', 'r', encoding='shift_jis', errors='ignore') as f:
        mesh_cpp = f.read()

    # PSOセット部分の置換
    pattern = r'if\s*\(\s*blendMode\s*==\s*BlendMode::Additive\s*\)\s*\{\s*commandList->SetPipelineState\(PSOManager::GetInstance\(\)\.GetAdditivePipelineState\(\)\);\s*\}\s*else\s*\{\s*commandList->SetPipelineState\(PSOManager::GetInstance\(\)\.GetMeshPipelineState\(\)\);\s*\}'
    replacement = r'''// 動的PSOの取得と設定
    ID3D12PipelineState* pso = PSOManager::GetInstance().GetPSO(material, PSOManager::GetInstance().GetMeshRootSignature());
    commandList->SetPipelineState(pso);'''

    mesh_cpp = re.sub(pattern, replacement, mesh_cpp)

    with open('Mesh.cpp', 'w', encoding='shift_jis', errors='replace') as f:
        f.write(mesh_cpp)
except Exception as e:
    print(f"Error modifying Mesh.cpp: {e}")
