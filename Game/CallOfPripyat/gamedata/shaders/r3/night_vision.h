#include "common.h"

//utility
float remap(float value, float inputMin, float inputMax, float outputMin, float outputMax)
{
    return (value - inputMin) * ((outputMax - outputMin) / (inputMax - inputMin)) + outputMin;
}
float rand(float2 n, float time)
{
  return 0.5 + 0.5 * 
     frac(sin(dot(n.xy, float2(12.9898, 78.233)))* 43758.5453 + time);
}

struct Circle
{
	float2 center;
	float radius;
};
	
/*float4 circle_mask_color(Circle circle, float2 position)
{
	float d = distance(circle.center, position);
	if(d > circle.radius)
	{
		return float4(0.0, 0.0, 0.0, 1.0);
	}
	
	float distanceFromCircle = circle.radius - d;
	float intencity = smoothstep(
								    0.0, 1.0, 
								    clamp(
									    remap(distanceFromCircle, 0.0, 0.1, 0.0, 1.0),
									    0.0,
									    1.0
								    )
								);
	return float4(intencity, intencity, intencity, 1.0);
}*/

/*float4 mask_blend(float4 a, float4 b)
{
	float4 one = float4(1.0, 1.0, 1.0, 1.0);
	return one - (one - a) * (one - b);
}*/

float f1(float x)
{
	return -4.0 * pow(x - 0.5, 2.0) + 1.0;
}