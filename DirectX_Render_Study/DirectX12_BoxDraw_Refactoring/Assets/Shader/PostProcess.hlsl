// ==========================================
// PostProcess.hlsl
// ポストプロセス (ブルーム・トーンマッピング・露出・色調フィルタ・色収差・アウトライン)
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

// --- ルート定数 (16 DWORD / 64 bytes) ---
cbuffer PostProcessCB : register(b0)
{
    float g_threshold;           // ブルーム閾値
    float g_knee;                // ソフトニー
    float g_intensity;           // ブルーム強度
    float g_spread;              // ブラー分散
    float2 g_direction;          // ブラー方向
    float g_bloomEnabled;        // 1.0: 有効, 0.0: 無効
    float g_effectType;          // 0: None, 1: Grayscale, 2: Sepia, 3: Invert, 4: Vignette
    float g_chromaticAberration; // 色収差強度 (0.0: 無効)
    float g_outlineIntensity;    // アウトライン強度 (0.0: 無効)
    float g_outlineThreshold;    // アウトライン閾値
    float g_outlineWidth;        // アウトライン幅
    float2 g_screenSize;         // 画面解像度 (幅, 高さ)
    float g_toneMapType;         // 0: None, 1: Reinhard, 2: ACES Filmic
    float g_exposure;            // 露出倍率 (1.0: デフォルト)
};

Texture2D    g_texture0 : register(t0); // メイン入力
Texture2D    g_texture1 : register(t1); // ブルームぼかしテクスチャ
SamplerState g_sampler  : register(s0); // リニアクランプ

// --- 色収差サンプリング関数 ---
float3 SampleWithChromaticAberration(Texture2D tex, SamplerState smp, float2 uv, float intensity)
{
    if (intensity <= 0.0001f)
    {
        return tex.Sample(smp, uv).rgb;
    }
    // 画面中心 (0.5, 0.5) からの距離に応じた放射状の色ズレ
    float2 toCenter = uv - float2(0.5f, 0.5f);
    float distSq = dot(toCenter, toCenter);
    float2 offset = toCenter * (distSq * intensity * 2.0f);

    float r = tex.Sample(smp, uv + offset).r;
    float g = tex.Sample(smp, uv).g;
    float b = tex.Sample(smp, uv - offset).b;
    return float3(r, g, b);
}

// --- エッジ検出 (Sobel Filter による輪郭線抽出) ---
float CalculateEdge(Texture2D tex, SamplerState smp, float2 uv, float2 screenSize, float width, float threshold)
{
    float2 texelSize = (width / max(screenSize, float2(1.0f, 1.0f)));

    float3 c00 = tex.Sample(smp, uv + float2(-texelSize.x, -texelSize.y)).rgb;
    float3 c01 = tex.Sample(smp, uv + float2(0.0f,         -texelSize.y)).rgb;
    float3 c02 = tex.Sample(smp, uv + float2( texelSize.x, -texelSize.y)).rgb;
    float3 c10 = tex.Sample(smp, uv + float2(-texelSize.x,  0.0f)).rgb;
    float3 c12 = tex.Sample(smp, uv + float2( texelSize.x,  0.0f)).rgb;
    float3 c20 = tex.Sample(smp, uv + float2(-texelSize.x,  texelSize.y)).rgb;
    float3 c21 = tex.Sample(smp, uv + float2(0.0f,          texelSize.y)).rgb;
    float3 c22 = tex.Sample(smp, uv + float2( texelSize.x,  texelSize.y)).rgb;

    float3 lumW = float3(0.299f, 0.587f, 0.114f);
    float l00 = dot(c00, lumW); float l01 = dot(c01, lumW); float l02 = dot(c02, lumW);
    float l10 = dot(c10, lumW); float l12 = dot(c12, lumW);
    float l20 = dot(c20, lumW); float l21 = dot(c21, lumW); float l22 = dot(c22, lumW);

    float gx = (l02 + 2.0f * l12 + l22) - (l00 + 2.0f * l10 + l20);
    float gy = (l20 + 2.0f * l21 + l22) - (l00 + 2.0f * l01 + l02);

    float edge = sqrt(gx * gx + gy * gy);
    return smoothstep(threshold, threshold * 2.0f + 0.05f, edge);
}

// --- ACES Filmic トーンマッピング関数 ---
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// --- トーンマッピング・露出適用 ---
float3 ApplyToneMapping(float3 col, float toneMapType, float exposure)
{
    col *= max(0.01f, exposure);

    int type = (int)(toneMapType + 0.5f);
    if (type == 1)
    {
        // Reinhard
        return col / (col + 1.0f);
    }
    else if (type == 2)
    {
        // ACES Filmic
        return ACESFilm(col);
    }
    return col;
}

// --- カラーフィルター適用関数 ---
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
    float3 col = SampleWithChromaticAberration(g_texture0, g_sampler, input.uv, g_chromaticAberration);
    col = ApplyToneMapping(col, g_toneMapType, g_exposure);
    col = ApplyEffect(col, input.uv, g_effectType);

    if (g_outlineIntensity > 0.001f)
    {
        float edge = CalculateEdge(g_texture0, g_sampler, input.uv, g_screenSize, g_outlineWidth, g_outlineThreshold);
        col = lerp(col, float3(0.0f, 0.0f, 0.0f), edge * g_outlineIntensity);
    }

    return float4(col, 1.0f);
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
    float3 finalColor = SampleWithChromaticAberration(g_texture0, g_sampler, input.uv, g_chromaticAberration);
    
    if (g_bloomEnabled > 0.5f)
    {
        float3 bloomColor = g_texture1.Sample(g_sampler, input.uv).rgb;
        finalColor += bloomColor * g_intensity;
    }
    
    finalColor = ApplyToneMapping(finalColor, g_toneMapType, g_exposure);
    finalColor = ApplyEffect(finalColor, input.uv, g_effectType);

    if (g_outlineIntensity > 0.001f)
    {
        float edge = CalculateEdge(g_texture0, g_sampler, input.uv, g_screenSize, g_outlineWidth, g_outlineThreshold);
        finalColor = lerp(finalColor, float3(0.0f, 0.0f, 0.0f), edge * g_outlineIntensity);
    }

    return float4(finalColor, 1.0f);
}
