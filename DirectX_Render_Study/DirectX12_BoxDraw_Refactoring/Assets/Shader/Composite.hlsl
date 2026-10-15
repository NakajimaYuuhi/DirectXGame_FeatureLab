struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

// Fullscreen triangle VS
VSOutput VSMain(uint vertexID : SV_VertexID) {
    VSOutput output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

cbuffer CompositeCB : register(b0)
{
    float g_opacity;
    float3 g_padding;
};

Texture2D    g_layerTex : register(t0);
SamplerState g_sampler  : register(s0);

float4 PSMain(VSOutput input) : SV_TARGET {
    float4 col = g_layerTex.Sample(g_sampler, input.uv);
    col.rgb *= g_opacity;
    col.a *= g_opacity;
    return col;
}
