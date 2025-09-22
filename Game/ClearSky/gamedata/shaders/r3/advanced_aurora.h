/**
 * @ Version: Advanced X-Ray Update 5
 * @ Description: Advanced X-Ray Aurora Shader (based on goLLuMz shader)
 * @ Modified: 22.09.25
 * @ Authors: Dance Maniac, goLLuMz
 * @ Mod: https://ap-pro.ru/forums/topic/2813-advanced-x-ray/
 * @ Original shader: https://www.shadertoy.com/view/McSfWm
 */

#define MAX_DIST 100.0
#define PI 3.1415926535

float4 device_influence;

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

    float auroraIntensity = device_influence.x;
    
    if (auroraIntensity < 0.05)
        return col;

    int sampleCount = lerp(30, 55, auroraIntensity);

    float baseSpeed = 0.02;
    float maxBoost = 0.03;
    float animationSpeed = baseSpeed + (maxBoost * auroraIntensity * auroraIntensity);
    
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

        float3 classicGreen = float3(0.2, 0.7, 0.3) * rzt;
        float3 teal = float3(0.1, 0.5, 0.4) * (rzt * 0.8);
        float3 softPurple = float3(0.3, 0.2, 0.5) * ((1.0 - rzt) * 0.3);

        float3 dynamicColors = classicGreen + teal + softPurple;
        dynamicColors *= (0.9 + 0.1 * sin(timeOffset * 0.5 + i * 0.1));
        
        col2.rgb = dynamicColors * rzt;
        
        avgCol = lerp(avgCol, col2, 0.5);
        col += avgCol * exp2(-i * 0.065 - 2.5) * smoothstep(0.0, 5.0, i);
    }
    
    col *= clamp(rd.y * 15.0 + 0.4, 0.0, 1.0);
    col *= auroraIntensity;

    col.rgb = pow(col.rgb, 1.0/1.2);
    
    return smoothstep(0.0, 1.1, col * 1.5);
}