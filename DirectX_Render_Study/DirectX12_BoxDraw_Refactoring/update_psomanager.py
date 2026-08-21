import re

with open('PSOManager.cpp', 'r', encoding='shift_jis', errors='ignore') as f:
    text = f.read()

# 1. 冒頭のインクルード
if '#include "ShaderManager.h"' not in text:
    text = '#include "ShaderManager.h"\n' + text

# 2. シェーダーコンパイル部分の置き換え
pattern1 = r'ComPtr<ID3DBlob> vertexShader;.*?// =========================================================\n\s*//\s*2\.'
replacement1 = r'''auto vertexShader = ShaderManager::GetInstance().GetShader(L"Triangle.hlsl", "VSMain", "vs_5_0");
    auto pixelShader = ShaderManager::GetInstance().GetShader(L"Triangle.hlsl", "PSMain", "ps_5_0");
    auto spriteVertexShader = ShaderManager::GetInstance().GetShader(L"Sprite.hlsl", "VSMain", "vs_5_0");
    auto spritePixelShader = ShaderManager::GetInstance().GetShader(L"Sprite.hlsl", "PSMain", "ps_5_0");

    // =========================================================
    //  2.'''
text = re.sub(pattern1, replacement1, text, flags=re.DOTALL)

# 3. PSO生成時のバイトコード設定の置き換え
text = text.replace(
    'psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };',
    'psoDesc.VS = vertexShader->GetBytecode();'
)
text = text.replace(
    'psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };',
    'psoDesc.PS = pixelShader->GetBytecode();'
)
text = text.replace(
    'spritePsoDesc.VS = { spriteVertexShader->GetBufferPointer(), spriteVertexShader->GetBufferSize() };',
    'spritePsoDesc.VS = spriteVertexShader->GetBytecode();'
)
text = text.replace(
    'spritePsoDesc.PS = { spritePixelShader->GetBufferPointer(), spritePixelShader->GetBufferSize() };',
    'spritePsoDesc.PS = spritePixelShader->GetBytecode();'
)

with open('PSOManager.cpp', 'w', encoding='shift_jis') as f:
    f.write(text)
