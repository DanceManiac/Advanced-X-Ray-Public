#ifndef OGSE_CONFIG_H
#define OGSE_CONFIG_H

#include "configurator_defines.h"
// ������� �� ���������� ����� �������
// ��������								r2_sunshafts [qt_off/qt_low/qt_medium/qt_high/qt_extreme]
// �������� ��������					r2_sunshafts_mode volumetric
// ������� �������� ��� Crysis			r2_sunshafts_mode screen_space
// ������ ����							r2_soft_water [on/off]
// ������ ��������						r2_soft_particles [on/off]
// ������ ����							r2_soft_shadows [qt_off/qt_low/qt_medium/qt_high/qt_extreme]
// SSDO									r2_ssao_mode ssdo
// HBAO									r2_ssao_mode hbao
// Steep Parallax						r2_steep_parallax [qt_off/qt_low/qt_medium/qt_high/qt_extreme]
// Dynamic Depth Of Field				r2_dof [qt_off/qt_low/qt_medium/qt_high/qt_extreme]
// Motion Blur							r2_mblur [on/off]
// Antialiasing							r2_aa [qt_off/qt_low/qt_medium/qt_high/qt_extreme]
// FXAA Antialiasing					r2_aa_mode fxaa
// SMAA Antialiasing					r2_aa_mode smaa
// ��������� ����						r2_detail_bump [on/off]

// PARAMETERS 
// ����������� ��������� ��� ��������� ���������������� �������
/////////////////////////////////////////////////////////////
// ������� ��������
	#define SUNSHAFTS_QUALITY 4
	#define SS_DUST
	#define SS_INTENSITY float(1.0)			// ������� �����, �������� ��������� � ����������� �� �������
	#define SS_BLEND_FACTOR float(0.8)		// ������ ���������� � ���������� ������. ��� ������ ��������, ��� ������ "��������" �� �������, �� �������� "����������"
	#define SS_LENGTH float(1.0)			// ����� �����. ��� ������ �����, ��� ������ ������������������
	#define SS_DUST_SPEED float(0.4)		// �������� ������ ���� 
	#define SS_DUST_INTENSITY float(2.0)	// ������� �������
	#define SS_DUST_DENSITY float(1.0)		// ��������� ������ ���� 
	#define SS_DUST_SIZE float(0.7)			// ������ �������

/////////////////////////////////////////////////////////////
// Screen-Space Directional Occlusion
	#define SSDO_RADIUS float(0.04)				// ������ ������������� � ������� ������. ��� ����, ��� ����� ������� ������, �� ��� ������ ������������� ��������� �����
	#define SSDO_GRASS_TUNING float(1.0)		// ��������� ��������� �����. ��� ������, ��� ������ ����������.
	#define SSDO_DISCARD_THRESHOLD float(1.0)	// ������������ ������� � ������� ��������, ��� ������� ������� ��� ����������� � ������� ���������. ���������� ������� "�����" � ��������� �������.
	#define SSDO_COLOR_BLEEDING float(15.0)		// ���� ����� �������. ���� ����� ������� ����, �� ��������� ������������� ������� � �����. ��� ����������� ������������ SSDO_BLEND_FACTOR.

/////////////////////////////////////////////////////////////
// Horizon-Based Ambient Occlusion	
	#define HBAO_NUM_STEPS int(3)			// ���������� ���������� ����� ��� ������ ���������. �������� ��������, �������� ������������������
	#define HBAO_RADIUS float(2.0)			// ������ ������ ���������.
	#define HBAO_STEP_SIZE float(4)			// ��� �������������. ������� �������� �������� � ����� ������ �����
	#define HBAO_ANGLE_BIAS float(0.5)		// ���� � �������� ��� ����������� ������������� ����� ��������������� ���������. �����������, ���� ��  ������ ��������� ������� ����.
	#define HBAO_THRESHOLD float(0.3)		// ����� ������������ �������. ��� ������, ��� ����� ������ ������ ����������.
	#define HBAO_GRASS_TUNING float(2.0)	// ��������� ��������� �����. ��� ������, ��� ������ ����������.

/////////////////////////////////////////////////////////////
// improved parallax occlusion mapping
	#define POM_PARALLAX_OFFSET float(0.02)			// ��������. ��� ������, ��� ������ ����� ��������� �������.

/////////////////////////////////////////////////////////////
// depth of field
	//common
	#define DDOF_SKY_DIST float(10000)		// ���������� �� ����. ������, ������ ���� ���� ��������� ��� ����������� ������ �� ����.
	#define DDOF_FOCUS_CIRCLE float(10)		// ������� ������� ������� ������������ ������, � ��������. ���������� �� ���������� ������� ���� ����� ������� ������� ��� ����� ������.
	// blur
//	#define DDOF_DEPTHBLUR 					// ���������� ������� ��������. ���� ����� ������ ������� ����������� �������� � �������������. ������ �� ������������������.
	#define DDOF_PENTAGON					// ��������� �������������� ����� ����. ��� ��������� ���� ����� ���������� ������� DDOF_SAMPLES = 5, DDOF_RINGS - 15-20 (ogse_dof.h)
		#define DDOF_FEATHER float(0.4) 	// ������ ����
		#define DDOF_FSTOP float(-0.2)		// ������ ������������ ������� �� ������. ����� ������� ������������� ��������. ��� ��������� ����� 0,7-DDOF_VIGNOUT ������ ���� ������ � ����� ������. ��� ��������� ����� 0,9-DDOF_VIGNOUT ������ ��������� �����.
		#define DDOF_FRINGE float(0.7) 		// ����������� �������� �������� ��� ������� ������������� ���������. ��� ������ ���������, ��� ������ ������ ���������.
	#define DDOF_KERNEL float(1)			// ����������� ��������. ������ �� ������������� �����	
	#define DDOF_THRESHOLD float(0.5) 		// ����� �������, ���� �������� ������� ������� ����� �������������. ����� 1,1 �� �������. ��� ����������, ��� �������, ���� ����������� DDOF_GAIN.
	#define DDOF_BIAS float(0.5) 			// ����������� ���������� ������ �� ������� �������. ��� ���������� ���������� ����� ������ ����.
	//far
	#define DDOF_FAR_PLANE float(100)		// ��������� �� ������ �� ������� ��������� �����.
	#define DDOF_FAR_INTENSITY float(0.6)	// ������������� �������. [0..1]
	//near
	#define DDOF_NEAR_PLANE float(20)		// ���������, � ������� ���� ����� �������� �����������. ������ ��������� - ������ �� ������ ����� ���������������� ������� ����
	#define DDOF_NEAR_MINDIST float (1.5)	// ���������� �� ������ �� ��������� ������ ������������. ����������, ����� �� ��������� ������
	#define DDOF_NEAR_INTENSITY float(0.3)	// ������������� �������. [0..1]
// zoom depth of field
	#define ZDOF_MINDIST float(250)	        // ����������� ��������� �� ������ ������, � ��������. ����� � ������ ����� ���
	#define ZDOF_MAXDIST float(500)			// ������������ ��������� �� ������ ������, � ��������. ������ - ����������� ����.
	#define ZDOF_OFFSET float(2)			// ������ ������� �������, ������� �������� �� ���������� ��� ������� �������� ������.
// reload depth of field
	#define RDOF_DIST float (1)				// ����������, �� ������� ���������� ����. ���������� ���, ����� �� ��������� ������ (0,7-1,2)
	#define RDOF_SPEED float (5)			// �������� ���������� �������. ������� �������� - 5-1

/////////////////////////////////////////////////////////////
// improved blur
	#define IMBLUR_START_DIST float(1.0)		// ��������� ��������� �����
	#define IMBLUR_FINAL_DIST float(300)		// �������� ��������� �����
	#define IMBLUR_SAMPLES int(20)				// ���������� �������. �������� ��������, �������� ������������������
	#define IMBLUR_CLAMP float(0.01)			// ������� �������� ��������
	#define IMBLUR_SCALE_X float(-0.03)			// ������� �������� �������� �� X
	#define IMBLUR_SCALE_Y float(0.03)			// ������� �������� �������� �� Y
	#define IMBLUR_VEL_START float(0.001)		// ��������� �������� - ��������� �������
	#define IMBLUR_VEL_FIN float(0.02)			// �������� �������� - ���������� �������	

/////////////////////////////////////////////////////////////
// ����������
	#define IKV_DIST float(0.1)					// ���������� ���������� ���������
	#define IKV_HEATING_SPEED float(0.1)		// �������� �������/���������� ������
	// �������
	#define IKV_PRESET_1_MIN float(0.274)		// ������� �������������� ����������. ������ ������ � ��� ������, ���� ������, ��� �������.
	#define IKV_PRESET_1_MAX float(0.300)		// PRESET_1 - ������ ��� �����, ��������, ����
	#define IKV_PRESET_2_MIN float(0.837)		// PRESET_2 - ������ ��� ������
	#define IKV_PRESET_2_MAX float(0.862)		
	// �����
	#define IKV_DEAD_COLOR float4(0.0,0.07,0.49,1.0)			// �������� ��������� �����������:
	#define IKV_MID3_COLOR float4(0.203,0.567,0.266,1.0)		// DEAD - ���� �������� ���������
	#define IKV_MID2_COLOR float4(0.644,0.672,0.098,1.0)		// LIVE - ���� ������� ���������
	#define IKV_MID1_COLOR float4(0.7,0.35,0.07,1.0)			// MID - ������������� �����
	#define IKV_LIVE_COLOR float4(0.7,0.16,0.08,1.0)	
	// ������
	#define IKV_NOISE_INTENSITY float(1.0)		// ������������� ����

/////////////////////////////////////////////////////////////
// ����
	#define WATER_ENV_POWER float (0.7)				// ������������� ��������� �������� �� ����
	#define PUDDLES_ENV_POWER float (0.3)			// ������������� ��������� �������� �� �����
	#define SW_USE_FOAM								// �������� "����" ������
	#define SW_FOAM_THICKNESS float (0.035)			// ������� "����"
	#define SW_FOAM_INTENSITY float (3)				// ������������� ����� "����"
	#define SW_WATER_INTENSITY float (1.0)			// ������� ����� ����
	#define SW_REFL_INTENSITY float(2.0)			// ������������� "�������" ���������
	#define SW_PUDDLES_REFL_INTENSITY float(3.0)	// ������������� "�������" ��������� � �����
	#define MOON_ROAD_INTENSITY float(1.5)			// ������������� "������ �������"
	#define WATER_GLOSS float(0.5)					// ������������� ��������� �� ����

/////////////////////////////////////////////////////////////
// ����� �� ����
	#define FL_POWER float(1.0)					// ����� ������������� ������
	#define FL_GLOW_RADIUS float(0.2)			// ������ ������ ��������� �����
	#define FL_DIRT_INTENSITY float(1.0)		// ������������� ������� Lens Dirt

	#define MODEL_SELFLIGHT_POWER float(6.0)	// ������� �������� �������

/////////////////////////////////////////////////////////////
// �������, ��������� � ������

	#define RAIN_MAX_DIFFUSE_SCALING float(0.7)				// ������������ ����������� ���������� ���������
	#define RAIN_MAX_DIFFUSE_DETAILS_MULT float(0.9)		// ����������� ���������� ����� ������������ ��������
	#define RAIN_MAX_SPECULAR_SCALING float (3.0)			// ������������ ������� ��������� �����
	#define RAIN_MAX_REFLECTION_POWER float(0.1)			// ������������ ���� ���������
	#define RAIN_WETTING_SPEED float(0.5)					// �������� ��������� ������������
	#define RAIN_GAIN_REFLECTIONS float(3.0)				// �������� ������� ��������� �� �������� ������������

	// test params, not used now
	#define RMAP_KERNEL float(1.0)
	#define RMAP_size float(2048)
	#define RMAP_MAX_ENV_MAP_COEFF float(0.1)

/////////////////////////////////////////////////////////////
// ����� �� �����
//	#define USE_GRASS_WAVE							// �������� "�����" �� ����� �� �����
	#define GRASS_WAVE_POWER float(2.0)				// "�������", ���������� ����
	#define GRASS_WAVE_FREQ float(0.7)				// ������� ��������� ����

/////////////////////////////////////////////////////////////
// ��������� �������������� ���������
	#define TRANSLUCENT_GLOSS float(1.0)						// ��������
	#define TRANSLUCENT_TORCH_ATT_FACTOR float(0.001231148)		// calculated as 1/(torch_range*torch_range*0.95*0.95)
	#define TRANSLUCENT_TORCH_COLOR float4(0.6,0.64,0.60,0.8)	// color of torch light
	#define TRANSLUCENT_TORCH_POSITION float3(0.00,0.0,0)		// position of torch light in view space
	#define TRANSLUCENT_TORCH_ANGLE_COS float(0.8434)			// cosinus of a half of a torch cone angle

/////////////////////////////////////////////////////////////
// ������������� ���������
							// �������� "������"
	#define ID_DETECTOR_1_DETECT_RADIUS float(20.0)			// ������ �������������� ����������. ������ ������������ ������ �� ��������� � �������
	#define ID_DETECTOR_1_COLOR float4(1.0,1.0,1.0,1.0)		// ���� ���������
	#define ID_DETECTOR_1_POWER float(6.0)					// ������� ���������

							// �������� "�������"
	#define ID_DETECTOR_2_DETECT_RADIUS float(20.0)			// ������ �������������� ����������. ������ ������������ ������ �� ��������� � �������
	#define ID_DETECTOR_2_COLOR float4(0.1,1.0,0.0,1.0)		// ���� ���������
	#define ID_DETECTOR_2_POWER float(5.0)					// ������� ���������
	#define ID_DETECTOR_2_CENTER float2(0.2559, 0.2305)		// ���������� ���������� ������ ������ ���������. ������ ��� ����� ��������
	#define ID_DETECTOR_2_SECTOR float(0.7)					// ������ ��������� �� ������. ��� ������ ��������, ��� ������ ������. ����� 1.0 �� �������!

							// �������� "�����"
	#define ID_DETECTOR_3_DETECT_RADIUS float(35.0)			// ������ �������������� ����������. ������ ������������ ������ �� ��������� � �������
	#define ID_DETECTOR_3_COLOR float4(0.1,1.0,0.0,1.0)		// ���� ���������
	#define ID_DETECTOR_3_POWER float(6.0)					// ������� ���������
	#define ID_DETECTOR_3_DOT_RADIUS float(0.01)			// ������ ����� ��������� �� ������
	#define ID_DETECTOR_3_SCREEN_CORNERS float4(0.4668, 0.8398, 0.1035, 0.2891)		// ���������� ���������� ����� ������ � ������� (max u,max v,min u,min v). ������ ��� ����� ��������
	#define USE_ANOMALY_DETECTION							// ��������� ������ ��������� ��������
	#define ID_DETECTOR_3_AN_COLOR float4(1.0,0.0,0.0,1.0)	// ���� ��������� ��������
	#define ID_DETECTOR_3_AN_DOT_RADIUS float(0.02)			// ������ ����� �������� �� ������
	#define ID_DETECTOR_3_NUM_COLOR float4(1.0,0.0,0.0,1.0)	// ���� ��������� ���� �� ������

/////////////////////////////////////////////////////////////
// ������
	#define DETAIL_TEXTURE_MULTIPLIER float(1.5)			// ��������� ������� ��������� ��������

// ������ ���������
	#define CONTRAST_FILTER_COEF float(0.4)					// ������� ���������

// ��������������
	#define COLOR_GRADING_LUMINANCE float3(0.213, 0.715, 0.072)		// ���� ��� ������� ������������� ����������. ����� �� ������.

/////////////////////////////////////////////////////////////
// FXAA
	#define FXAA_QUALITY__SUBPIX float(0.5)			// Choose the amount of sub-pixel aliasing removal.
													// This can effect sharpness.
													//   1.00 - upper limit (softer)
													//   0.75 - default amount of filtering
													//   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
													//   0.25 - almost off
													//   0.00 - completely off
	#define FXAA_QUALITY__EDGE_THRESHOLD float(0.063)    // The minimum amount of local contrast required to apply algorithm.
													//   0.333 - too little (faster)
													//   0.250 - low quality
													//   0.166 - default
													//   0.125 - high quality 
													//   0.063 - overkill (slower)
	#define FXAA_QUALITY__EDGE_THRESHOLD_MIN float(0.0312)    // Trims the algorithm from processing darks.
													//   0.0833 - upper limit (default, the start of visible unfiltered edges)
													//   0.0625 - high quality (faster)
													//   0.0312 - visible limit (slower)
#endif 