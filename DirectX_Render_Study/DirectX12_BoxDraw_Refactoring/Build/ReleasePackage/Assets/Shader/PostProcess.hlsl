// ==========================================
// PostProcess.hlsl
// Post-Process (Bloom, Tonemapping, Exposure, Color Filter, Chromatic Aberration, Outline, Fade)
// ==========================================

struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

// --- Fullscreen Triangle Vertex Shader ---
VSOutput VSMain(uint vertexID : SV_VertexID) {
    VSOutput output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

// --- Root Constants (24 DWORD / 96 bytes) ---
cbuffer PostProcessCB : register(b0)
{
    float g_threshold;           // [0] Bloom threshold
    float g_knee;                // [1] Soft knee
    float g_intensity;           // [2] Bloom intensity
    float g_spread;              // [3] Blur spread
    float2 g_direction;          // [4,5] Blur direction
    float g_bloomEnabled;        // [6] 1.0: Enabled, 0.0: Disabled
    float g_effectType;          // [7] 0: None, 1: Grayscale, 2: Sepia, 3: Invert, 4: Vignette
    float g_chromaticAberration; // [8] Chromatic aberration intensity
    float g_outlineIntensity;    // [9] Outline intensity
    float g_outlineThreshold;    // [10] Outline threshold
    float g_outlineWidth;        // [11] Outline width
    float2 g_screenSize;         // [12,13] Screen size (width, height)
    float g_toneMapType;         // [14] 0: None, 1: Reinhard, 2: ACES Filmic
    float g_exposure;            // [15] Exposure multiplier
    float3 g_outlineColor;       // [16,17,18] Outline RGB color
    float g_preserveAlpha;       // [19] 1.0: Preserve source & outline alpha for layer compositing
    float3 g_fadeColor;          // [20,21,22] Screen fade color (default 0,0,0)
    float g_fade;                // [23] Screen fade amount (0.0: normal, 1.0: full fade)
};

Texture2D    g_texture0 : register(t0); // Main input
Texture2D    g_texture1 : register(t1); // Bloom blur texture
SamplerState g_sampler  : register(s0); // Linear clamp

// --- Chromatic Aberration Sampling ---
float4 SampleWithChromaticAberration(Texture2D tex, SamplerState smp, float2 uv, float intensity)
{
    float4 centerSample = tex.Sample(smp, uv);
    if (intensity <= 0.0001f)
    {
        return centerSample;
    }
    float2 toCenter = uv - float2(0.5f, 0.5f);
    float distSq = dot(toCenter, toCenter);
    float2 offset = toCenter * (distSq * intensity * 2.0f);

    float r = tex.Sample(smp, uv + offset).r;
    float g = centerSample.g;
    float b = tex.Sample(smp, uv - offset).b;
    return float4(r, g, b, centerSample.a);
}

// --- Edge Detection (Sobel Filter on Luminance & Alpha Silhouette) ---
float CalculateEdge(Texture2D tex, SamplerState smp, float2 uv, float2 screenSize, float width, float threshold)
{
    float2 texelSize = (width / max(screenSize, float2(1.0f, 1.0f)));

    float4 c00 = tex.Sample(smp, uv + float2(-texelSize.x, -texelSize.y));
    float4 c01 = tex.Sample(smp, uv + float2( 0.0f,        -texelSize.y));
    float4 c02 = tex.Sample(smp, uv + float2( texelSize.x, -texelSize.y));
    float4 c10 = tex.Sample(smp, uv + float2(-texelSize.x,  0.0f));
    float4 c12 = tex.Sample(smp, uv + float2( texelSize.x,  0.0f));
    float4 c20 = tex.Sample(smp, uv + float2(-texelSize.x,  texelSize.y));
    float4 c21 = tex.Sample(smp, uv + float2( 0.0f,         texelSize.y));
    float4 c22 = tex.Sample(smp, uv + float2( texelSize.x,  texelSize.y));

    float3 lumW = float3(0.299f, 0.587f, 0.114f);
    float l00 = dot(c00.rgb, lumW); float l01 = dot(c01.rgb, lumW); float l02 = dot(c02.rgb, lumW);
    float l10 = dot(c10.rgb, lumW);                                 float l12 = dot(c12.rgb, lumW);
    float l20 = dot(c20.rgb, lumW); float l21 = dot(c21.rgb, lumW); float l22 = dot(c22.rgb, lumW);

    float gxLum = (l02 + 2.0f * l12 + l22) - (l00 + 2.0f * l10 + l20);
    float gyLum = (l20 + 2.0f * l21 + l22) - (l00 + 2.0f * l01 + l02);
    float edgeLum = sqrt(gxLum * gxLum + gyLum * gyLum);

    float gxAlpha = (c02.a + 2.0f * c12.a + c22.a) - (c00.a + 2.0f * c10.a + c20.a);
    float gyAlpha = (c20.a + 2.0f * c21.a + c22.a) - (c00.a + 2.0f * c01.a + c02.a);
    float edgeAlpha = sqrt(gxAlpha * gxAlpha + gyAlpha * gyAlpha);

    float edge = max(edgeLum, edgeAlpha);
    return smoothstep(threshold, threshold * 2.0f + 0.05f, edge);
}

// --- ACES Filmic Tonemapping ---
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// --- Tonemapping & Exposure ---
float3 ApplyToneMapping(float3 col, float toneMapType, float exposure)
{
    col *= max(0.01f, exposure);

    int type = (int)(toneMapType + 0.5f);
    if (type == 1)
    {
        return col / (col + 1.0f);
    }
    else if (type == 2)
    {
        return ACESFilm(col);
    }
    return col;
}

// --- Color Filter Effects ---
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

// --- PassThrough (when Bloom is disabled) ---
float4 PSPassThrough(VSOutput input) : SV_TARGET {
    float4 sampled = SampleWithChromaticAberration(g_texture0, g_sampler, input.uv, g_chromaticAberration);
    float3 col = sampled.rgb;
    float srcAlpha = sampled.a;

    col = ApplyToneMapping(col, g_toneMapType, g_exposure);
    col = ApplyEffect(col, input.uv, g_effectType);

    float outAlpha = (g_preserveAlpha > 0.5f) ? srcAlpha : 1.0f;

    if (g_outlineIntensity > 0.001f)
    {
        float edge = CalculateEdge(g_texture0, g_sampler, input.uv, g_screenSize, g_outlineWidth, g_outlineThreshold);
        float edgeFactor = saturate(edge * g_outlineIntensity);
        if (g_preserveAlpha > 0.5f && srcAlpha < 0.05f)
        {
            col = g_outlineColor;
        }
        else
        {
            col = lerp(col, g_outlineColor, edgeFactor);
        }
        if (g_preserveAlpha > 0.5f)
        {
            outAlpha = saturate(max(srcAlpha, edgeFactor));
        }
    }

    if (g_fade > 0.001f)
    {
        col = lerp(col, g_fadeColor, saturate(g_fade));
    }

    return float4(col, outAlpha);
}

// Default entry point
float4 PSMain(VSOutput input) : SV_TARGET {
    return PSPassThrough(input);
}

// --- Bright Pass ---
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

// --- Gaussian Blur ---
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

// --- Final Composite (with Bloom) ---
float4 PSComposite(VSOutput input) : SV_TARGET {
    float4 sampled = SampleWithChromaticAberration(g_texture0, g_sampler, input.uv, g_chromaticAberration);
    float3 finalColor = sampled.rgb;
    float srcAlpha = sampled.a;
    
    if (g_bloomEnabled > 0.5f)
    {
        float3 bloomColor = g_texture1.Sample(g_sampler, input.uv).rgb;
        finalColor += bloomColor * g_intensity;
    }
    
    finalColor = ApplyToneMapping(finalColor, g_toneMapType, g_exposure);
    finalColor = ApplyEffect(finalColor, input.uv, g_effectType);

    float outAlpha = (g_preserveAlpha > 0.5f) ? srcAlpha : 1.0f;

    if (g_outlineIntensity > 0.001f)
    {
        float edge = CalculateEdge(g_texture0, g_sampler, input.uv, g_screenSize, g_outlineWidth, g_outlineThreshold);
        float edgeFactor = saturate(edge * g_outlineIntensity);
        if (g_preserveAlpha > 0.5f && srcAlpha < 0.05f)
        {
            finalColor = g_outlineColor;
        }
        else
        {
            finalColor = lerp(finalColor, g_outlineColor, edgeFactor);
        }
        if (g_preserveAlpha > 0.5f)
        {
            outAlpha = saturate(max(srcAlpha, edgeFactor));
        }
    }

    if (g_fade > 0.001f)
    {
        finalColor = lerp(finalColor, g_fadeColor, saturate(g_fade));
    }

    return float4(finalColor, outAlpha);
}
