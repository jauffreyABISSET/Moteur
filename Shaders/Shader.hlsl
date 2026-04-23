#define MAX_LIGHTS 16

// Textures et sampler
Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer ObjectCB : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4 gOverrideColor; // w = facteur override
};

cbuffer MaterialCB : register(b1)
{
    float4 gColor;
    float roughness;
    float metallic;
    float isUI;
    float2 pad;
};

// --- Lumières ---
cbuffer LightCB : register(b2)
{
    float4 LightPosRange[MAX_LIGHTS];
    float4 LightColorIntensity[MAX_LIGHTS];
    float4 LightStrengthPad[MAX_LIGHTS];
    float4 LightDirType[MAX_LIGHTS];
    float4 LightSpotAngles[MAX_LIGHTS];

    int LightCount;
    float3 padding;
};

// --- Input vertex ---
struct VSInput
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

// --- Input pixel ---
struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    float4 Color : COLOR;
};

// --- Vertex Shader ---
PSInput VS(VSInput input)
{
    PSInput o;

    float4 worldPos = mul(float4(input.Pos, 1), gWorld);
    o.WorldPos = worldPos.xyz;
    o.Normal = normalize(mul((float3x3) gWorldInvTranspose, input.Normal));
    o.Pos = mul(float4(input.Pos, 1), gWorldViewProj);
    o.TexCoord = input.TexCoord;
    o.Color = input.Color;

    return o;
}

float4 PS(PSInput input) : SV_TARGET
{
    float3 normal = normalize(input.Normal);

    float4 texSample = gTexture.Sample(gSampler, input.TexCoord);
    float3 texColor = texSample.rgb;
    float texAlpha = texSample.a;

    // Couleur de base
    float3 baseColor = texColor * gColor.rgb * input.Color.rgb;

    // --- Override couleur par instance ---
    if (gOverrideColor.w > 0) 
        baseColor = lerp(baseColor, gOverrideColor.rgb, gOverrideColor.w);

    // Si c'est une UI, on ignore l'éclairage
    if (isUI > 0.1f)
        return float4(baseColor, texAlpha);

    // --- Éclairage ---
    float3 totalLight = 0;
    int count = clamp(LightCount, 0, MAX_LIGHTS);

    for (int i = 0; i < count; i++)
    {
        int lightType = (int) LightDirType[i].w;
        float3 lightColor = LightColorIntensity[i].rgb;
        float intensity = LightColorIntensity[i].w;
        float3 strength = LightStrengthPad[i].xyz;

        float3 L;
        float NdotL = 0;
        float attenuation = 1.0;
        float spotFactor = 1.0;

        if (lightType == 0) // Directionnelle
        {
            float3 lightDir = normalize(LightDirType[i].xyz);
            L = -lightDir;
            NdotL = dot(normal, L);
            if (NdotL <= 0)
                continue;
        }
        else if (lightType == 1) // Point
        {
            float3 lightPos = LightPosRange[i].xyz;
            float range = max(LightPosRange[i].w, 0.001);
            float3 toLight = lightPos - input.WorldPos;
            float dist = length(toLight);
            L = normalize(toLight);
            NdotL = dot(normal, L);
            if (NdotL <= 0)
                continue;
            attenuation = saturate(1.0 - dist / range);
        }
        else if (lightType == 2) // Spot
        {
            float3 lightPos = LightPosRange[i].xyz;
            float range = max(LightPosRange[i].w, 0.001);
            float3 lightDir = normalize(LightDirType[i].xyz);
            float3 toLight = lightPos - input.WorldPos;
            float dist = length(toLight);
            L = normalize(toLight);
            NdotL = dot(normal, L);
            if (NdotL <= 0)
                continue;
            attenuation = saturate(1.0 - dist / range);

            float cosTheta = dot(-L, lightDir);
            float cosInner = LightSpotAngles[i].x;
            float cosOuter = LightSpotAngles[i].y;
            float epsilon = max(cosInner - cosOuter, 0.001);
            spotFactor = saturate((cosTheta - cosOuter) / epsilon);
        }

        float3 diffuse = lightColor * strength * intensity * attenuation * NdotL * spotFactor;
        totalLight += diffuse;
    }

    float3 ambient = 0.15 * baseColor;

    return float4(baseColor * totalLight + ambient, 1);
}