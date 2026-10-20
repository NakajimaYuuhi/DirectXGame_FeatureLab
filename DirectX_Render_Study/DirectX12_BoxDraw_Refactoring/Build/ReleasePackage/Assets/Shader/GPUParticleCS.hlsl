struct GPUParticle
{
    float3 position;
    float  life;        // Remaining life in seconds (<= 0 means dead)
    float3 velocity;
    float  maxLife;     // Initial life
    float4 color;       // RGBA color
    float  scale;       // Particle size
    uint   active;      // 1: active, 0: dead
    float2 pad;         // Alignment
};

cbuffer SimulationConstants : register(b0)
{
    float  g_DeltaTime;
    float  g_TotalTime;
    uint   g_MaxParticles;
    uint   g_EmitCountThisFrame;

    float3 g_EmitterPosition;
    float  g_ParticleScale;

    float3 g_Gravity;
    float  g_InitialSpeed;

    float4 g_ParticleColor;
};

RWStructuredBuffer<GPUParticle> g_Particles : register(u0);

// Simple pseudo-random number generator
float Hash(uint n)
{
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 789221U) + 1376312589U;
    return float(n & 0x7fffffffU) / float(0x7fffffff);
}

float3 RandomDirection(uint seed)
{
    float u = Hash(seed) * 2.0f - 1.0f;
    float theta = Hash(seed + 1013904223U) * 6.2831853f;
    float f = sqrt(max(0.0f, 1.0f - u * u));
    return float3(f * cos(theta), u, f * sin(theta));
}

[numthreads(64, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    if (id >= g_MaxParticles)
        return;

    GPUParticle p = g_Particles[id];

    // If particle is active, update it
    if (p.active != 0)
    {
        p.life -= g_DeltaTime;
        if (p.life <= 0.0f)
        {
            p.active = 0;
            p.life = 0.0f;
        }
        else
        {
            // Apply gravity and movement
            p.velocity += g_Gravity * g_DeltaTime;
            p.position += p.velocity * g_DeltaTime;

            // Fade out alpha near end of life
            float lifeRatio = p.life / max(0.001f, p.maxLife);
            p.color.a = lifeRatio;
        }
    }

    // Try respawning if dead and within emit quota this frame
    if (p.active == 0 && id < g_EmitCountThisFrame)
    {
        uint seed = id + asuint(g_TotalTime * 1000.0f);
        float3 dir = RandomDirection(seed);
        // Biased upwards a bit
        dir.y = abs(dir.y) * 1.5f + 0.2f;
        dir = normalize(dir);

        float speed = g_InitialSpeed * (0.8f + 0.4f * Hash(seed + 41U));
        float maxLife = 1.5f + 1.0f * Hash(seed + 73U);

        p.position = g_EmitterPosition;
        p.velocity = dir * speed;
        p.life = maxLife;
        p.maxLife = maxLife;
        p.scale = g_ParticleScale * (0.7f + 0.6f * Hash(seed + 109U));
        p.color = g_ParticleColor;
        p.active = 1;
    }

    g_Particles[id] = p;
}
