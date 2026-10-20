struct GPUParticle
{
    float3 position;
    float  life;
    float3 velocity;
    float  maxLife;
    float4 color;
    float  scale;
    uint   active;
    float2 pad;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 g_ViewProj;
    float3   g_CameraRight;
    float    g_Pad0;
    float3   g_CameraUp;
    float    g_Pad1;
};

StructuredBuffer<GPUParticle> g_Particles : register(t0);

struct VSOutput
{
    float4 pos   : SV_POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

static const float2 QuadOffsets[6] = {
    float2(-0.5f, -0.5f),
    float2(-0.5f,  0.5f),
    float2( 0.5f, -0.5f),
    float2( 0.5f, -0.5f),
    float2(-0.5f,  0.5f),
    float2( 0.5f,  0.5f)
};

static const float2 QuadUVs[6] = {
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f)
};

VSOutput VSMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VSOutput output;

    GPUParticle p = g_Particles[instanceID];

    // If inactive, place behind near plane to discard
    if (p.active == 0 || p.life <= 0.0f)
    {
        output.pos = float4(0, 0, 0, 0);
        output.uv = float2(0, 0);
        output.color = float4(0, 0, 0, 0);
        return output;
    }

    uint cornerIndex = vertexID % 6;
    float2 offset = QuadOffsets[cornerIndex] * p.scale;

    float3 worldPos = p.position + g_CameraRight * offset.x + g_CameraUp * offset.y;

    output.pos = mul(g_ViewProj, float4(worldPos, 1.0f));
    output.uv = QuadUVs[cornerIndex];
    output.color = p.color;

    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    if (input.color.a <= 0.0f)
        discard;

    // Circular soft falloff
    float2 uvDist = input.uv - float2(0.5f, 0.5f);
    float distSq = dot(uvDist, uvDist);
    if (distSq > 0.25f)
        discard;

    // Smooth glowing falloff
    float alpha = saturate((0.25f - distSq) * 4.0f);
    alpha = pow(alpha, 1.5f);

    float4 col = input.color;
    col.a *= alpha;
    col.rgb *= col.a;

    return col;
}
