//=================================================================================================
//Look Modification Transforms
//Stackable preset color modifications (linear ACEScg in/out)
//=================================================================================================
#include "ACES_Color_Grading.h"
#include "ACES_LMTs\LMT_Contrast_Reduction.h"
#include "ACES_LMTs\LMT_Technicolor.h"
#include "ACES_LMTs\LMT_Bleach_Bypass.h"
#include "ACES_LMTs\LMT_Blue_Fix.h"

void ACES_LMT(inout float3 aces)
{
#ifdef USE_ACES
	//do color grading in ACEScg primaries for more predictable results
	aces = ACES_to_ACEScg(aces);
#endif

	Color_Grading(aces);
	Contrast_Reduction(aces);
	
	//Technicolor(aces);
	//Bleach_Bypass(aces);


#ifdef USE_ACES
	//Return to ACES AP0
	aces = ACEScg_to_ACES(aces);

	Blue_Fix(aces); //ACES AP0 blue color shift fix
#endif

}