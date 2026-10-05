// ==========================================
// PostProcess.hlsl
// ポストプロセス (ブルーム高輝度抽出・ブラー・合成)
// ==========================================

struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

// --- フルスクリーン三角形頂点シェーダー ---
VSOutput VSMain(uint vertexID : SV_VertexID) {
    VSOutput output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

// --- ルート定数 (8 DWORD / 32 bytes) ---
cbuffer PostProcessCB : register(b0)
{
    float g_threshold;    // ブルーム抽出閾値 (例: 0.8)
    float g_knee;         // ソフトニー幅 (例: 0.2)
    float g_intensity;    // ブルーム合成強度 (例: 1.2)
    float g_spread;       // ブラー拡散係数 (例: 1.0)
    float2 g_direction;   // ブラー方向 (水平: 1/w, 0 / 垂直: 0, 1/h)
    float g_bloomEnabled; // 1.0: 有効, 0.0: 無効
    float g_padding;
};

Texture2D    g_texture0 : register(t0); // メイン/入力テクスチャ
Texture2D    g_texture1 : register(t1); // ブルームブラーテクスチャ (Composite用)
SamplerState g_sampler  : register(s0); // リニアクランプサンプラー

// --- パススルー (単なる画面転送) ---
float4 PSPassThrough(VSOutput input) : SV_TARGET {
    return g_texture0.Sample(g_sampler, input.uv);
}

// vcxproj のデフォルトビルド用エントリーポイント
float4 PSMain(VSOutput input) : SV_TARGET {
    return PSPassThrough(input);
}

// --- 高輝度抽出 (Bright Pass) ---
float4 PSBrightPass(VSOutput input) : SV_TARGET {
    float4 color = g_texture0.Sample(g_sampler, input.uv);
    
    // RGBの輝度 (Luminance) を計算
    float lum = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    
    // ソフトニーによる滑らかな閾値抽出
    float knee = max(1e-4f, g_threshold * g_knee);
    float soft = lum - g_threshold + knee;
    soft = clamp(soft, 0.0f, 2.0f * knee);
    soft = (soft * soft) / (4.0f * knee + 1e-4f);
    
    float contribution = max(soft, lum - g_threshold);
    contribution /= max(lum, 1e-4f);
    
    float3 bright = color.rgb * max(0.0f, contribution);
    return float4(bright, 1.0f);
}

// --- ガウスブラー (バイリニア5タップサンプリング / 9テクセル相当) ---
float4 PSBlurPass(VSOutput input) : SV_TARGET {
    static const float c_weights[3] = { 0.2270270270f, 0.3162162162f, 0.0702702703f };
    static const float c_offsets[3] = { 0.0f, 1.3846153846f, 3.2307692308f };
    
    float2 offset0 = g_direction * (c_offsets[0] * g_spread);
    float3 result = g_texture0.Sample(g_sampler, input.uv).rgb * c_weights[0];
    
    [unroll]
    for (int i = 1; i < 3; ++i)
    {
        float2 offset = g_direction * (c_offsets[i] * g_spread);
        result += g_texture0.Sample(g_sampler, input.uv + offset).rgb * c_weights[i];
        result += g_texture0.Sample(g_sampler, input.uv - offset).rgb * c_weights[i];
    }
    
    return float4(result, 1.0f);
}

// --- 最終合成 (Composite Pass: Scene + Bloom -> BackBuffer) ---
float4 PSComposite(VSOutput input) : SV_TARGET {
    float4 sceneColor = g_texture0.Sample(g_sampler, input.uv);
    
    if (g_bloomEnabled > 0.5f)
    {
        float3 bloomColor = g_texture1.Sample(g_sampler, input.uv).rgb;
        float3 finalColor = sceneColor.rgb + bloomColor * g_intensity;
        
        // わずかな白飛び防止クランプ/サチュレーション
        return float4(finalColor, sceneColor.a);
    }
    
    return sceneColor;
}