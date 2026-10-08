// ==========================================
// PostProcess.hlsl
// ポストプロセス (ブルーム・グレースケール・セピア・反転・ビネット)
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
    float g_threshold;    // ブルーム閾値
    float g_knee;         // ソフトニー
    float g_intensity;    // ブルーム強度
    float g_spread;       // ブラー分散
    float2 g_direction;   // ブラー方向
    float g_bloomEnabled; // 1.0: 有効, 0.0: 無効
    float g_effectType;   // 0: None, 1: Grayscale, 2: Sepia, 3: Invert, 4: Vignette
};

Texture2D    g_texture0 : register(t0); // メイン入力
Texture2D    g_texture1 : register(t1); // ブルームぼかしテクスチャ
SamplerState g_sampler  : register(s0); // リニアクランプ

// --- エフェクト適用関数 ---
float3 ApplyEffect(float3 col, float2 uv, float effectType)
{
    int type = (int)(effectType + 0.5f);
    if (type == 1)
    {
        // Grayscale
        float gray = dot(col, float3(0.299f, 0.587f, 0.114f));
        return float3(gray, gray, gray);
    }
    else if (type == 2)
    {
        // Sepia
        float r = dot(col, float3(0.393f, 0.769f, 0.189f));
        float g = dot(col, float3(0.349f, 0.686f, 0.168f));
        float b = dot(col, float3(0.272f, 0.534f, 0.131f));
        return saturate(float3(r, g, b));
    }
    else if (type == 3)
    {
        // Invert
        return saturate(1.0f - col);
    }
    else if (type == 4)
    {
        // Vignette
        float2 d = uv - float2(0.5f, 0.5f);
        float dist = length(d);
        float vig = smoothstep(0.75f, 0.25f, dist);
        return col * vig;
    }
    return col;
}

// --- パススルー ---
float4 PSPassThrough(VSOutput input) : SV_TARGET {
    float4 col = g_texture0.Sample(g_sampler, input.uv);
    col.rgb = ApplyEffect(col.rgb, input.uv, g_effectType);
    return float4(col.rgb, 1.0f);
}

// vcxproj デフォルトエントリポイント
float4 PSMain(VSOutput input) : SV_TARGET {
    return PSPassThrough(input);
}

// --- 輝度抽出 (Bright Pass) ---
float4 PSBrightPass(VSOutput input) : SV_TARGET {
    float4 color = g_texture0.Sample(g_sampler, input.uv);
    
    float lum = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    float knee = max(1e-4f, g_threshold * g_knee);
    float soft = lum - g_threshold + knee;
    soft = clamp(soft, 0.0f, 2.0f * knee);
    soft = (soft * soft) / (4.0f * knee + 1e-4f);
    
    float contribution = max(soft, lum - g_threshold);
    contribution /= max(lum, 1e-4f);
    
    float3 bright = color.rgb * max(0.0f, contribution);
    return float4(bright, 1.0f);
}

// --- ガウスブラー ---
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

// --- 最終合成 ---
float4 PSComposite(VSOutput input) : SV_TARGET {
    float4 sceneColor = g_texture0.Sample(g_sampler, input.uv);
    float3 finalColor = sceneColor.rgb;
    
    if (g_bloomEnabled > 0.5f)
    {
        float3 bloomColor = g_texture1.Sample(g_sampler, input.uv).rgb;
        finalColor += bloomColor * g_intensity;
    }
    
    finalColor = ApplyEffect(finalColor, input.uv, g_effectType);
    return float4(finalColor, 1.0f);
}
