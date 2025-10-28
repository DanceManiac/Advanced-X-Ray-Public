/**
 * @ Version: Advanced X-Ray Update 5
 * @ Description: Advanced X-Ray Aurora Shader (based on goLLuMz shader)
 * @ Modified: 28.10.25
 * @ Authors: Dance Maniac, goLLuMz
 * @ Mod: https://ap-pro.ru/forums/topic/2813-advanced-x-ray/
 * @ Original shader: https://www.shadertoy.com/view/McSfWm
 */

#define MAX_DIST 100.0
#define PI 3.1415926535

uniform float4 aurora_params; // xyz - RGB цвет, w - интенсивность

float random(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.x + p3.y) * p3.z);
}

float2x2 mm2(float a)
{
    float c = cos(a), s = sin(a);
    return float2x2(c, s, -s, c);
}

float tri(float x)
{
    return abs(frac(x) - 0.5);
}

float2 tri2(float2 p)
{
    return float2(tri(p.x) + tri(p.y), tri(p.y + tri(p.x)));
}

float fbmAurora(float2 p, float spd)
{
    float z = 1.8;
    float z2 = 2.5;
    float rz = 0.0;
    p = mul(p, mm2(p.x * 0.06));
    float2 bp = p;
    
    for (float i = 0.0; i < 5.0; i++)
    {
        float2 dg = tri2(bp * 1.85) * 0.75;
        dg = mul(dg, mm2(timers.x * spd));
        p -= dg / z2;

        bp *= 1.3;
        z2 *= 0.45;
        z *= 0.42;
        p *= 1.21 + (rz - 1.0) * 0.02;

        rz += tri(p.x + tri(p.y)) * z;
        p = mul(p, mm2(timers.x * spd * 0.01));
    }
    return clamp(1.0 / pow(rz * 20.0, 1.3), 0.0, 1.0);
}

float4 aurora(float3 rd, float4 tc)
{
    float4 col = float4(0, 0, 0, 0);
    float4 avgCol = float4(0, 0, 0, 0);

    float auroraIntensity = aurora_params.w;
    
    if (auroraIntensity < 0.05)
        return col;

    int sampleCount = lerp(30, 55, auroraIntensity);

    float baseSpeed = 0.008;
    float maxBoost = 0.015;

    float speedMultiplier = lerp(0.3, 1.0, auroraIntensity);
    float animationSpeed = (baseSpeed + (maxBoost * auroraIntensity * auroraIntensity)) * speedMultiplier;
    
    float3 currentClassicGreen = float3(0.2, 0.7, 0.3);
    float3 currentTeal = float3(0.1, 0.5, 0.4);
    float3 currentSoftPurple = float3(0.3, 0.2, 0.5);
    
    for (int idx = 0; idx < sampleCount; idx++)
    {
        float i = float(idx);

        float timeOffset = timers.x * animationSpeed;
        float of = 0.006 * random(tc.xy + float2(timeOffset, i * 0.1)) * smoothstep(0.0, 15.0, i);
        
        float pt = ((0.8 + pow(i, 1.4) * 0.002)) / (rd.y * 2.0 + 0.4);
        pt -= of;
        float3 bpos = float3(0, 5.5, 0) + pt * rd;
        float2 p = bpos.zx;

        float rzt = fbmAurora(p, animationSpeed);
        
        float4 col2 = float4(0, 0, 0, rzt);

        float3 baseColor = aurora_params.xyz;

        float3 colorVariant1 = baseColor * float3(1.0, 1.2, 0.8);
        float3 colorVariant2 = baseColor * float3(0.8, 0.9, 1.2);
        float3 colorVariant3 = baseColor * float3(1.1, 0.7, 1.0);

        float3 dynamicColors = colorVariant1 * rzt + 
                              colorVariant2 * (rzt * 0.8) + 
                              colorVariant3 * ((1.0 - rzt) * 0.3);

        dynamicColors *= (0.9 + 0.1 * sin(timeOffset * 0.3 + i * 0.05));
        
        col2.rgb = dynamicColors * rzt;
        
        avgCol = lerp(avgCol, col2, 0.5);
        col += avgCol * exp2(-i * 0.065 - 2.5) * smoothstep(0.0, 5.0, i);
    }
    
    col *= clamp(rd.y * 15.0 + 0.4, 0.0, 1.0);
    col *= auroraIntensity;

    col.rgb = pow(col.rgb, 1.0/1.2);
    
    return smoothstep(0.0, 1.1, col * 1.5);
}