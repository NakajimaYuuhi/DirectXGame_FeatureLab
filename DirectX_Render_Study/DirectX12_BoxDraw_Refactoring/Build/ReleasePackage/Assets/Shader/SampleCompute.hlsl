cbuffer ComputeParams : register(b0)
{
    float g_Time;
    float3 g_Padding;
};

RWTexture2D<float4> g_Output : register(u0);

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    g_Output.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height)
        return;

    float2 uv = float2(DTid.xy) / float2(width, height);

    // Simple test pattern
    float r = sin(uv.x * 6.28318f + g_Time) * 0.5f + 0.5f;
    float g = cos(uv.y * 6.28318f + g_Time * 0.5f) * 0.5f + 0.5f;
    float b = sin((uv.x + uv.y) * 6.28318f + g_Time * 2.0f) * 0.5f + 0.5f;

    g_Output[DTid.xy] = float4(r, g, b, 1.0f);
}
