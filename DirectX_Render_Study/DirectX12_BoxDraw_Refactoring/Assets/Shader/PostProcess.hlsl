// ==========================================
// PostProcess.hlsl
// 画面全体を覆うポリゴンを描画し、テクスチャをモノクロにして出力する
// ==========================================


struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

// --- 頂点シェーダー ---
// 頂点バッファを一切使わず、3つの頂点ID(0,1,2)だけで画面全体を覆う巨大な三角形を作ります
VSOutput VSMain(uint vertexID : SV_VertexID) {
    VSOutput output;
    
    // vertexID から UV座標を生成 (0,0), (2,0), (0,2)
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    
    // UV座標からクリップ空間の座標を生成 (-1~1)
    output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return output;
}

// --- ピクセルシェーダー ---
Texture2D    g_texture : register(t0); // オフスクリーンテクスチャ
SamplerState g_sampler : register(s0); // サンプラー

float4 PSMain(VSOutput input) : SV_TARGET {
    // 1. テクスチャのサンプリング
    float4 color = g_texture.Sample(g_sampler, input.uv);
    
    // 2. モノクロ化 (RGBの輝度(Luminance)を計算)
    float luminance = dot(color.rgb, float3(0.299f, 0.587f, 0.114f));
    
    // 3. モノクロカラーを出力 (アルファはそのまま)
    return float4(luminance, luminance, luminance, color.a);
}
