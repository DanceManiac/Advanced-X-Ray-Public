#include "pch_script.h"
#include "Actor_Flags.h"
#include "hudmanager.h"
#ifdef DEBUG

#	include "PHDebug.h"
#endif // DEBUG
#include "alife_space.h"
#include "hit.h"
#include "PHDestroyable.h"
#include "Car.h"
#include "xrserver_objects_alife_monsters.h"
#include "CameraLook.h"
#include "CameraFirstEye.h"
#include "effectorfall.h"
#include "EffectorBobbing.h"
#include "ActorEffector.h"
#include "EffectorZoomInertion.h"
#include "SleepEffector.h"
#include "character_info.h"
#include "CustomOutfit.h"
#include "actorcondition.h"
#include "UIGameCustom.h"
#include "ui/UIArtefactPanel.h"
#include "../xrphysics/matrix_utils.h"
#include "clsid_game.h"
#include "game_cl_base_weapon_usage_statistic.h"
#include "Grenade.h"
#include "Torch.h"

// breakpoints
#include "../xrEngine/xr_input.h"
//
#include "Actor.h"
#include "ActorAnimation.h"
#include "actor_anim_defs.h"
#include "HudItem.h"
#include "ai_sounds.h"
#include "ai_space.h"
#include "trade.h"
#include "inventory.h"
//#include "Physics.h"
#include "level.h"
#include "GamePersistent.h"
#include "game_cl_base.h"
#include "game_cl_single.h"
#include "xrmessages.h"
#include "string_table.h"
#include "usablescriptobject.h"
#include "../xrEngine/cl_intersect.h"
//#include "ExtendedGeom.h"
#include "alife_registry_wrappers.h"
#include "../Include/xrRender/Kinematics.h"
#include "artefact.h"
#include "CharacterPhysicsSupport.h"
#include "material_manager.h"
#include "../xrphysics/IColisiondamageInfo.h"
#include "ui/UIMainIngameWnd.h"
#include "map_manager.h"
#include "GameTaskManager.h"
#include "actor_memory.h"
#include "Script_Game_Object.h"
#include "Game_Object_Space.h"
#include "script_callback_ex.h"
#include "InventoryBox.h"
#include "location_manager.h"
#include "player_hud.h"

#include "../Include/xrRender/UIRender.h"

#include "ai_object_location.h"

#include "DynamicHudGlass.h"
#include "ActorNightVision.h"
#include "AdvancedXrayGameConstants.h"
#include "../xrphysics/actorcameracollision.h"
#include "../../xrCore/_detail_collision_point.h"
#include "../xrEngine/Rain.h"
#include "../xrPhysics/ElevatorState.h"
#include "CustomDetector.h"
#include "CustomBackpack.h"

const u32		patch_frames	= 50;
const float		respawn_delay	= 1.f;
const float		respawn_auto	= 7.f;

static float IReceived = 0;
static float ICoincidenced = 0;
extern float cammera_into_collision_shift ;

//skeleton
static Fbox		bbStandBox;
static Fbox		bbCrouchBox;
static Fvector	vFootCenter;
static Fvector	vFootExt;

Flags32			psActorFlags={/*AF_DYNAMIC_MUSIC|*/AF_GODMODE_RT};

u32	death_camera_mode = READ_IF_EXISTS(pAdvancedSettings, r_u32, "gameplay", "death_camera_mode", 1);;

ENGINE_API extern int ps_r__ShaderNVG;
extern ENGINE_API Fvector4 ps_ssfx_hud_drops_1;

extern bool g_block_all_except_movement;

std::atomic<bool> isHidingInProgress(false);
std::atomic<bool> CheckNVGAnimNeeded(false);
std::atomic<bool> CleanMaskAnimNeeded(false);

CActor::CActor() : CEntityAlive(), current_ik_cam_shift(0)
{
	encyclopedia_registry	= xr_new<CEncyclopediaRegistryWrapper	>();
	game_news_registry		= xr_new<CGameNewsRegistryWrapper		>();
	// Cameras
	cameras[eacFirstEye]	= xr_new<CCameraFirstEye>				(this);
	cameras[eacFirstEye]->Load("actor_firsteye_cam");

	//Alundaio -psp always
	/*
	if(strstr(Core.Params,"-psp"))
		psActorFlags.set(AF_PSP, TRUE);
	else
		psActorFlags.set(AF_PSP, FALSE);
	*/

	//if( psActorFlags.test(AF_PSP) )
	//{
		cameras[eacLookAt]		= xr_new<CCameraLook2>				(this);
		cameras[eacLookAt]->Load("actor_look_cam_psp");
	//}
	//else
	//{
	//	cameras[eacLookAt]		= xr_new<CCameraLook>				(this);
	//	cameras[eacLookAt]->Load("actor_look_cam");
	//}
	//-Alundaio
	cameras[eacFreeLook]	= xr_new<CCameraLook>					(this);
	cameras[eacFreeLook]->Load("actor_free_cam");

	cam_active				= eacFirstEye;
	fPrevCamPos				= 0.0f;
	vPrevCamDir.set			(0.f,0.f,1.f);
	fCurAVelocity			= 0.0f;
	fFPCamYawMagnitude		= 0.0f; //--#SM+#--
	fFPCamPitchMagnitude	= 0.0f; //--#SM+#--
	// эффекторы
	pCamBobbing				= 0;

	r_torso.yaw				= 0;
	r_torso.pitch			= 0;
	r_torso.roll			= 0;
	r_torso_tgt_roll		= 0;
	r_model_yaw				= 0;
	r_model_yaw_delta		= 0;
	r_model_yaw_dest		= 0;

	b_DropActivated			= 0;
	f_DropPower				= 0.f;

	m_fRunFactor			= 2.f;
	m_fCrouchFactor			= 0.2f;
	m_fClimbFactor			= 1.f;
	m_fCamHeightFactor		= 0.87f;

	m_fFallTime				=	s_fFallTime;
	m_bAnimTorsoPlayed		=	false;

	m_pPhysicsShell			=	NULL;

	m_fFeelGrenadeRadius	=	10.0f;
	m_fFeelGrenadeTime      =	1.0f;
	
	m_holder				=	NULL;
	m_holderID				=	u16(-1);


#ifdef DEBUG
	Device.seqRender.Add	(this,REG_PRIORITY_LOW);
#endif

	//разрешить использование пояса в inventory
	inventory().SetBeltUseful(true);

	m_pPersonWeLookingAt	= NULL;
	m_pVehicleWeLookingAt	= NULL;
	m_pObjectWeLookingAt	= NULL;
	m_bPickupMode			= false;

	pStatGraph				= NULL;

	m_pActorEffector		= NULL;

	SetZoomAimingMode		(false);

	m_sDefaultObjAction		= NULL;

	m_fSprintFactor			= 4.f;

	//hFriendlyIndicator.create(FVF::F_LIT,RCache.Vertex.Buffer(),RCache.QuadIB);

	m_pUsableObject			= NULL;


	m_anims					= xr_new<SActorMotions>();
	m_vehicle_anims			= xr_new<SActorVehicleAnims>();
	m_entity_condition		= NULL;
	m_iLastHitterID			= u16(-1);
	m_iLastHittingWeaponID	= u16(-1);
	m_statistic_manager		= NULL;
	//-----------------------------------------------------------------------------------
	m_memory				= g_dedicated_server ? 0 : xr_new<CActorMemory>(this);
	m_bOutBorder			= false;
	m_hit_probability		= 1.f;
	m_feel_touch_characters = 0;
	//-----------------------------------------------------------------------------------
	m_dwILastUpdateTime		= 0;

	m_location_manager		= xr_new<CLocationManager>(this);
	m_block_sprint_counter	= 0;

	m_iBaseArtefactCount	= 0;

	// Alex ADD: for smooth crouch fix
	CurrentHeight			= -1.f;

	m_night_vision			= NULL;
	m_bNightVisionAllow		= true;
	m_bNightVisionOn		= false;

	m_bMaskAnimActivated	= false;
	m_bNVGActivated			= false;
	m_bEatAnimActive		= false;
	m_bActionAnimInProcess	= false;
	m_disabled_hitmarks		= false;
	m_bNVGSwitched			= false;
	m_bMaskClear			= false;
	m_iNVGAnimLength		= 0;
	m_iActionTiming			= 0;
	m_iMaskAnimLength		= 0;

	ActorSkills				= nullptr;
	TimerManager			= nullptr;

	m_fDevicesPsyFactor		= 0.0f;

	m_iTrySprintCounter		= 0;

	m_iInventoryCapacity	= 50;
	m_iInventoryFullness	= 0;
	m_iInventoryFullnessCtrl = 0;
}


CActor::~CActor()
{
	xr_delete				(m_location_manager);

	xr_delete				(m_memory);

	xr_delete				(encyclopedia_registry);
	xr_delete				(game_news_registry);
#ifdef DEBUG
	Device.seqRender.Remove(this);
#endif
	//xr_delete(Weapons);
	for (int i=0; i<eacMaxCam; ++i) xr_delete(cameras[i]);

	m_HeavyBreathSnd.destroy();
	m_BloodSnd.destroy		();
	m_DangerSnd.destroy		();

	xr_delete				(m_pActorEffector);

	xr_delete				(m_pPhysics_support);

	xr_delete				(m_anims);
	xr_delete				(m_vehicle_anims);

	xr_delete				(m_night_vision);
	xr_delete				(ActorSkills);
	xr_delete				(TimerManager);
}

void CActor::reinit	()
{
	character_physics_support()->movement()->CreateCharacter		();
	character_physics_support()->movement()->SetPhysicsRefObject	(this);
	CEntityAlive::reinit						();
	CInventoryOwner::reinit						();

	character_physics_support()->in_Init		();
	material().reinit							();

	m_pUsableObject								= NULL;
	if (!g_dedicated_server)
		memory().reinit							();
	
	set_input_external_handler					(0);
	m_time_lock_accel							= 0;
}

void CActor::reload	(LPCSTR section)
{
	CEntityAlive::reload		(section);
	CInventoryOwner::reload		(section);
	material().reload			(section);
	CStepManager::reload		(section);
	if (!g_dedicated_server)
		memory().reload			(section);
	m_location_manager->reload	(section);
}
void set_box(LPCSTR section, CPHMovementControl &mc, u32 box_num )
{
	Fbox	bb;Fvector	vBOX_center,vBOX_size;
	// m_PhysicMovementControl: BOX
	string64 buff, buff1;
	strconcat( sizeof(buff), buff, "ph_box",itoa( box_num, buff1, 10 ),"_center" );
	vBOX_center= pSettings->r_fvector3	(section, buff	);
	strconcat( sizeof(buff), buff, "ph_box",itoa( box_num, buff1, 10 ),"_size" );
	vBOX_size	= pSettings->r_fvector3	(section, buff);
	vBOX_size.y += cammera_into_collision_shift/2.f;
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	mc.SetBox		(box_num,bb);
}
void CActor::Load	(LPCSTR section )
{
	// Msg						("Loading actor: %s",section);
	inherited::Load				(section);
	material().Load				(section);
	CInventoryOwner::Load		(section);
	m_location_manager->Load	(section);

	if (GameID() == eGameIDSingle)
		OnDifficultyChanged		();
	//////////////////////////////////////////////////////////////////////////
	ISpatial*		self			=	smart_cast<ISpatial*> (this);
	if (self)	{
		self->spatial.type	|=	STYPE_VISIBLEFORAI;
		self->spatial.type	&= ~STYPE_REACTTOSOUND;
	}
	//////////////////////////////////////////////////////////////////////////

	// m_PhysicMovementControl: General
	//m_PhysicMovementControl->SetParent		(this);


	/*
	Fbox	bb;Fvector	vBOX_center,vBOX_size;
	// m_PhysicMovementControl: BOX
	vBOX_center= pSettings->r_fvector3	(section,"ph_box2_center"	);
	vBOX_size	= pSettings->r_fvector3	(section,"ph_box2_size"		);
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	character_physics_support()->movement()->SetBox		(2,bb);

	// m_PhysicMovementControl: BOX
	vBOX_center= pSettings->r_fvector3	(section,"ph_box1_center"	);
	vBOX_size	= pSettings->r_fvector3	(section,"ph_box1_size"		);
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	character_physics_support()->movement()->SetBox		(1,bb);

	// m_PhysicMovementControl: BOX
	vBOX_center= pSettings->r_fvector3	(section,"ph_box0_center"	);
	vBOX_size	= pSettings->r_fvector3	(section,"ph_box0_size"		);
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	character_physics_support()->movement()->SetBox		(0,bb);
	*/
	


	
	
	
	
	//// m_PhysicMovementControl: Foots
	//Fvector	vFOOT_center= pSettings->r_fvector3	(section,"ph_foot_center"	);
	//Fvector	vFOOT_size	= pSettings->r_fvector3	(section,"ph_foot_size"		);
	//bb.set	(vFOOT_center,vFOOT_center); bb.grow(vFOOT_size);
	////m_PhysicMovementControl->SetFoots	(vFOOT_center,vFOOT_size);

	// m_PhysicMovementControl: Crash speed and mass
	float	cs_min		= pSettings->r_float	(section,"ph_crash_speed_min"	);
	float	cs_max		= pSettings->r_float	(section,"ph_crash_speed_max"	);
	float	mass		= pSettings->r_float	(section,"ph_mass"				);
	character_physics_support()->movement()->SetCrashSpeeds	(cs_min,cs_max);
	character_physics_support()->movement()->SetMass		(mass);
	if(pSettings->line_exist(section,"stalker_restrictor_radius"))
		character_physics_support()->movement()->SetActorRestrictorRadius(rtStalker,pSettings->r_float(section,"stalker_restrictor_radius"));
	if(pSettings->line_exist(section,"stalker_small_restrictor_radius"))
		character_physics_support()->movement()->SetActorRestrictorRadius(rtStalkerSmall,pSettings->r_float(section,"stalker_small_restrictor_radius"));
	if(pSettings->line_exist(section,"medium_monster_restrictor_radius"))
		character_physics_support()->movement()->SetActorRestrictorRadius(rtMonsterMedium,pSettings->r_float(section,"medium_monster_restrictor_radius"));
	character_physics_support()->movement()->Load(section);

	set_box( section, *character_physics_support()->movement(), 2 );
	set_box( section, *character_physics_support()->movement(), 1 );
	set_box( section, *character_physics_support()->movement(), 0 );

	m_fWalkAccel				= pSettings->r_float(section,"walk_accel");	
	m_fJumpSpeed				= pSettings->r_float(section,"jump_speed");
	m_fRunFactor				= pSettings->r_float(section,"run_coef");
	m_fRunBackFactor			= pSettings->r_float(section,"run_back_coef");
	m_fWalkBackFactor			= pSettings->r_float(section,"walk_back_coef");
	m_fCrouchFactor				= pSettings->r_float(section,"crouch_coef");
	m_fClimbFactor				= pSettings->r_float(section,"climb_coef");
	m_fSprintFactor				= pSettings->r_float(section,"sprint_koef");

	m_fWalk_StrafeFactor		= READ_IF_EXISTS(pSettings, r_float, section, "walk_strafe_coef", 1.0f);
	m_fRun_StrafeFactor			= READ_IF_EXISTS(pSettings, r_float, section, "run_strafe_coef", 1.0f);


	m_fCamHeightFactor			= pSettings->r_float(section,"camera_height_factor");
	character_physics_support()->movement()->SetJumpUpVelocity(m_fJumpSpeed);
	float AirControlParam		= pSettings->r_float(section,"air_control_param"	);
	character_physics_support()->movement()->SetAirControlParam(AirControlParam);

	m_fPickupInfoRadius			= READ_IF_EXISTS(pSettings, r_float, section, "pickup_info_radius", 0.0f);

	m_fFeelGrenadeRadius		= pSettings->r_float(section,"feel_grenade_radius");
	m_fFeelGrenadeTime			= pSettings->r_float(section,"feel_grenade_time");
	m_fFeelGrenadeTime			*= 1000.0f;
	
	character_physics_support()->in_Load		(section);
	

if(!g_dedicated_server)
{
	LPCSTR hit_snd_sect = pSettings->r_string(section,"hit_sounds");
	for(int hit_type=0; hit_type<(int)ALife::eHitTypeMax; ++hit_type)
	{
		LPCSTR hit_name = ALife::g_cafHitType2String((ALife::EHitType)hit_type);
		LPCSTR hit_snds = READ_IF_EXISTS(pSettings, r_string, hit_snd_sect, hit_name, "");
		int cnt = _GetItemCount(hit_snds);
		string128		tmp;
		VERIFY			(cnt!=0);
		for(int i=0; i<cnt;++i)
		{
			sndHit[hit_type].push_back		(ref_sound());
			sndHit[hit_type].back().create	(_GetItem(hit_snds,i,tmp),st_Effect,sg_SourceType);
		}
		char buf[256];

		::Sound->create		(sndDie[0],			strconcat(sizeof(buf),buf,*cName(),"\\die0"), st_Effect,SOUND_TYPE_MONSTER_DYING);
		::Sound->create		(sndDie[1],			strconcat(sizeof(buf),buf,*cName(),"\\die1"), st_Effect,SOUND_TYPE_MONSTER_DYING);
		::Sound->create		(sndDie[2],			strconcat(sizeof(buf),buf,*cName(),"\\die2"), st_Effect,SOUND_TYPE_MONSTER_DYING);
		::Sound->create		(sndDie[3],			strconcat(sizeof(buf),buf,*cName(),"\\die3"), st_Effect,SOUND_TYPE_MONSTER_DYING);

		m_HeavyBreathSnd.create	(pSettings->r_string(section,"heavy_breath_snd"), st_Effect,SOUND_TYPE_MONSTER_INJURING);
		m_BloodSnd.create		(pSettings->r_string(section,"heavy_blood_snd"), st_Effect,SOUND_TYPE_MONSTER_INJURING);
		m_DangerSnd.create		(pSettings->r_string(section,"heavy_danger_snd"), st_Effect,SOUND_TYPE_MONSTER_INJURING);
	}

	if (this == Level().CurrentEntity()) //--#SM+#--      [reset some render flags]
	{
		if (g_pGamePersistent && g_pGamePersistent->m_pGShaderConstants)
			g_pGamePersistent->m_pGShaderConstants->m_blender_mode.set(0.f, 0.f, 0.f, 0.f);
	}
}
//Alundaio -psp always
//if (psActorFlags.test(AF_PSP))
//    cam_Set(eacLookAt);
//else
//-Alundaio
	cam_Set					(eacFirstEye);

	// sheduler
	shedule.t_min				= shedule.t_max = 1;

	// настройки дисперсии стрельбы
	m_fDispBase					= pSettings->r_float		(section,"disp_base"		 );
	m_fDispBase					= deg2rad(m_fDispBase);

	m_fDispAim					= pSettings->r_float		(section,"disp_aim"		 );
	m_fDispAim					= deg2rad(m_fDispAim);

	m_fDispVelFactor			= pSettings->r_float		(section,"disp_vel_factor"	 );
	m_fDispAccelFactor			= pSettings->r_float		(section,"disp_accel_factor" );
	m_fDispCrouchFactor			= pSettings->r_float		(section,"disp_crouch_factor");
	m_fDispCrouchNoAccelFactor	= pSettings->r_float		(section,"disp_crouch_no_acc_factor");

	LPCSTR							default_outfit = READ_IF_EXISTS(pSettings,r_string,section,"default_outfit",0);
	SetDefaultVisualOutfit			(default_outfit);

	invincibility_fire_shield_1st	= READ_IF_EXISTS(pSettings,r_string,section,"Invincibility_Shield_1st",0);
	invincibility_fire_shield_3rd	= READ_IF_EXISTS(pSettings,r_string,section,"Invincibility_Shield_3rd",0);
//-----------------------------------------
	m_AutoPickUp_AABB				= READ_IF_EXISTS(pSettings,r_fvector3,section,"AutoPickUp_AABB",Fvector().set(0.02f, 0.02f, 0.02f));
	m_AutoPickUp_AABB_Offset		= READ_IF_EXISTS(pSettings,r_fvector3,section,"AutoPickUp_AABB_offs",Fvector().set(0, 0, 0));

	CStringTable string_table;
	m_sCharacterUseAction			= "character_use";
	m_sDeadCharacterUseAction		= "dead_character_use";
	m_sDeadCharacterUseOrDragAction	= "dead_character_use_or_drag";
	m_sDeadCharacterDontUseAction	= "dead_character_dont_use";
	m_sCarCharacterUseAction		= "car_character_use";
	m_sInventoryItemUseAction		= "inventory_item_use";
	m_sInventoryBoxUseAction		= "inventory_box_use";
	//---------------------------------------------------------------------
	m_sHeadShotParticle	= READ_IF_EXISTS(pSettings,r_string,section,"HeadShotParticle",0);

	if (!ActorSkills && GameConstants::GetActorSkillsEnabled())
		ActorSkills = xr_new<CActorSkills>();

	if (!TimerManager)
		TimerManager = xr_new<CTimerManager>();

	m_iBaseArtefactCount = READ_IF_EXISTS(pSettings, r_u32, section, "base_artefacts_count", 0);
	m_iInventoryCapacity = READ_IF_EXISTS(pSettings, r_u32, section, "inventory_capacity", 50);
}

void CActor::PHHit(SHit &H)
{
	m_pPhysics_support->in_Hit( H, false );
}

struct playing_pred
{
	IC	bool	operator()			(ref_sound &s)
	{
		return	(NULL != s._feedback() );
	}
};

void	CActor::Hit							(SHit* pHDS)
{
	bool b_initiated = pHDS->aim_bullet; // physics strike by poltergeist

	pHDS->aim_bullet = false;

	SHit& HDS	= *pHDS;
	if( HDS.hit_type<ALife::eHitTypeBurn || HDS.hit_type >= ALife::eHitTypeMax )
	{
		string256	err;
		xr_sprintf		(err, "Unknown/unregistered hit type [%d]", HDS.hit_type);
		R_ASSERT2	(0, err );
	
	}
#ifdef DEBUG
	if(ph_dbg_draw_mask.test(phDbgCharacterControl)) {
		DBG_OpenCashedDraw();
		Fvector to;to.add(Position(),Fvector().mul(HDS.dir,HDS.phys_impulse()));
		DBG_DrawLine(Position(),to,color_xrgb(124,124,0));
		DBG_ClosedCashedDraw(500);
	}
#endif // DEBUG

	bool bPlaySound = true;
	if (!g_Alive()) bPlaySound = false;

	if (!IsGameTypeSingle() && !g_dedicated_server)
	{
		game_PlayerState* ps = Game().GetPlayerByGameID(ID());
		if (ps && ps->testFlag(GAME_PLAYER_FLAG_INVINCIBLE))
		{
			bPlaySound = false;
			if (Device.dwFrame != last_hit_frame &&
				HDS.bone() != BI_NONE)
			{		
				// вычислить позицию и направленность партикла
				Fmatrix pos; 

				CParticlesPlayer::MakeXFORM(this,HDS.bone(),HDS.dir,HDS.p_in_bone_space,pos);

				// установить particles
				CParticlesObject* ps = NULL;

				if (eacFirstEye == cam_active && this == Level().CurrentEntity())
					ps = CParticlesObject::Create(invincibility_fire_shield_1st,TRUE);
				else
					ps = CParticlesObject::Create(invincibility_fire_shield_3rd,TRUE);

				ps->UpdateParent(pos,Fvector().set(0.f,0.f,0.f));
				GamePersistent().ps_needtoplay.push_back(ps);
			};
		};
		 

		last_hit_frame = Device.dwFrame;
	};

	if(	!g_dedicated_server				&& 
		!sndHit[HDS.hit_type].empty()	&&
		conditions().PlayHitSound(pHDS)	)
	{
		ref_sound& S			= sndHit[HDS.hit_type][Random.randI(sndHit[HDS.hit_type].size())];
		bool b_snd_hit_playing	= sndHit[HDS.hit_type].end() != std::find_if(sndHit[HDS.hit_type].begin(), sndHit[HDS.hit_type].end(), playing_pred());

		if(ALife::eHitTypeExplosion == HDS.hit_type)
		{
			if (this == Level().CurrentControlEntity())
			{
				S.set_volume(10.0f);
				if(!m_sndShockEffector){
					m_sndShockEffector = xr_new<SndShockEffector>();
					m_sndShockEffector->Start(this, float(S.get_length_sec()*1000.0f), HDS.damage() );
				}
			}
			else
				bPlaySound = false;
		}
		if (bPlaySound && !b_snd_hit_playing) 
		{
			Fvector point		= Position();
			point.y				+= CameraHeight();
			S.play_at_pos		(this, point);
		}
	}

	
	//slow actor, only when he gets hit
	m_hit_slowmo = conditions().HitSlowmo(pHDS);

	//---------------------------------------------------------------
	if(		(Level().CurrentViewEntity()==this) && 
			!g_dedicated_server && 
			(HDS.hit_type == ALife::eHitTypeFireWound) )
	{
		CObject* pLastHitter			= Level().Objects.net_Find(m_iLastHitterID);
		CObject* pLastHittingWeapon		= Level().Objects.net_Find(m_iLastHittingWeaponID);
		HitSector						(pLastHitter, pLastHittingWeapon);
	}

	if( (mstate_real&mcSprint) && Level().CurrentControlEntity() == this && conditions().DisableSprint(pHDS) )
	{
		bool const is_special_burn_hit_2_self	=	(pHDS->who == this) && (pHDS->boneID == BI_NONE) && 
													((pHDS->hit_type==ALife::eHitTypeBurn)/*||(pHDS->hit_type==ALife::eHitTypeLightBurn)*/);
		if ( !is_special_burn_hit_2_self )
		{
			mstate_wishful	&=~mcSprint;
		}
	}
	if(!g_dedicated_server && !m_disabled_hitmarks)
	{
		bool b_fireWound = (pHDS->hit_type==ALife::eHitTypeFireWound || pHDS->hit_type==ALife::eHitTypeWound_2);
		b_initiated		 = b_initiated && (pHDS->hit_type==ALife::eHitTypeStrike);
	
		if(b_fireWound || b_initiated)
			HitMark			(HDS.damage(), HDS.dir, HDS.who, HDS.bone(), HDS.p_in_bone_space, HDS.impulse, HDS.hit_type);
	}

	if(IsGameTypeSingle())	
	{
		float hit_power				= HitArtefactsOnBelt(HDS.damage(), HDS.hit_type);

		if(GodMode())
		{
			HDS.power				= 0.0f;
			inherited::Hit			(&HDS);
			return;
		}else 
		{
			HDS.power				= hit_power;
			HDS.add_wound			= true;
			inherited::Hit			(&HDS);
		}
	}else
	{
		m_bWasBackStabbed			= false;
		if (HDS.hit_type == ALife::eHitTypeWound_2 && Check_for_BackStab_Bone(HDS.bone()))
		{
			// convert impulse into local coordinate system
			Fmatrix					mInvXForm;
			mInvXForm.invert		(XFORM());
			Fvector					vLocalDir;
			mInvXForm.transform_dir	(vLocalDir,HDS.dir);
			vLocalDir.invert		();

			Fvector a				= {0,0,1};
			float res				= a.dotproduct(vLocalDir);
			if (res < -0.707)
			{
				game_PlayerState* ps = Game().GetPlayerByGameID(ID());
				
				if (!ps || !ps->testFlag(GAME_PLAYER_FLAG_INVINCIBLE))						
					m_bWasBackStabbed = true;
			}
		};
		
		float hit_power				= 0.0f;

		if (m_bWasBackStabbed) 
			hit_power				= (HDS.damage() == 0) ? 0 : 100000.0f;
		else 
			hit_power				= HitArtefactsOnBelt(HDS.damage(), HDS.hit_type);

		HDS.power					= hit_power;
		HDS.add_wound				= true;
		inherited::Hit				(&HDS);

		if(OnServer() && !g_Alive() && HDS.hit_type==ALife::eHitTypeExplosion)
		{
			game_PlayerState* ps							= Game().GetPlayerByGameID(ID());
			Game().m_WeaponUsageStatistic->OnExplosionKill	(ps, HDS);
		}
	}
}

void CActor::HitMark	(float P, 
						 Fvector dir,			
						 CObject* who_object, 
						 s16 element, 
						 Fvector position_in_bone_space, 
						 float impulse,  
						 ALife::EHitType hit_type_)
{
	// hit marker
	if ( /*(hit_type==ALife::eHitTypeFireWound||hit_type==ALife::eHitTypeWound_2) && */
			g_Alive() && Local() && (Level().CurrentEntity()==this) )	
	{
		HUD().HitMarked				(0, P, dir);

		CEffectorCam* ce = Cameras().GetCamEffector((ECamEffectorType)effFireHit);
		if( ce ) return;

		int id						= -1;
		Fvector						cam_pos,cam_dir,cam_norm;
		cam_Active()->Get			(cam_pos,cam_dir,cam_norm);
		cam_dir.normalize_safe		();
		dir.normalize_safe			();

		float ang_diff				= angle_difference	(cam_dir.getH(), dir.getH());
		Fvector						cp;
		cp.crossproduct				(cam_dir,dir);
		bool bUp					=(cp.y>0.0f);

		Fvector cross;
		cross.crossproduct			(cam_dir, dir);
		VERIFY						(ang_diff>=0.0f && ang_diff<=PI);

		float _s1 = PI_DIV_8;
		float _s2 = _s1 + PI_DIV_4;
		float _s3 = _s2 + PI_DIV_4;
		float _s4 = _s3 + PI_DIV_4;

		if ( ang_diff <= _s1 )
		{
			id = 2;
		}
		else if ( ang_diff > _s1 && ang_diff <= _s2 )
		{
			id = (bUp)?5:7;
		}
		else if ( ang_diff > _s2 && ang_diff <= _s3 )
		{
			id = (bUp)?3:1;
		}
		else if ( ang_diff > _s3 && ang_diff <= _s4 )
		{
			id = (bUp)?4:6;
		}
		else if( ang_diff > _s4 )
		{
			id = 0;
		}
		else
		{
			VERIFY(0);
		}

		string64 sect_name;
		xr_sprintf( sect_name, "effector_fire_hit_%d", id );
		AddEffector( this, effFireHit, sect_name, P * 0.001f );

	}//if hit_type
}

void CActor::HitSignal(float perc, Fvector& vLocalDir, CObject* who, s16 element)
{
	if (g_Alive()) 
	{

		// check damage bone
		Fvector D;
		XFORM().transform_dir(D,vLocalDir);

		float	yaw, pitch;
		D.getHP(yaw,pitch);
		IRenderVisual *pV = Visual();
		IKinematicsAnimated *tpKinematics = smart_cast<IKinematicsAnimated*>(pV);
		IKinematics *pK = smart_cast<IKinematics*>(pV);
		VERIFY(tpKinematics);
#pragma todo("Dima to Dima : forward-back bone impulse direction has been determined incorrectly!")
		MotionID motion_ID = m_anims->m_normal.m_damage[iFloor(pK->LL_GetBoneInstance(element).get_param(1) + (angle_difference(r_model_yaw + r_model_yaw_delta,yaw) <= PI_DIV_2 ? 0 : 1))];
		float power_factor = perc/100.f; clamp(power_factor,0.f,1.f);
		VERIFY(motion_ID.valid());
		tpKinematics->PlayFX(motion_ID,power_factor);
	}
}
void start_tutorial(LPCSTR name);
void CActor::Die	(CObject* who)
{
#ifdef DEBUG
	Msg("--- Actor [%s] dies !", this->Name());
#endif // #ifdef DEBUG
	inherited::Die		(who);

	if (OnServer())
	{	
		xr_vector<CInventorySlot>::iterator I = inventory().m_slots.begin();
		xr_vector<CInventorySlot>::iterator E = inventory().m_slots.end();


		for (u32 slot_idx=0 ; I != E; ++I,++slot_idx)
		{
			if (slot_idx == inventory().GetActiveSlot()) 
			{
				if((*I).m_pIItem)
				{
					if (IsGameTypeSingle())
						(*I).m_pIItem->SetDropManual(TRUE);
					else
					{
						//This logic we do on a server site
						/*
						if ((*I).m_pIItem->object().CLS_ID != CLSID_OBJECT_W_KNIFE)
						{
							(*I).m_pIItem->SetDropManual(TRUE);
						}*/							
					}
				};
			continue;
			}
			else
			{
				CCustomOutfit *pOutfit = smart_cast<CCustomOutfit *> ((*I).m_pIItem);
				if (pOutfit) continue;
			};
			if((*I).m_pIItem) 
				inventory().Ruck((*I).m_pIItem);
		};


		///!!! чистка пояса
		TIItemContainer &l_blist = inventory().m_belt;
		while (!l_blist.empty())	
			inventory().Ruck(l_blist.front());

		if (!IsGameTypeSingle())
		{
			//if we are on server and actor has PDA - destroy PDA
			TIItemContainer &l_rlist	= inventory().m_ruck;
			for(TIItemContainer::iterator l_it = l_rlist.begin(); l_rlist.end() != l_it; ++l_it)
			{
				if (GameID() == eGameIDArtefactHunt)
				{
					CArtefact* pArtefact = smart_cast<CArtefact*> (*l_it);
					if (pArtefact)
					{
						(*l_it)->SetDropManual(TRUE);
						continue;
					};
				};

				if ((*l_it)->object().CLS_ID == CLSID_OBJECT_PLAYERS_BAG)
				{
					(*l_it)->SetDropManual(TRUE);
					continue;
				};
			};
		};
	};

	if(!g_dedicated_server)
	{
		::Sound->play_at_pos	(sndDie[Random.randI(SND_DIE_COUNT)],this,Position());

		m_HeavyBreathSnd.stop	();
		m_BloodSnd.stop			();		
		m_DangerSnd.stop		();		
	}

	if(IsGameTypeSingle())
	{
		if (death_camera_mode == 1)
			cam_Set				(eacFreeLook);
		else if (death_camera_mode == 2)
			cam_Set				(eacLookAt);
		else if (death_camera_mode == 3)
			cam_Set				(eacFirstEye);

		HUD().GetUI()->UIGame()->HideShownDialogs();
		start_tutorial		("game_over");
	}
	xr_delete				(m_sndShockEffector);
}

void	CActor::SwitchOutBorder(bool new_border_state)
{
	if(new_border_state)
	{
		callback(GameObject::eExitLevelBorder)(lua_game_object());
	}
	else 
	{
//.		Msg("enter level border");
		callback(GameObject::eEnterLevelBorder)(lua_game_object());
	}
	m_bOutBorder=new_border_state;
}

void CActor::g_Physics			(Fvector& _accel, float jump, float dt)
{
	// Correct accel
	Fvector		accel;
	accel.set					(_accel);
	m_hit_slowmo				-=	dt;
	if(m_hit_slowmo<0)			m_hit_slowmo = 0.f;

	accel.mul					(1.f-m_hit_slowmo);

	
	

	if(g_Alive())
	{
		if(mstate_real&mcClimb&&!cameras[eacFirstEye]->bClampYaw)
				accel.set(0.f,0.f,0.f);
		character_physics_support()->movement()->Calculate			(accel,cameras[cam_active]->vDirection,0,jump,dt,false);
		bool new_border_state=character_physics_support()->movement()->isOutBorder();
		if(m_bOutBorder!=new_border_state && Level().CurrentControlEntity() == this)
		{
			SwitchOutBorder(new_border_state);
		}
#ifdef DEBUG
		if(!psActorFlags.test(AF_NO_CLIP))
			character_physics_support()->movement()->GetPosition		(Position());
#else //DEBUG
		character_physics_support()->movement()->GetPosition		(Position());
#endif //DEBUG
		character_physics_support()->movement()->bSleep				=false;
	}

	if (Local() && g_Alive()) 
	{
		if(character_physics_support()->movement()->gcontact_Was)
			Cameras().AddCamEffector		(xr_new<CEffectorFall> (character_physics_support()->movement()->gcontact_Power));

		if (!fis_zero(character_physics_support()->movement()->gcontact_HealthLost))	
		{
			VERIFY( character_physics_support() );
			VERIFY( character_physics_support()->movement() );
			ICollisionDamageInfo* di=character_physics_support()->movement()->CollisionDamageInfo();
			VERIFY( di );
			bool b_hit_initiated =  di->GetAndResetInitiated();
			Fvector hdir;di->HitDir(hdir);
			SetHitInfo(this, NULL, 0, Fvector().set(0, 0, 0), hdir);
			//				Hit	(m_PhysicMovementControl->gcontact_HealthLost,hdir,di->DamageInitiator(),m_PhysicMovementControl->ContactBone(),di->HitPos(),0.f,ALife::eHitTypeStrike);//s16(6 + 2*::Random.randI(0,2))
			if (Level().CurrentControlEntity() == this)
			{
				
				SHit HDS = SHit(character_physics_support()->movement()->gcontact_HealthLost,
//.								0.0f,
								hdir,
								di->DamageInitiator(),
								character_physics_support()->movement()->ContactBone(),
								di->HitPos(),
								0.f,
								di->HitType(),
								0.0f, 
								b_hit_initiated);
//				Hit(&HDS);

				NET_Packet	l_P;
				HDS.GenHeader(GE_HIT, ID());
				HDS.whoID = di->DamageInitiator()->ID();
				HDS.weaponID = di->DamageInitiator()->ID();
				HDS.Write_Packet(l_P);

				u_EventSend	(l_P);
			}
		}
	}
}

float g_fov = READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "Camera_FOV", 67.5f);

float CActor::currentFOV()
{
	if (!psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT|HUD_WEAPON_RT2))
		return g_fov;

	CWeapon* pWeapon = smart_cast<CWeapon*>(inventory().ActiveItem());	

	if (eacFirstEye == cam_active && pWeapon &&
		pWeapon->IsZoomed() && 
		( !pWeapon->ZoomTexture() || (!pWeapon->IsRotatingToZoom() && pWeapon->ZoomTexture()) )
		 )
	{
		return pWeapon->GetZoomFactor() * (0.75f);
	}else
	{
		return g_fov;
	}
}

void CActor::UpdateCL	()
{
	UpdateInventoryOwner			(Device.dwTimeDelta);

	if (load_screen_renderer.IsActive() && inventory().GetActiveSlot() == PDA_SLOT)
		inventory().Activate(NO_ACTIVE_SLOT);

	if(m_feel_touch_characters>0)
	{
		for(xr_vector<CObject*>::iterator it = feel_touch.begin(); it != feel_touch.end(); it++)
		{
			CPhysicsShellHolder	*sh = smart_cast<CPhysicsShellHolder*>(*it);
			if(sh&&sh->character_physics_support())
			{
				sh->character_physics_support()->movement()->UpdateObjectBox(character_physics_support()->movement()->PHCharacter());
			}
		}
	}
	if(m_holder)
		m_holder->UpdateEx( currentFOV() );

	m_snd_noise -= 0.3f*Device.fTimeDelta;

	inherited::UpdateCL				();
	m_pPhysics_support->in_UpdateCL	();


	if (g_Alive()) 
		PickupModeUpdate	();	

	PickupModeUpdate_COD();

	SetZoomAimingMode		(false);
	CWeapon* pWeapon		= smart_cast<CWeapon*>(inventory().ActiveItem());	

	cam_Update(float(Device.dwTimeDelta)/1000.0f, currentFOV());

	if(Level().CurrentEntity() && this->ID()==Level().CurrentEntity()->ID() )
	{
		psHUD_Flags.set( HUD_CROSSHAIR_RT2, true );
		psHUD_Flags.set( HUD_DRAW_RT, true );
	}
	if(pWeapon )
	{
		if(pWeapon->IsZoomed())
		{
			float full_fire_disp = pWeapon->GetFireDispersion(true);

			CEffectorZoomInertion* S = smart_cast<CEffectorZoomInertion*>	(Cameras().GetCamEffector(eCEZoom));
			if(S) 
				S->SetParams(full_fire_disp);

			SetZoomAimingMode		(true);
		}

		if(Level().CurrentEntity() && this->ID()==Level().CurrentEntity()->ID() )
		{
			float fire_disp_full = pWeapon->GetFireDispersion(true);
			m_fdisp_controller.SetDispertion(fire_disp_full);
			
			fire_disp_full = m_fdisp_controller.GetCurrentDispertion();

			//--#SM+#-- +SecondVP+        FOV (Sin!) [fix for crosshair shaking while SecondVP]
			if (!Device.m_SecondViewport.IsSVPActive())
				HUD().SetCrosshairDisp(fire_disp_full, 0.02f);

			HUD().ShowCrosshair(pWeapon->use_crosshair());
#ifdef DEBUG
			HUD().SetFirstBulletCrosshairDisp(pWeapon->GetFirstBulletDisp());
#endif
			
			BOOL B = ! ((mstate_real & mcLookout) && !IsGameTypeSingle());

			psHUD_Flags.set( HUD_WEAPON_RT, B );

			B = B && pWeapon->show_crosshair();

			psHUD_Flags.set( HUD_CROSSHAIR_RT2, B );
			

			
			psHUD_Flags.set( HUD_DRAW_RT,		pWeapon->show_indicators() );

			//      [Update SecondVP with weapon data]
			pWeapon->UpdateSecondVP(); //--#SM+#-- +SecondVP+
			bool bUseMark = !!pWeapon->bMarkCanShow();
			bool bNVEnbl = !!pWeapon->bNVsecondVPstatus;

			//
			if (g_pGamePersistent && g_pGamePersistent->m_pGShaderConstants)
			{
				g_pGamePersistent->m_pGShaderConstants->hud_params.x = pWeapon->GetZRotatingFactor();  //--#SM+#--
				g_pGamePersistent->m_pGShaderConstants->hud_params.y = pWeapon->GetSecondVPFov(); //--#SM+#--
				g_pGamePersistent->m_pGShaderConstants->hud_params.z = bUseMark; //--#SM+#--
				g_pGamePersistent->m_pGShaderConstants->m_blender_mode.x = bNVEnbl;  //--#SM+#--
			}
		}

	}
	else
	{
		if(Level().CurrentEntity() && this->ID()==Level().CurrentEntity()->ID() )
		{
			HUD().SetCrosshairDisp(0.f);
			HUD().ShowCrosshair(false);

			//
			if (g_pGamePersistent && g_pGamePersistent->m_pGShaderConstants)
			{
				g_pGamePersistent->m_pGShaderConstants->hud_params.set(0.f, 0.f, 0.f, 0.f); //--#SM+#--
				g_pGamePersistent->m_pGShaderConstants->m_blender_mode.set(0.f, 0.f, 0.f, 0.f); //--#SM+#--
			}

			//    [Turn off SecondVP]
			//CWeapon::UpdateSecondVP();
			Device.m_SecondViewport.SetSVPActive(false); //--#SM+#-- +SecondVP+
		}
	}

	UpdateDefferedMessages();

	if (g_Alive()) 
		CStepManager::update(this==Level().CurrentViewEntity());

	spatial.type |=STYPE_REACTTOSOUND;

	if(m_sndShockEffector)
	{
		if (this == Level().CurrentViewEntity())
		{
			m_sndShockEffector->Update();

			if(!m_sndShockEffector->InWork() || !g_Alive())
				xr_delete(m_sndShockEffector);
		}
		else
			xr_delete(m_sndShockEffector);
	}
	Fmatrix							trans;
	if(cam_Active() == cam_FirstEye())
	{
		Cameras().hud_camera_Matrix		(trans);
	}else
		Cameras().camera_Matrix			(trans);
	
	
	if (IsFocused())
	{
		trans.c.sub(Device.vCameraPosition);
		g_player_hud->update(trans);
	}

	// Ascii hud rain drops support
	{
		float animSpeed = 1.f;
		float buildSpeed = 2.f;
		float dryingSpeed = 1.f;
		float rainFactor = g_pGamePersistent->Environment().CurrentEnv->rain_density;
		float rainHemi{};
		CEffect_Rain* rain = g_pGamePersistent->pEnvironment->eff_Rain;

		if (rainFactor > 0.f)
		{
			// get rain hemi
			if (rain)
			{
				rainHemi = rain->GetRainHemi();
			}
			else
			{
				CObject* E = g_pGameLevel->CurrentViewEntity();
				if (E && E->renderable_ROS())
				{
					float* hemi_cube = E->renderable_ROS()->get_luminocity_hemi_cube();
					float hemi_val = _max(hemi_cube[0], hemi_cube[1]);
					hemi_val = _max(hemi_val, hemi_cube[2]);
					hemi_val = _max(hemi_val, hemi_cube[3]);
					hemi_val = _max(hemi_val, hemi_cube[5]);

					rainHemi = hemi_val;
				}
			}

			if (rainHemi > 0.15f)
			{
				float rainSpeedFactor = (1.5f - rainFactor) * 10.f;
				m_dropsAnimIncrementor += (animSpeed * Device.fTimeDelta) / rainSpeedFactor;
				m_dropsIntensity += (buildSpeed * Device.fTimeDelta) / 100.f;
			}
			else
			{
				m_dropsIntensity -= (dryingSpeed * Device.fTimeDelta) / 100.f;
			}
		}
		else
		{
			m_dropsIntensity -= (dryingSpeed * Device.fTimeDelta) / 100.f;
		}

		clamp(m_dropsIntensity, 0.f, 1.f);

		if (fsimilar(m_dropsAnimIncrementor, FLT_MAX, 1.f))
			m_dropsAnimIncrementor = 0.f;

		ps_ssfx_hud_drops_1.x = m_dropsAnimIncrementor;
		ps_ssfx_hud_drops_1.y = m_dropsIntensity;
	}

	g_pGamePersistent->devices_shader_data.device_global_psy_influence = m_fDevicesPsyFactor;
	g_pGamePersistent->devices_shader_data.device_psy_zone_influence = HUD().GetUI()->UIGame()->get_zone_cur_power(ALife::eHitTypeTelepatic) * 2;
	g_pGamePersistent->devices_shader_data.device_radiation_zone_influence = HUD().GetUI()->UIGame()->get_zone_cur_power(ALife::eHitTypeRadiation) * 2;

	luabind::functor<bool> m_functor;

	if (ai().script_engine().functor("mfs_functions.devices_check_surge", m_functor))
		m_functor();
}

float	NET_Jump = 0;

#include "ai\monsters\ai_monster_utils.h"

void CActor::set_state_box(u32	mstate)
{
		if ( mstate & mcCrouch)
	{
		if (isActorAccelerated(mstate_real, IsZoomAimingMode()))
			character_physics_support()->movement()->ActivateBox(1, true);
		else
			character_physics_support()->movement()->ActivateBox(2, true);
	}
	else 
		character_physics_support()->movement()->ActivateBox(0, true);
}
void CActor::shedule_Update	(u32 DT)
{
	setSVU							(OnServer());
//.	UpdateInventoryOwner			(DT);

	if(IsFocused())
	{
		BOOL bHudView				= HUDview();
		if(bHudView)
		{
			CInventoryItem* pInvItem	= inventory().ActiveItem();	
			if( pInvItem )
			{
				CHudItem* pHudItem		= smart_cast<CHudItem*>(pInvItem);	
				if(pHudItem)
				{
					if( pHudItem->IsHidden() )
					{
						g_player_hud->detach_item	(pHudItem);
					}
					else
					{
						g_player_hud->attach_item	(pHudItem);
					}
				}
			}else
			{
					g_player_hud->detach_item_idx	( 0 );
					//Msg("---No active item in inventory(), item 0 detached.");
			}
		}
		else
		{
			g_player_hud->detach_all_items();
			//Msg("---No hud view found, all items detached.");
		}
			
	}

	if(m_holder || !getEnabled() || !Ready())
	{
		m_sDefaultObjAction				= NULL;
		inherited::shedule_Update		(DT);
		return;
	}

	clamp					(DT,0u,100u);
	float	dt	 			=  float(DT)/1000.f;

	// Check controls, create accel, prelimitary setup "mstate_real"
	
	//----------- for E3 -----------------------------
//	if (Local() && (OnClient() || Level().CurrentEntity()==this))
	if (Level().CurrentControlEntity() == this && !Level().IsDemoPlay())
	//------------------------------------------------
	{
		g_cl_CheckControls		(mstate_wishful,NET_SavedAccel,NET_Jump,dt);
		{
			/*
			if (mstate_real & mcJump)
			{
				NET_Packet	P;
				u_EventGen(P, GE_ACTOR_JUMPING, ID());
				P.w_sdir(NET_SavedAccel);
				P.w_float(NET_Jump);
				u_EventSend(P);
			}
			*/
		}
		g_cl_Orientate			(mstate_real,dt);
		g_Orientate				(mstate_real,dt);

		g_Physics				(NET_SavedAccel,NET_Jump,dt);
		
		g_cl_ValidateMState		(dt,mstate_wishful);
		g_SetAnimation			(mstate_real);
		
		// Check for game-contacts
		Fvector C; float R;		
		//m_PhysicMovementControl->GetBoundingSphere	(C,R);
		
		Center( C );
		R = Radius();
		feel_touch_update( C, R );
		Feel_Grenade_Update( m_fFeelGrenadeRadius );

		// Dropping
		if (b_DropActivated)	{
			f_DropPower			+= dt*0.1f;
			clamp				(f_DropPower,0.f,1.f);
		} else {
			f_DropPower			= 0.f;
		}
		if (!Level().IsDemoPlay())
		{		
		mstate_wishful &=~mcAccel;
		mstate_wishful &=~mcLStrafe;
		mstate_wishful &=~mcRStrafe;
		mstate_wishful &=~mcLLookout;
		mstate_wishful &=~mcRLookout;
		mstate_wishful &=~mcFwd;
		mstate_wishful &=~mcBack;

		if (!psActorFlags.test(AF_CROUCH_TOGGLE))
			mstate_wishful &= ~mcCrouch;
		}
	}
	else 
	{
		make_Interpolation();
	
		if (NET.size())
		{
			
//			NET_SavedAccel = NET_Last.p_accel;
//			mstate_real = mstate_wishful = NET_Last.mstate;

			g_sv_Orientate				(mstate_real,dt			);
			g_Orientate					(mstate_real,dt			);
			g_Physics					(NET_SavedAccel,NET_Jump,dt	);			
			if (!m_bInInterpolation)
				g_cl_ValidateMState			(dt,mstate_wishful);
			g_SetAnimation				(mstate_real);

			set_state_box(NET_Last.mstate);


		}	
		mstate_old = mstate_real;
	}

	if (this == Level().CurrentViewEntity())
	{
		UpdateMotionIcon		(mstate_real);
	};

	NET_Jump = 0;


	inherited::shedule_Update	(DT);

	//эффектор включаемый при ходьбе
	if (!pCamBobbing)
	{
		pCamBobbing = xr_new<CEffectorBobbing>	();
		Cameras().AddCamEffector			(pCamBobbing);
	}
	pCamBobbing->SetState						(mstate_real, conditions().IsLimping(), IsZoomAimingMode());

	//звук тяжелого дыхания при уталости и хромании
	if(this==Level().CurrentControlEntity() && !g_dedicated_server )
	{
		if(conditions().IsLimping() && g_Alive()){
			if(!m_HeavyBreathSnd._feedback()){
				m_HeavyBreathSnd.play_at_pos(this, Fvector().set(0,ACTOR_HEIGHT,0), sm_Looped | sm_2D);
			}else{
				m_HeavyBreathSnd.set_position(Fvector().set(0,ACTOR_HEIGHT,0));
			}
		}else if(m_HeavyBreathSnd._feedback()){
			m_HeavyBreathSnd.stop		();
		}

		// -------------------------------
		float bs = conditions().BleedingSpeed();
		if(bs>0.6f)
		{
			Fvector snd_pos;
			snd_pos.set(0,ACTOR_HEIGHT,0);
			if(!m_BloodSnd._feedback())
				m_BloodSnd.play_at_pos(this, snd_pos, sm_Looped | sm_2D);
			else
				m_BloodSnd.set_position(snd_pos);

			float v = bs+0.25f;

			m_BloodSnd.set_volume	(v);
		}else{
			if(m_BloodSnd._feedback())
				m_BloodSnd.stop();
		}

		if(!g_Alive()&&m_BloodSnd._feedback())
				m_BloodSnd.stop();
		// -------------------------------
		bs = conditions().GetZoneDanger();
		if ( bs > 0.1f )
		{
			Fvector snd_pos;
			snd_pos.set(0,ACTOR_HEIGHT,0);
			if(!m_DangerSnd._feedback())
				m_DangerSnd.play_at_pos(this, snd_pos, sm_Looped | sm_2D);
			else
				m_DangerSnd.set_position(snd_pos);

			float v = bs+0.25f;
//			Msg( "bs            = %.2f", bs );

			m_DangerSnd.set_volume	(v);
		}
		else
		{
			if(m_DangerSnd._feedback())
				m_DangerSnd.stop();
		}

		if(!g_Alive()&&m_DangerSnd._feedback())
			m_DangerSnd.stop();
	}
	
	//если в режиме HUD, то сама модель актера не рисуется
	if(!character_physics_support()->IsRemoved())
		setVisible(!HUDview());

	//что актер видит перед собой
	collide::rq_result& RQ				= HUD().GetCurrentRayQuery();
	

	float InteractionDist;
	if (eacFirstEye != cam_active) 
	{
		InteractionDist = 2.4f;
	}
	else 
	{
		InteractionDist = 2.0f;
	}

	float dist_to_obj = RQ.range;
	if (RQ.O && eacFirstEye != cam_active)
		dist_to_obj = get_bone_position(this, "bip01_spine").distance_to((smart_cast<CGameObject*>(RQ.O))->Position());
	if (!input_external_handler_installed() && RQ.O && dist_to_obj < InteractionDist)
	{
		m_pObjectWeLookingAt			= smart_cast<CGameObject*>(RQ.O);
		
		CGameObject						*game_object = smart_cast<CGameObject*>(RQ.O);
		m_pUsableObject					= smart_cast<CUsableScriptObject*>(game_object);
		m_pInvBoxWeLookingAt			= smart_cast<CInventoryBox*>(game_object);
		m_pPersonWeLookingAt			= smart_cast<CInventoryOwner*>(game_object);
		m_pVehicleWeLookingAt			= smart_cast<CHolderCustom*>(game_object);
		CEntityAlive* pEntityAlive		= smart_cast<CEntityAlive*>(game_object);
		
		if (GameID() == eGameIDSingle )
		{
			if (m_pUsableObject && m_pUsableObject->tip_text())
			{
				m_sDefaultObjAction = CStringTable().translate( m_pUsableObject->tip_text() );
			}
			else
			{
				if (m_pPersonWeLookingAt && pEntityAlive->g_Alive() && m_pPersonWeLookingAt->IsTalkEnabled())
				{
					m_sDefaultObjAction = m_sCharacterUseAction;
				}
				else if (pEntityAlive && !pEntityAlive->g_Alive())
				{
					if ( m_pPersonWeLookingAt && m_pPersonWeLookingAt->deadbody_closed_status() )
					{
						m_sDefaultObjAction = m_sDeadCharacterDontUseAction;
					}
					else
					{
					bool b_allow_drag = !!pSettings->line_exist("ph_capture_visuals",pEntityAlive->cNameVisual());
				
					if(b_allow_drag)
						{
						m_sDefaultObjAction = m_sDeadCharacterUseOrDragAction;
						}
						else if ( pEntityAlive->cast_inventory_owner() )
						{
						m_sDefaultObjAction = m_sDeadCharacterUseAction;
						}
					} // m_pPersonWeLookingAt
				}
				else if (m_pVehicleWeLookingAt)
				{
					m_sDefaultObjAction = m_sCarCharacterUseAction;
				}
				else if (	m_pObjectWeLookingAt && 
							m_pObjectWeLookingAt->cast_inventory_item() && 
							m_pObjectWeLookingAt->cast_inventory_item()->CanTake() )
				{
					m_sDefaultObjAction = m_sInventoryItemUseAction;
				}
				else 
				{
					m_sDefaultObjAction = NULL;
			}
		}
	}
	}
	else 
	{
		m_pPersonWeLookingAt	= NULL;
		m_sDefaultObjAction		= NULL;
		m_pUsableObject			= NULL;
		m_pObjectWeLookingAt	= NULL;
		m_pVehicleWeLookingAt	= NULL;
		m_pInvBoxWeLookingAt	= NULL;
	}

//	UpdateSleep									();

	//для свойст артефактов, находящихся на поясе
	UpdateArtefactsOnBeltAndOutfit				();
	m_pPhysics_support->in_shedule_Update		(DT);
	Check_for_AutoPickUp						();

	if (Actor())
		DynamicHudGlass::UpdateDynamicHudGlass();

	UpdateInventoryItems();

	if (GameConstants::GetActorSkillsEnabled())
		UpdateSkills();

	if (CheckNVGAnimNeeded.load())
	{
		StartNVGAnimation();
		CheckNVGAnimNeeded.store(false);
	}

	if (CleanMaskAnimNeeded.load())
	{
		CleanMask();
		CleanMaskAnimNeeded.store(false);
	}

	if (m_bActionAnimInProcess)
	{
		if (m_bNVGActivated)
			UpdateNVGUseAnim();

		if (m_bMaskAnimActivated)
			UpdateMaskUseAnim();
	}

	inventory().UpdateUseAnim(this);

	if (TimerManager)
	{
		TimerManager->Update();

		/*TimerManager->SetOnTimerStopCallback([](std::string name)	// Does not work after restarting the game
		{
		});*/
	}
};
#include "debug_renderer.h"
void CActor::renderable_Render	()
{
	VERIFY(_valid(XFORM()));
	inherited::renderable_Render();
	//if(1/*!HUDview()*/)
	if ((cam_active == eacFirstEye && // first eye cam
		::Render->get_generation() == ::Render->GENERATION_R2 && // R2
		::Render->active_phase() == 1) // shadow map rendering on R2	
		||
		!(IsFocused() &&
		(cam_active == eacFirstEye) &&
			((!m_holder) || (m_holder && m_holder->allowWeapon() && m_holder->HUDView())))
		)
		//{
		CInventoryOwner::renderable_Render();
	//}
	//VERIFY(_valid(XFORM()));
}

BOOL CActor::renderable_ShadowGenerate	() 
{
	if(m_holder)
		return FALSE;
	
	return inherited::renderable_ShadowGenerate();
}



void CActor::g_PerformDrop	( )
{
	b_DropActivated			= FALSE;

	PIItem pItem			= inventory().ActiveItem();
	if (0==pItem)			return;

	if(pItem->IsQuestItem()) return;

	u32 s					= inventory().GetActiveSlot();
	if(inventory().m_slots[s].m_bPersistent)	return;

	pItem->SetDropManual	(TRUE);
}

bool CActor::use_default_throw_force()
{
	if (!g_Alive())
		return false;
	
	return true;
}

float CActor::missile_throw_force()
{
	return 0.f;
}

#ifdef DEBUG
extern	BOOL	g_ShowAnimationInfo		;
#endif // DEBUG
// HUD

void CActor::OnHUDDraw	(CCustomHUD*)
{
	R_ASSERT						(IsFocused());
	if(! ( (mstate_real & mcLookout) && !IsGameTypeSingle() ) )
		g_player_hud->render_hud		();


#if 0//ndef NDEBUG
	if (Level().CurrentControlEntity() == this && g_ShowAnimationInfo)
	{
		string128 buf;
		HUD().Font().pFontStat->SetColor	(0xffffffff);
		HUD().Font().pFontStat->OutSet		(170,530);
		HUD().Font().pFontStat->OutNext	("Position:      [%3.2f, %3.2f, %3.2f]",VPUSH(Position()));
		HUD().Font().pFontStat->OutNext	("Velocity:      [%3.2f, %3.2f, %3.2f]",VPUSH(m_PhysicMovementControl->GetVelocity()));
		HUD().Font().pFontStat->OutNext	("Vel Magnitude: [%3.2f]",m_PhysicMovementControl->GetVelocityMagnitude());
		HUD().Font().pFontStat->OutNext	("Vel Actual:    [%3.2f]",m_PhysicMovementControl->GetVelocityActual());
		switch (m_PhysicMovementControl->Environment())
		{
		case CPHMovementControl::peOnGround:	xr_strcpy(buf,"ground");			break;
		case CPHMovementControl::peInAir:		xr_strcpy(buf,"air");				break;
		case CPHMovementControl::peAtWall:		xr_strcpy(buf,"wall");				break;
		}
		HUD().Font().pFontStat->OutNext	(buf);

		if (IReceived != 0)
		{
			float Size = 0;
			Size = HUD().Font().pFontStat->GetSize();
			HUD().Font().pFontStat->SetSize(Size*2);
			HUD().Font().pFontStat->SetColor	(0xffff0000);
			HUD().Font().pFontStat->OutNext ("Input :		[%3.2f]", ICoincidenced/IReceived * 100.0f);
			HUD().Font().pFontStat->SetSize(Size);
		};
	};
#endif
}

void CActor::RenderIndicator			(Fvector dpos, float r1, float r2, const ui_shader &IndShader)
{
	if (!g_Alive()) return;


	UIRender->StartPrimitive(4, IUIRender::ptTriStrip, IUIRender::pttLIT);

	CBoneInstance& BI = smart_cast<IKinematics*>(Visual())->LL_GetBoneInstance(u16(m_head));
	Fmatrix M;
	smart_cast<IKinematics*>(Visual())->CalculateBones	();
	M.mul						(XFORM(),BI.mTransform);

	Fvector pos = M.c; pos.add(dpos);
	const Fvector& T        = Device.vCameraTop;
	const Fvector& R        = Device.vCameraRight;
	Fvector Vr, Vt;
	Vr.x            = R.x*r1;
	Vr.y            = R.y*r1;
	Vr.z            = R.z*r1;
	Vt.x            = T.x*r2;
	Vt.y            = T.y*r2;
	Vt.z            = T.z*r2;

	Fvector         a,b,c,d;
	a.sub           (Vt,Vr);
	b.add           (Vt,Vr);
	c.invert        (a);
	d.invert        (b);

	UIRender->PushPoint(d.x+pos.x,d.y+pos.y,d.z+pos.z, 0xffffffff, 0.f,1.f);
	UIRender->PushPoint(a.x+pos.x,a.y+pos.y,a.z+pos.z, 0xffffffff, 0.f,0.f);
	UIRender->PushPoint(c.x+pos.x,c.y+pos.y,c.z+pos.z, 0xffffffff, 1.f,1.f);
	UIRender->PushPoint(b.x+pos.x,b.y+pos.y,b.z+pos.z, 0xffffffff, 1.f,0.f);
	//pv->set         (d.x+pos.x,d.y+pos.y,d.z+pos.z, 0xffffffff, 0.f,1.f);        pv++;
	//pv->set         (a.x+pos.x,a.y+pos.y,a.z+pos.z, 0xffffffff, 0.f,0.f);        pv++;
	//pv->set         (c.x+pos.x,c.y+pos.y,c.z+pos.z, 0xffffffff, 1.f,1.f);        pv++;
	//pv->set         (b.x+pos.x,b.y+pos.y,b.z+pos.z, 0xffffffff, 1.f,0.f);        pv++;
	// render	
	//dwCount 				= u32(pv-pv_start);
	//RCache.Vertex.Unlock	(dwCount,hFriendlyIndicator->vb_stride);

	UIRender->CacheSetXformWorld(Fidentity);
	//RCache.set_xform_world		(Fidentity);
	UIRender->SetShader(*IndShader);
	//RCache.set_Shader			(IndShader);
	//RCache.set_Geometry			(hFriendlyIndicator);
	//RCache.Render	   			(D3DPT_TRIANGLESTRIP,dwOffset,0, dwCount, 0, 2);
	UIRender->FlushPrimitive();
};

static float mid_size = 0.097f;
static float fontsize = 15.0f;
static float upsize	= 0.33f;
void CActor::RenderText				(LPCSTR Text, Fvector dpos, float* pdup, u32 color)
{
	if (!g_Alive()) return;
	
	CBoneInstance& BI = smart_cast<IKinematics*>(Visual())->LL_GetBoneInstance(u16(m_head));
	Fmatrix M;
	smart_cast<IKinematics*>(Visual())->CalculateBones	();
	M.mul						(XFORM(),BI.mTransform);
	//------------------------------------------------
	Fvector v0, v1;
	v0.set(M.c); v1.set(M.c);
	Fvector T        = Device.vCameraTop;
	v1.add(T);

	Fvector v0r, v1r;
	Device.mFullTransform.transform(v0r,v0);
	Device.mFullTransform.transform(v1r,v1);
	float size = v1r.distance_to(v0r);
	CGameFont* pFont = HUD().Font().pFontArial14;
	if (!pFont) return;
//	float OldFontSize = pFont->GetHeight	();	
	float delta_up = 0.0f;
	if (size < mid_size) delta_up = upsize;
	else delta_up = upsize*(mid_size/size);
	dpos.y += delta_up;
	if (size > mid_size) size = mid_size;
//	float NewFontSize = size/mid_size * fontsize;
	//------------------------------------------------
	M.c.y += dpos.y;

	Fvector4 v_res;	
	Device.mFullTransform.transform(v_res,M.c);

	if (v_res.z < 0 || v_res.w < 0)	return;
	if (v_res.x < -1.f || v_res.x > 1.f || v_res.y<-1.f || v_res.y>1.f) return;

	float x = (1.f + v_res.x)/2.f * (Device.dwWidth);
	float y = (1.f - v_res.y)/2.f * (Device.dwHeight);

	pFont->SetAligment	(CGameFont::alCenter);
	pFont->SetColor		(color);
//	pFont->SetHeight	(NewFontSize);
	pFont->Out			(x,y,Text);
	//-------------------------------------------------
//	pFont->SetHeight(OldFontSize);
	*pdup = delta_up;
};

void CActor::SetPhPosition(const Fmatrix &transform)
{
	if(!m_pPhysicsShell){ 
		character_physics_support()->movement()->SetPosition(transform.c);
	}
	//else m_phSkeleton->S
}

void CActor::ForceTransform(const Fmatrix& m)
{
	//if( !g_Alive() )
	//			return;
	//VERIFY(_valid(m));
	//XFORM().set( m );
	//if( character_physics_support()->movement()->CharacterExist() )
	//		character_physics_support()->movement()->EnableCharacter();
	//character_physics_support()->set_movement_position( m.c );
	//character_physics_support()->movement()->SetVelocity( 0, 0, 0 );

	character_physics_support()->ForceTransform( m );
	const float block_damage_time_seconds = 2.f;
	if(!IsGameTypeSingle())
		character_physics_support()->movement()->BlockDamageSet( u64( block_damage_time_seconds/fixed_step ) );
}

ENGINE_API extern float		psHUD_FOV;
float CActor::Radius()const
{ 
	float R		= inherited::Radius();
	CWeapon* W	= smart_cast<CWeapon*>(inventory().ActiveItem());
	if (W) R	+= W->Radius();
	//	if (HUDview()) R *= 1.f/psHUD_FOV;
	return R;
}

bool		CActor::use_bolts				() const
{
	if (!IsGameTypeSingle()) return false;
	return CInventoryOwner::use_bolts();
};

int		g_iCorpseRemove = 1;

bool  CActor::NeedToDestroyObject() const
{
	if(IsGameTypeSingle())
	{
		return false;
	}
	else 
	{
		if (g_Alive()) return false;
		if (g_iCorpseRemove == -1) return false;
		if (g_iCorpseRemove == 0 && m_bAllowDeathRemove) return true;
		if(TimePassedAfterDeath()>m_dwBodyRemoveTime && m_bAllowDeathRemove)
			return true;
		else
			return false;
	}
}

ALife::_TIME_ID	 CActor::TimePassedAfterDeath()	const
{
	if(!g_Alive())
		return Level().timeServer() - GetLevelDeathTime();
	else
		return 0;
}


void CActor::OnItemTake			(CInventoryItem *inventory_item)
{
	CInventoryOwner::OnItemTake(inventory_item);
	if (OnClient()) return;
}

void CActor::OnItemDrop(CInventoryItem *inventory_item)
{
	CInventoryOwner::OnItemDrop(inventory_item);

	CArtefact* artefact = smart_cast<CArtefact*>(inventory_item);
	if(artefact && artefact->m_eItemCurrPlace == EItemPlaceBelt)
		MoveArtefactBelt(artefact, false);

	CCustomOutfit* outfit		= smart_cast<CCustomOutfit*>(inventory_item);
	if(outfit && inventory_item->m_eItemCurrPlace==EItemPlaceSlot)
	{
		outfit->ApplySkinModel	(this, false, false);
	}

	CWeapon* weapon = smart_cast<CWeapon*>(inventory_item);
	if (weapon && inventory_item->m_eItemCurrPlace == EItemPlaceSlot)
	{
		weapon->OnZoomOut();
		if (weapon->GetRememberActorNVisnStatus())
			weapon->EnableActorNVisnAfterZoom();
	}
}


void CActor::OnItemDropUpdate ()
{
	CInventoryOwner::OnItemDropUpdate		();

	TIItemContainer::iterator				I = inventory().m_all.begin();
	TIItemContainer::iterator				E = inventory().m_all.end();
	
	for ( ; I != E; ++I)
		if( !(*I)->IsInvalid() && !attached(*I))
			attach(*I);
}


void CActor::OnItemRuck		(CInventoryItem *inventory_item, EItemPlace previous_place)
{
	CInventoryOwner::OnItemRuck(inventory_item, previous_place);

	CArtefact* artefact = smart_cast<CArtefact*>(inventory_item);
	if(artefact && previous_place == EItemPlaceBelt)
		MoveArtefactBelt(artefact, false);
}
void CActor::OnItemBelt		(CInventoryItem *inventory_item, EItemPlace previous_place)
{
	CInventoryOwner::OnItemBelt(inventory_item, previous_place);

	CArtefact* artefact = smart_cast<CArtefact*>(inventory_item);
	if(artefact)
		MoveArtefactBelt(artefact, true);
}


void CActor::MoveArtefactBelt(const CArtefact* artefact, bool on_belt)
{
	VERIFY(artefact);

	//повесить артефакт на пояс
	if(on_belt)
	{
		VERIFY(m_ArtefactsOnBelt.end() == std::find(m_ArtefactsOnBelt.begin(), m_ArtefactsOnBelt.end(), artefact));
		m_ArtefactsOnBelt.push_back(artefact);
	}
	else
	{
		xr_vector<const CArtefact*>::iterator it = std::remove(m_ArtefactsOnBelt.begin(), m_ArtefactsOnBelt.end(), artefact);
		VERIFY(it != m_ArtefactsOnBelt.end());
		m_ArtefactsOnBelt.erase(it);
	}

	if (Level().CurrentViewEntity() && Level().CurrentViewEntity() == this && HUD().GetUI()->UIMainIngameWnd->UIArtefactsPanel)
		HUD().GetUI()->UIMainIngameWnd->UIArtefactsPanel->InitIcons(m_ArtefactsOnBelt);
}

#define ARTEFACTS_UPDATE_TIME 0.100f

void CActor::UpdateArtefactsOnBeltAndOutfit()
{
	static float update_time = 0;

	float f_update_time = 0;

	if(update_time<ARTEFACTS_UPDATE_TIME)
	{
		update_time += conditions().fdelta_time();
		return;
	}
	else
	{
		f_update_time	= update_time;
		update_time		= 0.0f;
	}

	if (xr_strcmp("from_belt", GameConstants::GetAfInfluenceMode()) == 0 || xr_strcmp("from_ruck_only_rad", GameConstants::GetAfInfluenceMode()) == 0)
		UpdateArtefactsOnBelt();
	else if (xr_strcmp("from_ruck", GameConstants::GetAfInfluenceMode()) == 0)
		UpdateArtefactsInRuck();

	CCustomOutfit* outfit = GetOutfit();
	if ( outfit )
	{
		conditions().ChangeBleeding		(outfit->m_fBleedingRestoreSpeed  * f_update_time);
		conditions().ChangeHealth		(outfit->m_fHealthRestoreSpeed    * f_update_time);
		conditions().ChangePower		(outfit->m_fPowerRestoreSpeed     * f_update_time);
		conditions().ChangeSatiety		(outfit->m_fSatietyRestoreSpeed   * f_update_time);
		conditions().ChangeThirst		(outfit->m_fThirstRestoreSpeed    * f_update_time);
		conditions().ChangeRadiation	(outfit->m_fRadiationRestoreSpeed * f_update_time);
		conditions().ChangeIntoxication	(outfit->m_fIntoxicationRestoreSpeed * f_update_time);
		conditions().ChangeSleepeness	(outfit->m_fSleepenessRestoreSpeed * f_update_time);
		conditions().ChangeAlcoholism	(outfit->m_fAlcoholismRestoreSpeed * f_update_time);
		conditions().ChangeNarcotism	(outfit->m_fNarcotismRestoreSpeed * f_update_time);
		conditions().ChangePsyHealth	(outfit->m_fPsyHealthRestoreSpeed * f_update_time);
	}
	else
	{
		/* 		if (GetNightVisionStatus())
		{
		} */
	}

	CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
	if (backpack)
	{
		conditions().ChangeBleeding		(backpack->m_fBleedingRestoreSpeed		* f_update_time);
		conditions().ChangeHealth		(backpack->m_fHealthRestoreSpeed		* f_update_time);
		conditions().ChangePower		(backpack->m_fPowerRestoreSpeed			* f_update_time);
		conditions().ChangeSatiety		(backpack->m_fSatietyRestoreSpeed		* f_update_time);
		conditions().ChangeThirst		(backpack->m_fThirstRestoreSpeed		* f_update_time);
		conditions().ChangeRadiation	(backpack->m_fRadiationRestoreSpeed		* f_update_time);
		conditions().ChangeIntoxication	(backpack->m_fIntoxicationRestoreSpeed	* f_update_time);
		conditions().ChangeSleepeness	(backpack->m_fSleepenessRestoreSpeed	* f_update_time);
		conditions().ChangeAlcoholism	(backpack->m_fAlcoholismRestoreSpeed	* f_update_time);
		conditions().ChangeNarcotism	(backpack->m_fNarcotismRestoreSpeed		* f_update_time);
		conditions().ChangePsyHealth	(backpack->m_fPsyHealthRestoreSpeed		* f_update_time);
	}
}

void CActor::UpdateArtefactsOnBelt()
{
	static float update_time = 0;

	float f_update_time = 0;

	if (update_time < ARTEFACTS_UPDATE_TIME)
	{
		update_time += conditions().fdelta_time();
		return;
	}
	else
	{
		f_update_time = update_time;
		update_time = 0.0f;
	}

	TIItemContainer::iterator it = inventory().m_belt.begin();
	TIItemContainer::iterator ite = inventory().m_belt.end();
	TIItemContainer::iterator itR = inventory().m_ruck.begin();
	TIItemContainer::iterator iteR = inventory().m_ruck.end();

	if (GameConstants::GetArtefactsDegradation() && xr_strcmp("from_ruck", GameConstants::GetArtefactDegradationMode()) == 0)
	{
		for (itR; iteR != itR; ++itR)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itR);

			if (artefact)
				artefact->UpdateDegradation();
		}
	}

	if (xr_strcmp("from_ruck_only_rad", GameConstants::GetAfInfluenceMode()) == 0)
	{
		for (itR; iteR != itR; ++itR)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itR);
			if (artefact)
			{
				conditions().ChangeRadiation(artefact->m_fRadiationRestoreSpeed * f_update_time);

				if (GameConstants::GetArtefactsDegradation() && (xr_strcmp("from_ruck", GameConstants::GetArtefactDegradationMode()) == 0))
					artefact->UpdateDegradation();
			}
		}
	}

	for(TIItemContainer::iterator it = inventory().m_belt.begin(); 
		inventory().m_belt.end() != it; ++it) 
	{
		CArtefact*	artefact = smart_cast<CArtefact*>(*it);
		if(artefact)
		{
			conditions().ChangeBleeding		(artefact->m_fBleedingRestoreSpeed  * f_update_time);
			conditions().ChangeHealth		(artefact->m_fHealthRestoreSpeed    * f_update_time);
			conditions().ChangePower		(artefact->m_fPowerRestoreSpeed     * f_update_time);
			conditions().ChangeSatiety		(artefact->m_fSatietyRestoreSpeed   * f_update_time);
			conditions().ChangeThirst		(artefact->m_fThirstRestoreSpeed	* f_update_time);
			conditions().ChangeRadiation	(artefact->m_fRadiationRestoreSpeed * f_update_time);
			conditions().ChangeIntoxication	(artefact->m_fIntoxicationRestoreSpeed * f_update_time);
			conditions().ChangeSleepeness	(artefact->m_fSleepenessRestoreSpeed * f_update_time);
			conditions().ChangeAlcoholism	(artefact->m_fAlcoholismRestoreSpeed * f_update_time);
			conditions().ChangeNarcotism	(artefact->m_fNarcotismRestoreSpeed * f_update_time);
			conditions().ChangePsyHealth	(artefact->m_fPsyHealthRestoreSpeed * f_update_time);

			if (GameConstants::GetArtefactsDegradation())
				artefact->UpdateDegradation();
		}
	}
}

void CActor::UpdateArtefactsInRuck()
{
	static float update_time = 0;

	float f_update_time = 0;

	if (update_time < ARTEFACTS_UPDATE_TIME)
	{
		update_time += conditions().fdelta_time();
		return;
	}
	else
	{
		f_update_time = update_time;
		update_time = 0.0f;
	}

	TIItemContainer::iterator it = inventory().m_ruck.begin();
	TIItemContainer::iterator ite = inventory().m_ruck.end();

	for(TIItemContainer::iterator it = inventory().m_belt.begin(); 
		inventory().m_belt.end() != it; ++it) 
	{
		CArtefact*	artefact = smart_cast<CArtefact*>(*it);
		if(artefact)
		{
			conditions().ChangeBleeding		(artefact->m_fBleedingRestoreSpeed  * f_update_time);
			conditions().ChangeHealth		(artefact->m_fHealthRestoreSpeed    * f_update_time);
			conditions().ChangePower		(artefact->m_fPowerRestoreSpeed     * f_update_time);
			conditions().ChangeSatiety		(artefact->m_fSatietyRestoreSpeed   * f_update_time);
			conditions().ChangeThirst		(artefact->m_fThirstRestoreSpeed	* f_update_time);
			conditions().ChangeRadiation	(artefact->m_fRadiationRestoreSpeed * f_update_time);
			conditions().ChangeIntoxication	(artefact->m_fIntoxicationRestoreSpeed * f_update_time);
			conditions().ChangeSleepeness	(artefact->m_fSleepenessRestoreSpeed * f_update_time);
			conditions().ChangeAlcoholism	(artefact->m_fAlcoholismRestoreSpeed * f_update_time);
			conditions().ChangeNarcotism	(artefact->m_fNarcotismRestoreSpeed * f_update_time);
			conditions().ChangePsyHealth	(artefact->m_fPsyHealthRestoreSpeed * f_update_time);

			if (GameConstants::GetArtefactsDegradation())
				artefact->UpdateDegradation();
		}
	}
}

void CActor::UpdateInventoryItems()
{
	TIItemContainer::iterator it = inventory().m_ruck.begin();
	TIItemContainer::iterator ite = inventory().m_ruck.end();

	for (; it != ite; ++it)
	{
		CEatableItem* current_eatable = smart_cast<CEatableItem*>(*it);
		if (current_eatable)
		{
			current_eatable->UpdateInRuck(this);
		}

		if (GameConstants::GetLimitedInventory())
		{
			CInventoryItem* item_to_drop = smart_cast<CInventoryItem*>(*it);

			if (item_to_drop && item_to_drop->m_pInventory && !item_to_drop->IsQuestItem() && m_iInventoryFullness > MaxCarryInvCapacity())
			{
				if (m_iInventoryFullnessCtrl > MaxCarryInvCapacity())
				{
					NET_Packet P;
					CGameObject::u_EventGen(P, GE_OWNERSHIP_REJECT, ID());
					P.w_u16(item_to_drop->object().ID());
					CGameObject::u_EventSend(P);

					m_iInventoryFullnessCtrl -= item_to_drop->GetOccupiedInvSpace();
				}

				SDrawStaticStruct* _s = HUD().GetUI()->UIGame()->AddCustomStatic("backpack_full", true);
				_s->wnd()->SetText(CStringTable().translate("st_backpack_full").c_str());
			}
		}
	}
}

void CActor::UpdateSkills()
{
	static float update_time = 0;
	float f_update_time = 0;

	f_update_time += conditions().fdelta_time();

	if (ActorSkills)
	{
		float BleedingRestoreSkill = conditions().m_fV_BleedingSkill * ActorSkills->survivalSkillLevel;
		float HealthRestoreSkill = conditions().m_fV_HealthSkill * ActorSkills->survivalSkillLevel;
		float PowerRestoreSkill = conditions().m_fV_PowerSkill * ActorSkills->survivalSkillLevel;
		float SatietyRestoreSkill = conditions().m_fV_SatietySkill * ActorSkills->survivalSkillLevel;
		float ThirstRestoreSkill = conditions().m_fV_ThirstSkill * ActorSkills->survivalSkillLevel;
		float IntoxicationRestoreSkill = conditions().m_fV_IntoxicationSkill * ActorSkills->survivalSkillLevel;
		float SleepenessRestoreSkill = conditions().m_fV_SleepenessSkill * ActorSkills->survivalSkillLevel;
		float RadiationRestoreSkill = conditions().m_fV_RadiationSkill * ActorSkills->survivalSkillLevel;

		conditions().ChangeBleeding(BleedingRestoreSkill * f_update_time);
		conditions().ChangeHealth(HealthRestoreSkill * f_update_time);
		conditions().ChangePower(PowerRestoreSkill * f_update_time);
		conditions().ChangeSatiety(SatietyRestoreSkill * f_update_time);
		conditions().ChangeThirst(ThirstRestoreSkill * f_update_time);
		conditions().ChangeIntoxication(IntoxicationRestoreSkill * f_update_time);
		conditions().ChangeSleepeness(SleepenessRestoreSkill * f_update_time);
		conditions().ChangeRadiation(RadiationRestoreSkill * f_update_time);
	}
}

float	CActor::HitArtefactsOnBelt(float hit_power, ALife::EHitType hit_type)
{
	TIItemContainer::iterator it  = inventory().m_belt.begin(); 
	TIItemContainer::iterator ite = inventory().m_belt.end();
	TIItemContainer::iterator itR = inventory().m_ruck.begin();
	TIItemContainer::iterator iteR = inventory().m_ruck.end();

	if (xr_strcmp("from_belt", GameConstants::GetAfInfluenceMode()) == 0 || xr_strcmp("from_ruck_only_rad", GameConstants::GetAfInfluenceMode()) == 0)
	{
		for (; it != ite; ++it)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*it);
			if (artefact)
			{
				hit_power -= artefact->m_ArtefactHitImmunities.AffectHit(1.0f, hit_type);
			}
		}
	}
	else if (xr_strcmp("from_ruck", GameConstants::GetAfInfluenceMode()) == 0)
	{
		for (; itR != iteR; ++itR)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itR);
			if (artefact)
			{
				hit_power -= artefact->m_ArtefactHitImmunities.AffectHit(1.0f, hit_type);
			}
		}
	}

	clamp(hit_power, 0.0f, flt_max);

	return hit_power;
}

float CActor::GetProtection_ArtefactsOnBelt( ALife::EHitType hit_type )
{
	float sum = 0.0f;
	TIItemContainer::iterator it  = inventory().m_belt.begin(); 
	TIItemContainer::iterator ite = inventory().m_belt.end();
	TIItemContainer::iterator itR = inventory().m_ruck.begin();
	TIItemContainer::iterator iteR = inventory().m_ruck.end();

	if (xr_strcmp("from_belt", GameConstants::GetAfInfluenceMode()) == 0 || xr_strcmp("from_ruck_only_rad", GameConstants::GetAfInfluenceMode()) == 0)
	{
		for (; it != ite; ++it)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*it);
			if (artefact)
			{
				sum += artefact->m_ArtefactHitImmunities.AffectHit(1.0f, hit_type);
			}
		}
	}
	else if (xr_strcmp("from_ruck", GameConstants::GetAfInfluenceMode()) == 0)
	{
		for (; itR != iteR; ++itR)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itR);
			if (artefact)
			{
				sum += artefact->m_ArtefactHitImmunities.AffectHit(1.0f, hit_type);
			}
		}
	}

	return sum;
}

void	CActor::SetZoomRndSeed		(s32 Seed)
{
	if (0 != Seed) m_ZoomRndSeed = Seed;
	else m_ZoomRndSeed = s32(Level().timeServer_Async());
};

void	CActor::SetShotRndSeed		(s32 Seed)
{
	if (0 != Seed) m_ShotRndSeed = Seed;
	else m_ShotRndSeed = s32(Level().timeServer_Async());
};

void CActor::spawn_supplies			()
{
	inherited::spawn_supplies		();
	CInventoryOwner::spawn_supplies	();
}


void CActor::AnimTorsoPlayCallBack(CBlend* B)
{
	CActor* actor		= (CActor*)B->CallbackParam;
	actor->m_bAnimTorsoPlayed = FALSE;
}

void CActor::SetActorVisibility(u16 who, float value)
{
	CUIMotionIcon		&motion_icon	= HUD().GetUI()->UIMainIngameWnd->MotionIcon();
	motion_icon.SetActorVisibility		(who, value);
}

void CActor::UpdateMotionIcon(u32 mstate_rl)
{
	CUIMotionIcon		&motion_icon=HUD().GetUI()->UIMainIngameWnd->MotionIcon();
	if(mstate_rl&mcClimb)
	{
		motion_icon.ShowState(CUIMotionIcon::stClimb);
	}
	else
	{
		if(mstate_rl&mcCrouch)
		{
			if (!isActorAccelerated(mstate_rl, IsZoomAimingMode()))
				motion_icon.ShowState(CUIMotionIcon::stCreep);
			else
				motion_icon.ShowState(CUIMotionIcon::stCrouch);
		}
		else
		if(mstate_rl&mcSprint)
				motion_icon.ShowState(CUIMotionIcon::stSprint);
		else
		if(mstate_rl&mcAnyMove && isActorAccelerated(mstate_rl, IsZoomAimingMode()))
			motion_icon.ShowState(CUIMotionIcon::stRun);
		else
			motion_icon.ShowState(CUIMotionIcon::stNormal);
	}

/*
						stNormal, --
						stCrouch, --
						stCreep,  --
						stClimb,  --
						stRun,    --
						stSprint, --
*/
}


CPHDestroyable*	CActor::ph_destroyable	()
{
	return smart_cast<CPHDestroyable*>(character_physics_support());
}

CEntityConditionSimple *CActor::create_entity_condition	(CEntityConditionSimple* ec)
{
	if(!ec)
		m_entity_condition		= xr_new<CActorCondition>(this);
	else
		m_entity_condition		= smart_cast<CActorCondition*>(ec);
	
	return		(inherited::create_entity_condition(m_entity_condition));
}

DLL_Pure *CActor::_construct			()
{
	m_pPhysics_support				=	xr_new<CCharacterPhysicsSupport>(CCharacterPhysicsSupport::etActor,this);
	CEntityAlive::_construct		();
	CInventoryOwner::_construct		();
	CStepManager::_construct		();
	
	return							(this);
}

bool CActor::use_center_to_aim			() const
{
	return							(!!(mstate_real&mcCrouch));
}

bool CActor::can_attach			(const CInventoryItem *inventory_item) const
{
	const CAttachableItem	*item = smart_cast<const CAttachableItem*>(inventory_item);
	if (!item || /*!item->enabled() ||*/ !item->can_be_attached())
		return			(false);

	//можно ли присоединять объекты такого типа
	if( m_attach_item_sections.end() == std::find(m_attach_item_sections.begin(),m_attach_item_sections.end(),inventory_item->object().cNameSect()) )
		return false;

	//если уже есть присоединненый объет такого типа 
	if(attached(inventory_item->object().cNameSect()))
		return false;

	return true;
}

void CActor::OnDifficultyChanged	()
{
	// immunities
	VERIFY(g_SingleGameDifficulty>=egdNovice && g_SingleGameDifficulty<=egdMaster); 
	LPCSTR diff_name				= get_token_name(difficulty_type_token, g_SingleGameDifficulty);
	string128						tmp;
	strconcat						(sizeof(tmp),tmp,"actor_immunities_",diff_name);
	conditions().LoadImmunities		(tmp,pSettings);
	// hit probability
	strconcat						(sizeof(tmp),tmp,"hit_probability_",diff_name);
	m_hit_probability				= pSettings->r_float(*cNameSect(),tmp);
	// two hits death parameters
	strconcat						(sizeof(tmp),tmp,"actor_thd_",diff_name);
	conditions().LoadTwoHitsDeathParams(tmp);
}

CVisualMemoryManager	*CActor::visual_memory	() const
{
	return							(&memory().visual());
}

float		CActor::GetMass				()
{
	return g_Alive()?character_physics_support()->movement()->GetMass():m_pPhysicsShell?m_pPhysicsShell->getMass():0; 
}

bool CActor::is_on_ground()
{
	return (character_physics_support()->movement()->Environment() != CPHMovementControl::peInAir);
}

CCustomOutfit* CActor::GetOutfit() const
{
	PIItem _of	= inventory().m_slots[OUTFIT_SLOT].m_pIItem;
	return _of?smart_cast<CCustomOutfit*>(_of):NULL;
}

CCustomOutfit* CActor::GetPants() const
{
	PIItem _of = inventory().m_slots[PANTS_SLOT].m_pIItem;
	return _of ? smart_cast<CCustomOutfit*>(_of) : NULL;
}

bool CActor::is_ai_obstacle				() const
{
	return							(false);//true);
}

float CActor::GetRestoreSpeed( ALife::EConditionRestoreType const& type )
{
	float res = 0.0f;
	switch ( type )
	{
	case ALife::eHealthRestoreSpeed:
	{
		res = conditions().change_v().m_fV_HealthRestore;
		res += conditions().V_SatietyHealth() * ( (conditions().GetSatiety() > 0.0f) ? 1.0f : -1.0f );

		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fHealthRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fHealthRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fHealthRestoreSpeed;
		}
		break;
	}
	case ALife::eRadiationRestoreSpeed:
	{	
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fRadiationRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fRadiationRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fRadiationRestoreSpeed;
		}
		break;
	}
	case ALife::eSatietyRestoreSpeed:
	{
		res = conditions().V_Satiety();

		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fSatietyRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fSatietyRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fSatietyRestoreSpeed;
		}
		break;
	}
	case ALife::ePowerRestoreSpeed:
	{
		res = conditions().GetSatietyPower();

		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fPowerRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fPowerRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fPowerRestoreSpeed;
		}
		break;
	}
	case ALife::eBleedingRestoreSpeed:
	{
		res = conditions().change_v().m_fV_WoundIncarnation;
	
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fBleedingRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fBleedingRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fBleedingRestoreSpeed;
		}
		break;
	}
	case ALife::eThirstRestoreSpeed:
	{
		res = conditions().V_Thirst();

		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for (; itb != ite; ++itb)
		{
			CArtefact*	artefact = smart_cast<CArtefact*>(*itb);
			if (artefact)
			{
				res += artefact->m_fThirstRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if (outfit)
		{
			res += outfit->m_fThirstRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fThirstRestoreSpeed;
		}
		break;
	}
	case ALife::eIntoxicationRestoreSpeed:
	{
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for (; itb != ite; ++itb)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itb);
			if (artefact)
			{
				res += artefact->m_fIntoxicationRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if (outfit)
		{
			res += outfit->m_fIntoxicationRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fIntoxicationRestoreSpeed;
		}
		break;
	}
	case ALife::eSleepenessRestoreSpeed:
	{
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for (; itb != ite; ++itb)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itb);
			if (artefact)
			{
				res += artefact->m_fSleepenessRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if (outfit)
		{
			res += outfit->m_fSleepenessRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fSleepenessRestoreSpeed;
		}
		break;
	}
	case ALife::eAlcoholismRestoreSpeed:
	{
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for (; itb != ite; ++itb)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itb);
			if (artefact)
			{
				res += artefact->m_fAlcoholismRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if (outfit)
		{
			res += outfit->m_fAlcoholismRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fAlcoholismRestoreSpeed;
		}
		break;
	}
	case ALife::eNarcotismRestoreSpeed:
	{
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for (; itb != ite; ++itb)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itb);
			if (artefact)
			{
				res += artefact->m_fNarcotismRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if (outfit)
		{
			res += outfit->m_fNarcotismRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fNarcotismRestoreSpeed;
		}
		break;
	}
	case ALife::ePsyHealthRestoreSpeed:
	{
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for (; itb != ite; ++itb)
		{
			CArtefact* artefact = smart_cast<CArtefact*>(*itb);
			if (artefact)
			{
				res += artefact->m_fPsyHealthRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if (outfit)
		{
			res += outfit->m_fPsyHealthRestoreSpeed;
		}

		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
		if (backpack)
		{
			res += backpack->m_fPsyHealthRestoreSpeed;
		}
		break;
	}
	}//switch

	return res;
}


void CActor::On_SetEntity()
{
	CCustomOutfit* pOutfit = GetOutfit();
	if (!pOutfit)
		g_player_hud->load_default();
	else
		pOutfit->ApplySkinModel(this, true, true);
}

void CActor::NVGAnimCheckDetector()
{
	if (isHidingInProgress.load())
		return;

	CCustomDetector* pDet = smart_cast<CCustomDetector*>(inventory().ItemFromSlot(DETECTOR_SLOT));

	if (!pDet || pDet->IsHidden())
	{
		StartNVGAnimation();
		return;
	}

	isHidingInProgress.store(true);

	std::thread hidingThread([&, pDet]
		{
			while (pDet && !pDet->IsHidden())
				pDet->HideDetector(true);

			isHidingInProgress.store(false);
			CheckNVGAnimNeeded.store(true);
		});

	hidingThread.detach();
}

void CActor::StartNVGAnimation()
{
	CWeapon* Wpn = smart_cast<CWeapon*>(inventory().ActiveItem());
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(OUTFIT_SLOT));

	if (Wpn && Wpn->IsZoomed())
		return;

	LPCSTR anim_sect = READ_IF_EXISTS(pAdvancedSettings, r_string, "actions_animations", "switch_nightvision_section", nullptr);

	if (!anim_sect)
	{
		SwitchNightVision(!m_bNightVisionOn);
		return;
	}

	if (!(pOutfit && pOutfit->m_NightVisionSect.size()))
		return;

	if (Wpn && !(Wpn->GetState() == CWeapon::eIdle))
		return;

	m_bNVGActivated = true;

	int anim_timer = READ_IF_EXISTS(pSettings, r_u32, anim_sect, "anim_timing", 0);

	g_block_all_except_movement = true;
	g_actor_allow_ladder = false;

	LPCSTR use_cam_effector = READ_IF_EXISTS(pSettings, r_string, anim_sect, !Wpn ? "anim_camera_effector" : "anim_camera_effector_weapon", nullptr);
	float effector_intensity = READ_IF_EXISTS(pSettings, r_float, anim_sect, "cam_effector_intensity", 1.0f);
	float anim_speed = READ_IF_EXISTS(pSettings, r_float, anim_sect, "anim_speed", 1.0f);
	
	if (pSettings->line_exist(anim_sect, "anm_use"))
	{
		g_player_hud->script_anim_play(!inventory().GetActiveSlot() ? 2 : 1, anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", true, anim_speed);

		if (use_cam_effector)
			g_player_hud->PlayBlendAnm(use_cam_effector, 0, anim_speed, effector_intensity, false);

		m_iNVGAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", anim_speed);
	}

	if (pSettings->line_exist(anim_sect, "snd_using"))
	{
		if (m_action_anim_sound._feedback())
			m_action_anim_sound.stop();

		shared_str snd_name = pSettings->r_string(anim_sect, "snd_using");
		m_action_anim_sound.create(snd_name.c_str(), st_Effect, sg_SourceType);
		m_action_anim_sound.play(NULL, sm_2D);
	}

	m_iActionTiming = Device.dwTimeGlobal + anim_timer;

	m_bNVGSwitched = false;
	m_bActionAnimInProcess = true;
}

void CActor::UpdateNVGUseAnim()
{
	if ((m_iActionTiming <= Device.dwTimeGlobal && !m_bNVGSwitched) && g_Alive())
	{
		m_iActionTiming = Device.dwTimeGlobal;
		SwitchNightVision(!m_bNightVisionOn);
		m_bNVGSwitched = true;
	}

	if (m_bNVGActivated)
	{
		if ((m_iNVGAnimLength <= Device.dwTimeGlobal) || !g_Alive())
		{
			m_iNVGAnimLength = Device.dwTimeGlobal;
			m_iActionTiming = Device.dwTimeGlobal;
			m_action_anim_sound.stop();
			g_block_all_except_movement = false;
			g_actor_allow_ladder = true;
			m_bActionAnimInProcess = false;
			m_bNVGActivated = false;
		}
	}
}

void CActor::CleanMaskAnimCheckDetector()
{
	if (isHidingInProgress.load())
		return;

	CCustomDetector* pDet = smart_cast<CCustomDetector*>(inventory().ItemFromSlot(DETECTOR_SLOT));

	if (!pDet || pDet->IsHidden())
	{
		CleanMask();
		return;
	}

	isHidingInProgress.store(true);

	std::thread hidingThread([&, pDet]
		{
			while (pDet && !pDet->IsHidden())
				pDet->HideDetector(true);

			isHidingInProgress.store(false);
			CleanMaskAnimNeeded.store(true);
		});

	hidingThread.detach();
}

void CActor::CleanMask()
{
	LPCSTR anim_sect = READ_IF_EXISTS(pAdvancedSettings, r_string, "actions_animations", "clean_mask_section", nullptr);

	if (!anim_sect)
		return;

	CWeapon* Wpn = smart_cast<CWeapon*>(inventory().ActiveItem());
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(OUTFIT_SLOT));

	if (!(pOutfit && pOutfit->m_b_HasGlass))
		return;

	if (Wpn && !(Wpn->GetState() == CWeapon::eIdle))
		return;

	m_bMaskAnimActivated = true;

	int anim_timer = READ_IF_EXISTS(pSettings, r_u32, anim_sect, "anim_timing", 0);

	g_block_all_except_movement = true;
	g_actor_allow_ladder = false;

	LPCSTR use_cam_effector = READ_IF_EXISTS(pSettings, r_string, anim_sect, !Wpn ? "anim_camera_effector" : "anim_camera_effector_weapon", nullptr);
	float effector_intensity = READ_IF_EXISTS(pSettings, r_float, anim_sect, "cam_effector_intensity", 1.0f);
	float anim_speed = READ_IF_EXISTS(pSettings, r_float, anim_sect, "anim_speed", 1.0f);

	if (pSettings->line_exist(anim_sect, "anm_use"))
	{
		g_player_hud->script_anim_play(!inventory().GetActiveSlot() ? 2 : 1, anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", true, anim_speed);

		if (use_cam_effector)
			g_player_hud->PlayBlendAnm(use_cam_effector, 0, anim_speed, effector_intensity, false);

		m_iMaskAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", anim_speed);
	}

	if (pSettings->line_exist(anim_sect, "snd_using"))
	{
		if (m_action_anim_sound._feedback())
			m_action_anim_sound.stop();

		shared_str snd_name = pSettings->r_string(anim_sect, "snd_using");
		m_action_anim_sound.create(snd_name.c_str(), st_Effect, sg_SourceType);
		m_action_anim_sound.play(NULL, sm_2D);
	}

	m_iActionTiming = Device.dwTimeGlobal + anim_timer;

	m_bMaskClear = false;
	m_bActionAnimInProcess = true;
}

void CActor::UpdateMaskUseAnim()
{
	if ((m_iActionTiming <= Device.dwTimeGlobal && !m_bMaskClear) && g_Alive())
	{
		m_iActionTiming = Device.dwTimeGlobal;
		m_bMaskClear = true;
	}

	if (m_bMaskAnimActivated)
	{
		if ((m_iMaskAnimLength <= Device.dwTimeGlobal) || !g_Alive())
		{
			m_iMaskAnimLength = Device.dwTimeGlobal;
			m_iActionTiming = Device.dwTimeGlobal;
			m_action_anim_sound.stop();
			g_block_all_except_movement = false;
			g_actor_allow_ladder = true;
			m_bActionAnimInProcess = false;
			m_bMaskAnimActivated = false;
			m_bMaskClear = false;
		}
	}
}

void CActor::SwitchNightVision(bool vision_on, bool use_sounds, bool send_event)
{
	m_bNightVisionOn = vision_on;

	if (!m_night_vision)
		m_night_vision = xr_new<CNightVisionEffector>(cNameSect());

	bool bIsActiveNow = m_night_vision->IsActive();

	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(OUTFIT_SLOT));
	if (pOutfit && pOutfit->m_NightVisionSect.size())
	{
		if (m_bNightVisionAllow)
		{
			if (m_bNightVisionOn && !bIsActiveNow)
			{
				m_night_vision->Start(pOutfit->m_NightVisionSect, this, use_sounds);
			}
		}
		else
		{
			m_night_vision->OnDisabled(this, use_sounds);
			m_bNightVisionOn = false;
		}
	}

	if ((!m_bNightVisionOn && bIsActiveNow) || (!m_bNightVisionOn && ps_r__ShaderNVG == 1))
	{
		m_night_vision->Stop(100000.0f, use_sounds);
	}

	//Alun: Update flags and send message they were changed
	if (send_event)
	{
		m_trader_flags.set(CSE_ALifeTraderAbstract::eTraderFlagNightVisionActive, m_bNightVisionOn);
		CGameObject *object = smart_cast<CGameObject*>(this);
		NET_Packet packet;
		object->u_EventGen(packet, GE_TRADER_FLAGS, object->ID());
		packet.w_u32(m_trader_flags.get());
		object->u_EventSend(packet);
		//Msg("GE_TRADER_FLAGS event sent %d", m_trader_flags.get());
	}
}

void CActor::block_action(EGameActions cmd)
{
	if (m_blocked_actions.find(cmd) == m_blocked_actions.end())
	{
		m_blocked_actions[cmd] = true;
	}
}

void CActor::unblock_action(EGameActions cmd)
{
	auto iter = m_blocked_actions.find(cmd);
	if (iter != m_blocked_actions.end())
	{
		m_blocked_actions.erase(iter);
	}
}

bool CActor::use_HolderEx(CHolderCustom* object, bool bForce)
{
	if (m_holder)
	{
		/*
		CCar* car = smart_cast<CCar*>(m_holder);
		if (car)
		{
			detach_Vehicle();
			return true;
		}
		*/
		if (!m_holder->ExitLocked() || bForce)
		{
			if (!object || (m_holder == object)) {

				CGameObject* go = smart_cast<CGameObject*>(m_holder);
				CPhysicsShellHolder* pholder = smart_cast<CPhysicsShellHolder*>(go);
				if (pholder)
				{
					pholder->PPhysicsShell()->SplitterHolderDeactivate();
					if (!character_physics_support()->movement()->ActivateBoxDynamic(0))
					{
						pholder->PPhysicsShell()->SplitterHolderActivate();
						return true;
					}
					pholder->PPhysicsShell()->SplitterHolderActivate();
				}

				SetWeaponHideState(INV_STATE_BLOCK_ALL, false);

				if (go)
					this->callback(GameObject::eDetachVehicle)(go->lua_game_object());

				m_holder->detach_Actor();

				character_physics_support()->movement()->CreateCharacter();
				character_physics_support()->movement()->SetPosition(m_holder->ExitPosition());
				character_physics_support()->movement()->SetVelocity(m_holder->ExitVelocity());

				r_model_yaw = -m_holder->Camera()->yaw;
				r_torso.yaw = r_model_yaw;
				r_model_yaw_dest = r_model_yaw;

				cam_Active()->Direction().set(m_holder->Camera()->Direction());

				SetCallbacks();

				m_holder = NULL;
				m_holderID = u16(-1);

				IKinematicsAnimated* V = smart_cast<IKinematicsAnimated*>(Visual()); R_ASSERT(V);
				V->PlayCycle(m_anims->m_normal.legs_idle);
				V->PlayCycle(m_anims->m_normal.m_torso_idle);

				IKinematics* pK = smart_cast<IKinematics*>(Visual());
				u16 head_bone = pK->LL_BoneID("bip01_head");
				pK->LL_GetBoneInstance(u16(head_bone)).set_callback(bctPhysics, HeadCallback, this);
			}
		}
		return true;
	}
	else
	{
		/*
		CCar* car = smart_cast<CCar*>(object);
		if (car)
		{
			attach_Vehicle(object);
			return true;
		}
		*/
		if (object && (!object->EnterLocked() || bForce))
		{
			Fvector center;	Center(center);
			if ((bForce || object->Use(Device.vCameraPosition, Device.vCameraDirection, center)) && object->attach_Actor(this))
			{
				inventory().SetActiveSlot(NO_ACTIVE_SLOT);
				SetWeaponHideState(INV_STATE_BLOCK_ALL, true);

				// destroy actor character
				character_physics_support()->movement()->DestroyCharacter();

				m_holder = object;
				CGameObject* oHolder = smart_cast<CGameObject*>(object);
				m_holderID = oHolder->ID();

				if (pCamBobbing) {
					Cameras().RemoveCamEffector(eCEBobbing);
					pCamBobbing = NULL;
				}

				if (actor_camera_shell)
					destroy_physics_shell(actor_camera_shell);

				IKinematics* pK = smart_cast<IKinematics*>(Visual());
				u16 head_bone = pK->LL_BoneID("bip01_head");
				pK->LL_GetBoneInstance(u16(head_bone)).set_callback(bctPhysics, VehicleHeadCallback, this);

				CCar* car = smart_cast<CCar*>(object);
				if (car)
				{
					u16 anim_type = car->DriverAnimationType();
					SVehicleAnimCollection& anims = m_vehicle_anims->m_vehicles_type_collections[anim_type];
					IKinematicsAnimated* V = smart_cast<IKinematicsAnimated*>(Visual()); R_ASSERT(V);
					V->PlayCycle(anims.idles[0], FALSE);
					CStepManager::on_animation_start(MotionID(), 0);
				}

				CGameObject* go = smart_cast<CGameObject*>(object);
				if (go)
					this->callback(GameObject::eAttachVehicle)(go->lua_game_object());

				return true;
			}
		}
	}
	return false;
}

bool CActor::HasItemsForRepair(xr_vector<std::pair<shared_str, int>> item)
{
	for (int i{}; i < item.size(); ++i)
	{
		CInventoryOwner* l_tpInventoryOwner = smart_cast<CInventoryOwner*>(this);

		if (!l_tpInventoryOwner)
			return false;

		CInventoryItem* l_tpInventoryItem = l_tpInventoryOwner->inventory().GetItemFromInventory(item[i].first.c_str());

		if (!l_tpInventoryItem)
			return false;

		int need_count = item[i].second;
		int has_count = 0;

		auto calc = [&](LPCSTR section)
		{
			if (section == item[i].first.c_str())
				has_count++;
		};

		TIItemContainer::iterator I = l_tpInventoryOwner->inventory().m_all.begin();
		TIItemContainer::iterator E = l_tpInventoryOwner->inventory().m_all.end();

		for (; I != E; ++I)
			calc((*I)->object().cNameSect().c_str());

		if (has_count < need_count)
			return false;
	}

	return true;
}

void CActor::RemoveItemsForRepair(xr_vector<std::pair<shared_str, int>> item)
{
	for (int i{}; i < item.size(); ++i)
	{
		CInventoryOwner* l_tpInventoryOwner = smart_cast<CInventoryOwner*>(this);

		if (!l_tpInventoryOwner)
			return;

		CInventoryItem* l_tpInventoryItem = l_tpInventoryOwner->inventory().GetItemFromInventory(item[i].first.c_str());

		if (!l_tpInventoryItem)
			return;

		int need_count = item[i].second;
		int has_count = 0;

		auto calc = [&](LPCSTR section)
		{
			if (section == item[i].first.c_str())
			{
				has_count++;

				return true;
			}
			return false;
		};

		TIItemContainer::iterator I = l_tpInventoryOwner->inventory().m_all.begin();
		TIItemContainer::iterator E = l_tpInventoryOwner->inventory().m_all.end();

		for (; I != E; ++I)
		{
			bool is_item_to_remove = calc((*I)->object().cNameSect().c_str());

			if (has_count && has_count <= need_count && is_item_to_remove)
				(*I)->object().DestroyObject();
		}
	}
}

void CActor::ChangeInventoryFullness(int val)
{
	if (!this)
		return;

	m_iInventoryFullness += val;

	if (m_iInventoryFullness < 0)
		m_iInventoryFullness = 0;

	if (val > 0)
		m_iInventoryFullnessCtrl = m_iInventoryFullness;
}

//Максимальная вместительность инвентаря
int CActor::MaxCarryInvCapacity() const
{
	int res = m_iInventoryCapacity;

	CCustomOutfit* outfit = GetOutfit();
	if (outfit)
		res += outfit->GetInventoryCapacity();

	CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(inventory().ItemFromSlot(BACKPACK_SLOT));
	if (backpack)
		res += backpack->GetInventoryCapacity();

	CCustomOutfit* pants = smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(PANTS_SLOT));
	if (pants)
		res += pants->GetInventoryCapacity();

	return res;
}