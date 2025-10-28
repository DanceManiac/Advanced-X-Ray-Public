#define RB_INTENSITY float(0.2)           // Absolute maximal intensity of the rainbows
#define RB_SECONDARY float(0.1)           // Relative intensity of the secondary rainbow
#define RB_SATURATION float(0.75)         // Saturation of the rainbows
#define RB_DECAY float(5.0)               // Intensity decay / falloff
#define RB_COLRATIO float(0.60)           // Color ratio
#define RB_COLSHIFT float(0.0)            // Color shift
#define RB_COLPOWER float(0.52)           // Color power    
#define RB_ENDXMIN float(-6.0)            // Fade out towards ground (min)
#define RB_ENDXMAX float(5.0)             // Fade out towards ground (max)
#define RB_COFX float(1.5)                // Color amount towards ground
#define RB_ENDY float(2.0)                // Fade out towards sky
#define RB_COFY float(1.0)                // Color amount towards sky
#define RB_VIEWANGLE float4(0.5,1.0,-1.0,0.999) // View angle range for rainbows
#define RB_GREENREDDIFF float(-0.05)      // Minimum green-red color difference
#define RB_GREENBLUEDIFF float(0.05)      // Minimum green-blue color difference    
#define RB_POSITION float2(0.9,-0.6)      // Rainbow position
#define RB_ORIGIN float2(-2.0,-0.6)       // Coordinate system origin
#define RB_COF float(9.0)                 // Maximum intensity multiplier
#define RB_SUNFACTOR float(3.0)           // Sunlight intensity effect
#define RB_WIDESCREEN                     // Wide-screen adjustment

// Generate rainbow color from position
float4 generate_rainbow_color(float position, bool is_white)
{
    position += 0.333;
    position = -(position - 0.5) + 0.5;

    int color_index = (int)(position * 6.0 % 6.0) + 5 * (position < 0.0);
    float color_progress = position * 6.0 % 1.0 + (position < 0.0);
    float inverse_progress = 1.0 - color_progress;
    
    float red = 0.0, green = 0.0, blue = 0.0;
    
    if (color_index == 0)
	{
        red = 1.0, green = color_progress, blue = 0.0;
    }
	else if (color_index == 1)
	{
        red = inverse_progress, green = 1.0, blue = 0.0;
    }
	else if (color_index == 2)
	{
        red = 0.0, green = 1.0, blue = color_progress;
    }
	else if (color_index == 3)
	{
        red = 0.0, green = inverse_progress, blue = 1.0;
    }
	else if (color_index == 4)
	{
        red = color_progress, green = 0.0, blue = 1.0;
    }
	else if (color_index == 5)
	{
        red = 1.0, green = 0.0, blue = inverse_progress;
    }
    
    if (is_white)
	{
        return float4(1.0, 1.0, 1.0, 1.0);
    }
	else
	{
        return float4(red, green, blue, 1.0);
    }
}

// Main rainbow drawing function
float4 draw_rainbow(float2 screen_pos, float2 rainbow_pos, float intensity, bool enabled, bool is_white)
{
    float4 final_color = float4(0.0, 0.0, 0.0, 0.0);
    
    // Rainbow geometry parameters
    float primary_thickness = 0.16;
    float secondary_thickness = 0.1;
    float2 primary_radius = float2(1.3, 1.3);
    float2 secondary_radius = float2(1.6, 1.6);
    
    // Adjust for white rainbow mode
    if (is_white)
	{
        primary_thickness = 0.18;
        primary_radius = float2(0.9, 0.9);
        secondary_thickness = 0.75;
        secondary_radius = float2(1.2, 1.2);
        rainbow_pos.y += 0.2;
    }
    
    float2 primary_outer = primary_radius + primary_thickness;
    float2 secondary_outer = secondary_radius + secondary_thickness;
    
    float2 normalized_pos = screen_pos;
    
    // Wide-screen adjustment
#ifdef RB_WIDESCREEN
    normalized_pos.y *= 10.0 / 16.0;
#endif
    
    // Distance and angle calculations
    float distance_to_center = distance(rainbow_pos, normalized_pos);
    float primary_x_angle = (normalized_pos.x - rainbow_pos.x) / primary_radius.x;
    float primary_y_angle = (normalized_pos.y - rainbow_pos.y) / primary_radius.y;
    float secondary_x_angle = (normalized_pos.x - rainbow_pos.x) / secondary_radius.x;
    float secondary_y_angle = (normalized_pos.y - rainbow_pos.y) / secondary_radius.y;
    
    // Only draw if enabled and on the correct side
    if (enabled && normalized_pos.x < rainbow_pos.x)
	{
        float blend_factor = 0.0;
        float4 rainbow_color_val = float4(0.0, 0.0, 0.0, 0.0);
        
        // Calculate fade exponent based on intensity
        float fade_range = abs(RB_ENDXMAX - RB_ENDXMIN);
        float fade_exponent = RB_ENDXMIN + intensity * fade_range;
        
        // Primary rainbow
        if (distance_to_center >= primary_radius.x && distance_to_center <= primary_outer.x)
		{
            float position = (distance_to_center - primary_radius.x) / (primary_outer.x - primary_radius.x);
            
            // Generate color and calculate intensity
            float color_position = pow(position * RB_COLRATIO - RB_COLSHIFT, RB_COLPOWER);
            rainbow_color_val = generate_rainbow_color(color_position, is_white);
            
            blend_factor = 1.0 * pow(1.0 - abs(position - 0.5), RB_DECAY);
            
            // Apply fade effects
            float ground_fade = saturate(RB_COFX - pow(primary_x_angle, fade_exponent));
            float sky_fade = saturate(RB_COFY - pow(primary_y_angle, RB_ENDY));
            
            blend_factor *= ground_fade * sky_fade;
        }
        // Secondary rainbow
        else if (distance_to_center >= secondary_radius.x && distance_to_center <= secondary_outer.x)
		{
            float position = (distance_to_center - secondary_radius.x) / (secondary_outer.x - secondary_radius.x);
            
            // Generate inverted color for secondary rainbow
            float color_position = 1.0 - pow(position * RB_COLRATIO - RB_COLSHIFT, RB_COLPOWER);
            rainbow_color_val = generate_rainbow_color(color_position, is_white);
            
            blend_factor = RB_SECONDARY * pow(1.0 - abs(position - 0.5), RB_DECAY);
            
            // Apply fade effects
            float ground_fade = saturate(RB_COFX - pow(secondary_x_angle, fade_exponent));
            float sky_fade = saturate(RB_COFY - pow(secondary_y_angle, RB_ENDY));
            
            blend_factor *= ground_fade * sky_fade;
        }
        
        // Apply saturation and final intensity
        if (blend_factor > 0.0)
		{
            float luminance = dot(rainbow_color_val.xyz, float3(0.3, 0.59, 0.11));
            rainbow_color_val.xyz = lerp(float3(luminance, luminance, luminance), 
                                       rainbow_color_val.xyz, 
                                       RB_SATURATION);
            
            final_color = (1.0 - RB_INTENSITY * blend_factor) * final_color + 
                         RB_INTENSITY * blend_factor * rainbow_color_val;
        }
    }
    
    return final_color;
}