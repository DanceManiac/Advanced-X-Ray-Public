/**
 * @ Version: Advanced X-Ray Update 5
 * @ Description: Advanced X-Ray Rainbow Shader (based on Meltac shader)
 * @ Modified: 28.10.25
 * @ Authors: Dance Maniac, Meltac
 * @ Mod: https://ap-pro.ru/forums/topic/2813-advanced-x-ray/
 * @ Original shader: https://ap-pro.ru/forums/topic/239-dynamic-shaders-2-30-wip/
 */
 
#define RB_INTENSITY float(0.375)         // Интенсивность
#define RB_SECONDARY float(0.13)          // Вторичная радуга
#define RB_SATURATION float(0.85)         // Насыщенность
#define RB_DECAY float(4.0)               // Плавность затухания краёв
#define RB_COLRATIO float(0.65)           // Распределение цветов
#define RB_COLSHIFT float(0.05)           // Сдвиг цветов
#define RB_COLPOWER float(0.6)            // Мягкость переходов
#define RB_ENDXMIN float(-4.0)            // Затухание к земле (мин)
#define RB_ENDXMAX float(3.0)             // Затухание к земле (макс)
#define RB_COFX float(1.8)                // Количество цвета к земле
#define RB_ENDY float(1.8)                // Затухание к небу
#define RB_COFY float(1.2)                // Количество цвета к небу
#define RB_VIEWANGLE float4(0.5,1.0,-1.0,0.999) // Диапазон углов обзора для появления радуги
#define RB_GREENREDDIFF float(-0.05)      // Минимальная разница зеленый-красный
#define RB_GREENBLUEDIFF float(0.05)      // Минимальная разница зеленый-синий
#define RB_POSITION float2(0.9,-0.6)      // Позиция радуги
#define RB_ORIGIN float2(-2.0,-0.6)       // Начало системы координат
#define RB_COF float(8.0)                 // Множитель максимальной интенсивности
#define RB_SUNFACTOR float(2.5)           // Влияние интенсивности солнца
#define RB_ARCSIZE float(1.0)             // Размер дуги - 0.5 = полдуги, 1.0 = полная дуга
#define RB_WIDESCREEN                     // Коррекция для широкоэкранных мониторов

// Generate rainbow color from position
float4 generate_rainbow_color(float position, bool is_white)
{
    if (is_white)
        return float4(1.0, 1.0, 1.0, 1.0);

    float3 color;
    
    if (position < 0.1667)
	{ // Красный-оранжевый
        float t = position / 0.1667;
        color = lerp(float3(1.0, 0.1, 0.1), float3(1.0, 0.5, 0.0), smoothstep(0.0, 1.0, t));
    }
    else if (position < 0.3333)
	{ // Оранжевый-желтый
        float t = (position - 0.1667) / 0.1667;
        color = lerp(float3(1.0, 0.5, 0.0), float3(1.0, 0.9, 0.1), smoothstep(0.0, 1.0, t));
    }
    else if (position < 0.5)
	{ // Желтый-зеленый
        float t = (position - 0.3333) / 0.1667;
        color = lerp(float3(1.0, 0.9, 0.1), float3(0.3, 0.8, 0.1), smoothstep(0.0, 1.0, t));
    }
    else if (position < 0.6667)
	{ // Зеленый-голубой
        float t = (position - 0.5) / 0.1667;
        color = lerp(float3(0.3, 0.8, 0.1), float3(0.1, 0.8, 0.9), smoothstep(0.0, 1.0, t));
    }
    else if (position < 0.8333)
	{ // Голубой-синий
        float t = (position - 0.6667) / 0.1667;
        color = lerp(float3(0.1, 0.8, 0.9), float3(0.1, 0.3, 1.0), smoothstep(0.0, 1.0, t));
    }
    else
	{ // Синий-фиолетовый
        float t = (position - 0.8333) / 0.1667;
        color = lerp(float3(0.1, 0.3, 1.0), float3(0.6, 0.2, 0.8), smoothstep(0.0, 1.0, t));
    }
    
    return float4(color, 1.0);
}

// Main rainbow drawing function
float4 draw_rainbow(float2 screen_pos, float2 rainbow_pos, float intensity, bool enabled, bool is_white)
{
    float4 final_color = float4(0.0, 0.0, 0.0, 0.0);
    
    if (!enabled)
        return final_color;

    float primary_thickness = 0.14;
    float secondary_thickness = 0.08;
    float primary_radius = 1.35;
    float secondary_radius = 1.7;
    
    // Adjust for white rainbow mode
    if (is_white)
    {
        primary_thickness = 0.16;
        primary_radius = 1.0;
        secondary_thickness = 0.7;
        secondary_radius = 1.4;
        rainbow_pos.y += 0.15;
    }
    
    float primary_outer = primary_radius + primary_thickness;
    float secondary_outer = secondary_radius + secondary_thickness;
    
    float2 normalized_pos = screen_pos;
    
    // Wide-screen adjustment
#ifdef RB_WIDESCREEN
    normalized_pos.y *= 10.0 / 16.0;
#endif

    float2 to_fragment = normalized_pos - rainbow_pos;
    float distance_to_center = length(to_fragment);

    float angle = atan2(to_fragment.y, to_fragment.x);
    
    // Ограничение радуги только верхней полусферой (углы от 0 до PI)
    float min_angle = (1.0 - RB_ARCSIZE) * 1.57;  // от 0 до 90 градусов
	float max_angle = 3.14159 - min_angle;        // от 180 до 90 градусов  
	bool is_within_arc = angle > min_angle && angle < max_angle;

    bool is_on_correct_side = normalized_pos.x < rainbow_pos.x;
    
    // Only draw if enabled, on correct side and in upper half
    if (is_on_correct_side && is_within_arc)
    {
        float blend_factor = 0.0;
        float4 rainbow_color_val = float4(0.0, 0.0, 0.0, 0.0);
        
        // Calculate fade exponent based on intensity
        float fade_range = abs(RB_ENDXMAX - RB_ENDXMIN);
        float fade_exponent = RB_ENDXMIN + intensity * fade_range;
        
        // Primary rainbow
        if (distance_to_center >= primary_radius && distance_to_center <= primary_outer)
        {
            float position = (distance_to_center - primary_radius) / primary_thickness;
            
            // Generate color and calculate intensity
            float color_position = pow(position * RB_COLRATIO - RB_COLSHIFT, RB_COLPOWER);
            rainbow_color_val = generate_rainbow_color(color_position, is_white);

            blend_factor = 1.0 * pow(1.0 - abs(position - 0.5), RB_DECAY);
            
            // Apply fade effects
            float primary_x_angle = (normalized_pos.x - rainbow_pos.x) / primary_radius;
            float primary_y_angle = (normalized_pos.y - rainbow_pos.y) / primary_radius;
            
            float ground_fade = saturate(RB_COFX - pow(abs(primary_x_angle), fade_exponent));
            float sky_fade = saturate(RB_COFY - pow(primary_y_angle, RB_ENDY));

            float vertical_pos = normalized_pos.y - rainbow_pos.y;
            float arc_fade = smoothstep(0.0, 0.4, vertical_pos) * (1.0 - smoothstep(0.6, 1.0, vertical_pos));
            blend_factor *= ground_fade * sky_fade * arc_fade;

            float shimmer = 0.9 + 0.1 * sin(position * 20.0 + timers.x * 2.0);
            blend_factor *= shimmer;
        }
        // Secondary rainbow
        else if (distance_to_center >= secondary_radius && distance_to_center <= secondary_outer)
        {
            float position = (distance_to_center - secondary_radius) / secondary_thickness;
            
            // Generate inverted color for secondary rainbow
            float color_position = 1.0 - pow(position * RB_COLRATIO - RB_COLSHIFT, RB_COLPOWER);
            rainbow_color_val = generate_rainbow_color(color_position, is_white);

            blend_factor = RB_SECONDARY * pow(1.0 - abs(position - 0.3), RB_DECAY + 1.0);
            
            // Apply fade effects
            float secondary_x_angle = (normalized_pos.x - rainbow_pos.x) / secondary_radius;
            float secondary_y_angle = (normalized_pos.y - rainbow_pos.y) / secondary_radius;
            
            float ground_fade = saturate(RB_COFX - pow(abs(secondary_x_angle), fade_exponent));
            float sky_fade = saturate(RB_COFY - pow(secondary_y_angle, RB_ENDY));

            float vertical_pos = normalized_pos.y - rainbow_pos.y;
            float arc_fade = smoothstep(0.0, 0.4, vertical_pos) * (1.0 - smoothstep(0.6, 1.0, vertical_pos));
            blend_factor *= ground_fade * sky_fade * arc_fade;
        }
        
        // Apply saturation and final intensity
        if (blend_factor > 0.0)
        {
            float luminance = dot(rainbow_color_val.xyz, float3(0.299, 0.587, 0.114));
            rainbow_color_val.xyz = lerp(float3(luminance, luminance, luminance), 
                                       rainbow_color_val.xyz, 
                                       RB_SATURATION);

            final_color = RB_INTENSITY * blend_factor * rainbow_color_val;
        }
    }
    
    return final_color;
}