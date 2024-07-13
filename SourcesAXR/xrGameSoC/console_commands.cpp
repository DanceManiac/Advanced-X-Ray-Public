#include "pch_script.h"
#include "../xrEngine/xr_ioconsole.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/xr_ioc_cmd.h"
#include "../xrEngine/customhud.h"
#include "../xrEngine/fdemorecord.h"
#include "../xrEngine/fdemoplay.h"
#include "xrMessages.h"
#include "xrserver.h"
#include "level.h"
#include "script_debugger.h"
#include "ai_debug.h"
#include "alife_simulator.h"
#include "game_cl_base.h"
#include "game_cl_single.h"
#include "game_sv_single.h"
#include "hit.h"
#include "PHDestroyable.h"
#include "actor.h"
#include "Actor_Flags.h"
#include "customzone.h"
#include "script_engine.h"
#include "script_engine_space.h"
#include "script_process.h"
#include "xrServer_Objects.h"
#include "ui/UIMainIngameWnd.h"
#include "PhysicsGamePars.h"
#include "phworld.h"
#include "string_table.h"
#include "autosave_manager.h"
#include "ai_space.h"
#include "ai/monsters/BaseMonster/base_monster.h"
#include "date_time.h"
#include "mt_config.h"
#include "ui/UIOptConCom.h"
#include "zone_effector.h"
#include "GameTask.h"
#include "MainMenu.h"
#include "saved_game_wrapper.h"
#include "level_graph.h"
#include "doug_lea_memory_allocator.h"
#include "cameralook.h"

#include "GameSpy/GameSpy_Full.h"
#include "GameSpy/GameSpy_Patching.h"
#include "../Include/xrRender/Kinematics.h"

#include "ai_object_location.h"
#include "ui/UIDebugFonts.h" 

#ifdef DEBUG
#	include "PHDebug.h"
#	include "game_graph.h"
#endif // DEBUG

#include "../../xrNetServer/NET_AuthCheck.h"

#include "HUDManager.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "InfoPortion.h"
#include "GametaskManager.h"
#include "AdvancedXrayGameConstants.h"

// Hud Type
xr_token			qhud_type_token[] = {
	{ "hud_1",					1},
	{ "hud_2",					2},
	{ 0,						0}
};

// M.F.S. Crosshair Type
extern u32	crosshair_type;
xr_token							crosshair_type_token[] = {
	{ "default_crosshair",			1												},
	{ "builds_crosshair",			2												},
	{ "point_crosshair",			3												},
	{ 0,							0												}
};

extern u32	death_camera_mode;
xr_token							death_camera_mode_token[] = {
	{ "freelook",					1												},
	{ "fixedlook",					2												},
	{ "firsteye",					3												},
	{ 0,							0												}
};

string_path		g_last_saved_game;
int				quick_save_counter = 0;
extern u32		last_quick;

extern void show_smart_cast_stats		();
extern void clear_smart_cast_stats		();
extern void release_smart_cast_stats	();

extern	u64		g_qwStartGameTime;
extern	u64		g_qwEStartGameTime;

ENGINE_API
extern  float   psHUD_FOV_def;
extern	float	psSqueezeVelocity;
extern	int		psLUA_GCSTEP;

extern	int		x_m_x;
extern	int		x_m_z;
extern	BOOL	net_cl_inputguaranteed	;
extern	BOOL	net_sv_control_hit		;
extern	int		g_dwInputUpdateDelta	;
#ifdef DEBUG
extern	BOOL	g_ShowAnimationInfo		;
#endif // DEBUG
extern	BOOL	g_bShowHitSectors		;
extern	BOOL	g_bDebugDumpPhysicsStep	;
extern	ESingleGameDifficulty g_SingleGameDifficulty;
extern	BOOL	g_show_wnd_rect			;
extern	BOOL	g_show_wnd_rect2			;
//-----------------------------------------------------------
extern	float	g_fTimeFactor;
		int		g_keypress_on_start = 1;
extern	int		hud_adj_mode;
extern	int		hud_adj_item_idx;
//extern	float	g_bHudAdjustDeltaPos;
//extern	float	g_bHudAdjustDeltaRot;

extern	BOOL	g_advanced_crosshair;

extern bool		g_saves_locked;
extern BOOL		g_dbgShowMaterialInfo;
//Custom commands for scripts

const int I_SCRIPT_CMDS_COUNT = GameConstants::GetIntScriptCMDCount();
const BOOL B_SCRIPT_CMDS_COUNT = GameConstants::GetBOOLScriptCMDCount();
xr_vector<int> i_script_cmd(I_SCRIPT_CMDS_COUNT);
xr_vector<xr_string> i_script_cmd_name(I_SCRIPT_CMDS_COUNT);
xr_vector<BOOL> b_script_cmd(B_SCRIPT_CMDS_COUNT);
xr_vector<xr_string> b_script_cmd_name(B_SCRIPT_CMDS_COUNT);

//Custom commands for scripts end

void register_mp_console_commands();
//-----------------------------------------------------------

		BOOL	g_bCheckTime			= FALSE;
		int		net_cl_inputupdaterate	= 50;
		Flags32	g_mt_config				= {mtLevelPath | mtDetailPath | mtObjectHandler | mtSoundPlayer | mtAiVision | mtBullets | mtLUA_GC | mtLevelSounds | mtALife};
#ifdef DEBUG
		Flags32	dbg_net_Draw_Flags		= {0};
#endif

#ifdef DEBUG
		BOOL	g_bDebugNode			= FALSE;
		u32		g_dwDebugNodeSource		= 0;
		u32		g_dwDebugNodeDest		= 0;
extern	BOOL	g_bDrawBulletHit;

		float	debug_on_frame_gather_stats_frequency	= 0.f;
#endif
#ifdef DEBUG 
extern LPSTR	dbg_stalker_death_anim;
extern BOOL		b_death_anim_velocity;
#endif
int g_AI_inactive_time = 0;
Flags32 g_uCommonFlags;
enum E_COMMON_FLAGS{
	flAiUseTorchDynamicLights = 1
};

CUIOptConCom g_OptConCom;

#ifndef PURE_ALLOC
#	ifndef USE_MEMORY_MONITOR
#		define SEVERAL_ALLOCATORS
#	endif // USE_MEMORY_MONITOR
#endif // PURE_ALLOC

#ifdef SEVERAL_ALLOCATORS
	//ENGINE_API 	u32 engine_lua_memory_usage	();
	extern		u32 game_lua_memory_usage	();
#endif // SEVERAL_ALLOCATORS

class CCC_MemStats : public IConsole_Command
{
public:
	CCC_MemStats(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) {
		Memory.mem_compact		();
		u32		_crt_heap		= mem_usage_impl((HANDLE)_get_heap_handle(),0,0);
		u32		_process_heap	= mem_usage_impl(GetProcessHeap(),0,0);
#ifdef SEVERAL_ALLOCATORS
		u32		_game_lua		= game_lua_memory_usage();
		u32		_render			= ::Render->memory_usage();
#endif // SEVERAL_ALLOCATORS
		int		_eco_strings	= (int)g_pStringContainer->stat_economy			();
		int		_eco_smem		= (int)g_pSharedMemoryContainer->stat_economy	();
		u32		m_base=0,c_base=0,m_lmaps=0,c_lmaps=0;
		
		if (Device.m_pRender) Device.m_pRender->ResourcesGetMemoryUsage(m_base, c_base, m_lmaps, c_lmaps);
		
		log_vminfo	();
		
		Msg		("* [ D3D ]: textures[%d K]", (m_base+m_lmaps)/1024);

#ifndef SEVERAL_ALLOCATORS
		Msg		("* [x-ray]: crt heap[%d K], process heap[%d K]",_crt_heap/1024,_process_heap/1024);
#else // SEVERAL_ALLOCATORS
		Msg		("* [x-ray]: crt heap[%d K], process heap[%d K], game lua[%d K], render[%d K]",_crt_heap/1024,_process_heap/1024,_game_lua/1024,_render/1024);
#endif // SEVERAL_ALLOCATORS

		Msg		("* [x-ray]: economy: strings[%d K], smem[%d K]",_eco_strings/1024,_eco_smem);

#ifdef FS_DEBUG
		Msg		("* [x-ray]: file mapping: memory[%d K], count[%d]",g_file_mapped_memory/1024,g_file_mapped_count);
		dump_file_mappings	();
#endif // DEBUG
	}
};

// console commands
class CCC_GameDifficulty : public CCC_Token {
public:
	CCC_GameDifficulty(LPCSTR N) : CCC_Token(N,(u32*)&g_SingleGameDifficulty,difficulty_type_token)  {};
	virtual void Execute(LPCSTR args) {
		CCC_Token::Execute(args);
		if (g_pGameLevel && Level().game){
//#ifndef	DEBUG
			if (GameID() != GAME_SINGLE){
				Msg("For this game type difficulty level is disabled.");
				return;
			};
//#endif

			game_cl_Single* game		= smart_cast<game_cl_Single*>(Level().game); VERIFY(game);
			game->OnDifficultyChanged	();
		}
	}
	virtual void	Info	(TInfo& I)		
	{
		xr_strcpy(I,"game difficulty"); 
	}
};





#ifdef DEBUG
class CCC_ALifePath : public IConsole_Command {
public:
	CCC_ALifePath(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		if (!ai().get_level_graph())
			Msg("! there is no graph!");
		else {
			int id1=-1, id2=-1;
			sscanf(args ,"%d %d",&id1,&id2);
			if ((-1 != id1) && (-1 != id2))
				if (_max(id1,id2) > (int)ai().game_graph().header().vertex_count() - 1)
					Msg("! there are only %d vertexes!",ai().game_graph().header().vertex_count());
				else
					if (_min(id1,id2) < 0)
						Msg("! invalid vertex number (%d)!",_min(id1,id2));
					else {
//						Sleep				(1);
//						CTimer				timer;
//						timer.Start			();
//						float				fValue = ai().m_tpAStar->ffFindMinimalPath(id1,id2);
//						Msg					("* %7.2f[%d] : %11I64u cycles (%.3f microseconds)",fValue,ai().m_tpAStar->m_tpaNodes.size(),timer.GetElapsed_ticks(),timer.GetElapsed_ms()*1000.f);
					}
			else
				Msg("! not enough parameters!");
		}
	}
};
#endif // DEBUG

class CCC_ALifeTimeFactor : public IConsole_Command {
public:
	CCC_ALifeTimeFactor(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		float id1 = 0.0f;
		sscanf(args ,"%f",&id1);
		if (id1 < EPS_L)
			Msg("Invalid time factor! (%.4f)",id1);
		else {
			if (!OnServer())
				return;

			Level().Server->game->SetGameTimeFactor(id1);
		}
	}
};

class CCC_ALifeSwitchDistance : public IConsole_Command {
public:
	CCC_ALifeSwitchDistance(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		if ((GameID() == GAME_SINGLE)  &&ai().get_alife()) {
			float id1 = 0.0f;
			sscanf(args ,"%f",&id1);
			if (id1 < 2.0f)
				Msg("Invalid online distance! (%.4f)",id1);
			else {
				NET_Packet		P;
				P.w_begin		(M_SWITCH_DISTANCE);
				P.w_float		(id1);
				Level().Send	(P,net_flags(TRUE,TRUE));
			}
		}
		else
			Log("!Not a single player game!");
	}
};

class CCC_ALifeProcessTime : public IConsole_Command {
public:
	CCC_ALifeProcessTime(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		if ((GameID() == GAME_SINGLE)  &&ai().get_alife()) {
			game_sv_Single	*tpGame = smart_cast<game_sv_Single *>(Level().Server->game);
			VERIFY			(tpGame);
			int id1 = 0;
			sscanf(args ,"%d",&id1);
			if (id1 < 1)
				Msg("Invalid process time! (%d)",id1);
			else
				tpGame->alife().set_process_time(id1);
		}
		else
			Log("!Not a single player game!");
	}
};


class CCC_ALifeObjectsPerUpdate : public IConsole_Command {
public:
	CCC_ALifeObjectsPerUpdate(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		if ((GameID() == GAME_SINGLE)  &&ai().get_alife()) {
			game_sv_Single	*tpGame = smart_cast<game_sv_Single *>(Level().Server->game);
			VERIFY			(tpGame);
			int id1 = 0;
			sscanf(args ,"%d",&id1);
			tpGame->alife().objects_per_update(id1);
		}
		else
			Log("!Not a single player game!");
	}
};

class CCC_ALifeSwitchFactor : public IConsole_Command {
public:
	CCC_ALifeSwitchFactor(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		if ((GameID() == GAME_SINGLE)  &&ai().get_alife()) {
			game_sv_Single	*tpGame = smart_cast<game_sv_Single *>(Level().Server->game);
			VERIFY			(tpGame);
			float id1 = 0;
			sscanf(args ,"%f",&id1);
			clamp(id1,.1f,1.f);
			tpGame->alife().set_switch_factor(id1);
		}
		else
			Log		("!Not a single player game!");
	}
};

class CCC_TimeFactor : public IConsole_Command {
public:
					CCC_TimeFactor	(LPCSTR N) : IConsole_Command(N) {}
	virtual void	Execute			(LPCSTR args)
	{
		float				time_factor = (float)atof(args);
		clamp				(time_factor,.001f,1000.f);
		Device.time_factor	(time_factor);
		psSpeedOfSound		= time_factor;
	}
};

//-----------------------------------------------------------------------
class CCC_DemoRecord : public IConsole_Command
{
public:

	CCC_DemoRecord(LPCSTR N) : IConsole_Command(N) {};
	virtual void Execute(LPCSTR args) {
		#ifndef	DEBUG
		if (GameID() != GAME_SINGLE) 
		{
			Msg("For this game type Demo Record is disabled.");
			return;
		};
		#endif
		Console->Hide	();
		string_path		fn_; 
		strconcat		(sizeof(fn_),fn_, args, ".xrdemo");
		string_path		fn;
		FS.update_path	(fn, "$game_saves$", fn_);

		g_pGameLevel->Cameras().AddCamEffector(xr_new<CDemoRecord> (fn));
	}
};
class CCC_DemoPlay : public IConsole_Command
{
public:
	CCC_DemoPlay(LPCSTR N) : 
	  IConsole_Command(N) 
	  { bEmptyArgsHandled = TRUE; };
	  virtual void Execute(LPCSTR args) {
		#ifndef	DEBUG
		if (GameID() != GAME_SINGLE) 
		{
			Msg("For this game type Demo Play is disabled.");
			return;
		};
		#endif
		  if (0==g_pGameLevel)
		  {
			  Msg	("! There are no level(s) started");
		  } else {
			  Console->Hide			();
			  string_path			fn;
			  u32		loops	=	0;
			  LPSTR		comma	=	strchr(const_cast<LPSTR>(args),',');
			  if (comma)	{
				  loops			=	atoi	(comma+1);
				  *comma		=	0;	//. :)
			  }
			  strconcat			(sizeof(fn),fn, args, ".xrdemo");
			  FS.update_path	(fn, "$game_saves$", fn);
			  g_pGameLevel->Cameras().AddCamEffector(xr_new<CDemoPlay> (fn, 1.0f, loops));
		  }
	  }
};

bool valid_file_name(LPCSTR file_name)
{

	LPCSTR		I = file_name;
	LPCSTR		E = file_name + xr_strlen(file_name);
	for ( ; I != E; ++I) {
		if (!strchr("/\\:*?\"<>|",*I))
			continue;

		return	(false);
	};

	return		(true);
}

void get_files_list(xr_vector<shared_str>& files, LPCSTR dir, LPCSTR file_ext)
{
	VERIFY(dir && file_ext);
	files.clear_not_free();

	FS_Path* P = FS.get_path(dir);
	P->m_Flags.set(FS_Path::flNeedRescan, TRUE);
	FS.m_Flags.set(CLocatorAPI::flNeedCheck, TRUE);
	FS.rescan_pathes();

	LPCSTR fext;
	STRCONCAT(fext, "*", file_ext);

	FS_FileSet  files_set;
	FS.file_list(files_set, dir, FS_ListFiles, fext);
	u32 len_str_ext = xr_strlen(file_ext);

	FS_FileSetIt itb = files_set.begin();
	FS_FileSetIt ite = files_set.end();

	for (; itb != ite; ++itb)
	{
		LPCSTR fn_ext = (*itb).name.c_str();
		VERIFY(xr_strlen(fn_ext) > len_str_ext);
		string_path fn;
		strncpy_s(fn, sizeof(fn), fn_ext, xr_strlen(fn_ext) - len_str_ext);
		files.push_back(fn);
	}
	FS.m_Flags.set(CLocatorAPI::flNeedCheck, FALSE);
}

#include "UIGameCustom.h"
#include "HUDManager.h"
class CCC_ALifeSave : public IConsole_Command {
public:
	CCC_ALifeSave(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args) {
		
#if 0
		if (!Level().autosave_manager().ready_for_autosave()) {
			Msg		("! Cannot save the game right now!");
			return;
		}
#endif
		if(!IsGameTypeSingle()){
			Msg("for single-mode only");
			return;
		}
		if(!g_actor || !Actor()->g_Alive())
		{
			Msg("cannot make saved game because actor is dead :(");
			return;
		}

		if (g_saves_locked)
		{
#ifdef DEBUG
			Msg("Can`t make saved game: locked by Lua.");
#endif
			SDrawStaticStruct* _s = HUD().GetUI()->UIGame()->AddCustomStatic("game_save_blocked_icon", true);
			SDrawStaticStruct* _s2 = HUD().GetUI()->UIGame()->AddCustomStatic("game_saved", true);
			_s2->wnd()->SetText(*CStringTable().translate("st_saves_locked"));
			return;
		}

		string_path				S,S1;
		S[0]					= 0;
//.		sscanf					(args ,"%s",S);
		xr_strcpy					(S,args);
		
#ifdef DEBUG
		CTimer					timer;
		timer.Start				();
#endif
		if (!xr_strlen(S))
		{
			if (last_quick < 1 && quick_save_counter == 0)
				strconcat(sizeof(S), S, Core.UserName, "_", "quicksave");
			else
				xr_sprintf(S, "%s - quicksave %d", Core.UserName, last_quick);

			NET_Packet			net_packet;
			net_packet.w_begin	(M_SAVE_GAME);
			net_packet.w_stringZ(S);
			net_packet.w_u8		(0);
			Level().Send		(net_packet,net_flags(TRUE));
			
			if (last_quick < quick_save_counter && quick_save_counter > 0)
				last_quick++;
			else
				last_quick = 0;
		}
		else
		{
			if(!valid_file_name(S))
			{
				Msg("invalid file name");
				return;
			}

			NET_Packet			net_packet;
			net_packet.w_begin	(M_SAVE_GAME);
			net_packet.w_stringZ(S);
			net_packet.w_u8		(1);
			Level().Send		(net_packet,net_flags(TRUE));
		}
#ifdef DEBUG
		Msg						("Game save overhead  : %f milliseconds",timer.GetElapsed_sec()*1000.f);
#endif
		SDrawStaticStruct* _s		= HUD().GetUI()->UIGame()->AddCustomStatic("game_saved", true);
		_s->wnd()->SetText			(*CStringTable().translate("st_game_saved"));
		SDrawStaticStruct* _s2		= HUD().GetUI()->UIGame()->AddCustomStatic("game_saved_icon", true);

		xr_strcat				(S,".dds");
		FS.update_path			(S1,"$game_saves$",S);
		
#ifdef DEBUG
		timer.Start				();
#endif
		MainMenu()->Screenshot		(IRender_interface::SM_FOR_GAMESAVE,S1);

#ifdef DEBUG
		Msg						("Screenshot overhead : %f milliseconds",timer.GetElapsed_sec()*1000.f);
#endif
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		get_files_list(tips, "$game_saves$", SAVE_EXTENSION);
	}
};

class CCC_ALifeLoadFrom : public IConsole_Command {
public:
	CCC_ALifeLoadFrom(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args)
	{
		if (!ai().get_alife()) {
			Log						("! ALife simulator has not been started yet");
			return;
		}

		string256					saved_game;
		saved_game[0]				= 0;
//.		sscanf						(args,"%s",saved_game);
		xr_strcpy					(saved_game, args);
		if (!xr_strlen(saved_game)) {
			Log						("! Specify file name!");
			return;
		}

		if (!CSavedGameWrapper::saved_game_exist(saved_game)) {
			Msg						("! Cannot find saved game %s",saved_game);
			return;
		}

		if (!CSavedGameWrapper::valid_saved_game(saved_game)) {
			Msg						("! Cannot load saved game %s, version mismatch or saved game is corrupted",saved_game);
			return;
		}
/*     moved to level_network_messages.cpp
		CSavedGameWrapper			wrapper(args);
		if (wrapper.level_id() == ai().level_graph().level_id()) {
			if (Device.Paused())
				Device.Pause		(FALSE, TRUE, TRUE, "CCC_ALifeLoadFrom");

			Level().remove_objects	();

			game_sv_Single			*game = smart_cast<game_sv_Single*>(Level().Server->game);
			R_ASSERT				(game);
			game->restart_simulator	(saved_game);

			return;
		}
*/
		if(MainMenu()->IsActive())
			MainMenu()->Activate(false);

		if (Device.Paused())
			GAME_PAUSE				(FALSE, TRUE, TRUE, "CCC_ALifeLoadFrom");

		NET_Packet					net_packet;
		net_packet.w_begin			(M_LOAD_GAME);
		net_packet.w_stringZ		(saved_game);
		Level().Send				(net_packet,net_flags(TRUE));
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		get_files_list(tips, "$game_saves$", SAVE_EXTENSION);
	}
};

class CCC_LoadLastSave : public IConsole_Command {
public:
					CCC_LoadLastSave	(LPCSTR N) : IConsole_Command(N)
	{
		bEmptyArgsHandled		= true;
	}

	virtual void	Execute				(LPCSTR args)
	{
		if (args && *args) {
			xr_strcpy				(g_last_saved_game,args);
			return;
		}

		if (!*g_last_saved_game) {
			Msg					("! cannot load last saved game since it hasn't been specified");
			return;
		}

		string512				command;
		if (ai().get_alife()) {
			strconcat			(sizeof(command),command,"load ",g_last_saved_game);
			Console->Execute	(command);
			return;
		}

		strconcat				(sizeof(command),command,"start server(",g_last_saved_game,"/single/alife/load)");
		Console->Execute		(command);
	}
	
	virtual void	Save				(IWriter *F)
	{
		if (!*g_last_saved_game)
			return;

		F->w_printf				("%s %s\r\n",cName,g_last_saved_game); 
	}
};

class CCC_DebugFonts : public IConsole_Command
{
public:
	CCC_DebugFonts(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; }
	virtual void Execute(LPCSTR args)
	{
		if (g_pGamePersistent && g_pGameLevel && Level().game)
			HUD().GetUI()->StartStopMenu(xr_new<CUIDebugFonts>(), true);
		else if (MainMenu() && MainMenu()->IsActive())
			MainMenu()->StartStopMenu(xr_new<CUIDebugFonts>(), true);
	}
	virtual void	Info(TInfo& I)
	{
		strcpy_s(I, "draw all existing fonts");
	}
};

class CCC_FlushLog : public IConsole_Command {
public:
	CCC_FlushLog(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR /**args/**/) {
		FlushLog();
		Msg		("* Log file has been saved successfully!");
	}
};

class CCC_ClearLog : public IConsole_Command {
public:
	CCC_ClearLog(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR) {
		LogFile->clear_not_free	();
		FlushLog				();
		Msg						("* Log file has been cleaned successfully!");
	}
};

class CCC_FloatBlock : public CCC_Float {
public:
	CCC_FloatBlock(LPCSTR N, float* V, float _min=0, float _max=1) :
	  CCC_Float(N,V,_min,_max)
	  {};

	  virtual void	Execute	(LPCSTR args)
	  {
#ifdef _DEBUG
		  CCC_Float::Execute(args);
#else
		  if (!g_pGameLevel || GameID() == GAME_SINGLE)
			  CCC_Float::Execute(args);
		  else
		  {
			  Msg ("! Command disabled for this type of game");
		  }
#endif
	  }
};



class CCC_Net_CL_InputUpdateRate : public CCC_Integer {
protected:
	int		*value_blin;
public:
	CCC_Net_CL_InputUpdateRate(LPCSTR N, int* V, int _min=0, int _max=999) :
	  CCC_Integer(N,V,_min,_max),
		  value_blin(V)
	  {};

	  virtual void	Execute	(LPCSTR args)
	  {
		  CCC_Integer::Execute(args);
		  if ((*value_blin > 0) && g_pGameLevel)
		  {
			  g_dwInputUpdateDelta = 1000/(*value_blin);
		  };
	  }
};

class CCC_Script : public IConsole_Command {
public:
	CCC_Script(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args) {
		string256	S;
		S[0]		= 0;
		sscanf		(args ,"%s",S);
		if (!xr_strlen(S))
			Log("* Specify script name!");
		else {
			// rescan pathes
			FS_Path* P = FS.get_path("$game_scripts$");
			P->m_Flags.set	(FS_Path::flNeedRescan,TRUE);
			//FS.rescan_pathes();
			// run script
			if (ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel))
				ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel)->add_script(S,false,true);
		}
	}
};

class CCC_ScriptCommand : public IConsole_Command {
public:
	CCC_ScriptCommand	(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void	Execute				(LPCSTR args) {
		if (!xr_strlen(args))
			Log("* Specify string to run!");
		else {
#if 1
			if (ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel))
				ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel)->add_script(args,true,true);
#else
			string4096		S;
			shared_str		m_script_name = "console command";
			xr_sprintf			(S,"%s\n",args);
			int				l_iErrorCode = luaL_loadbuffer(ai().script_engine().lua(),S,xr_strlen(S),"@console_command");
			if (!l_iErrorCode) {
				l_iErrorCode = lua_pcall(ai().script_engine().lua(),0,0,0);
				if (l_iErrorCode) {
					ai().script_engine().print_output(ai().script_engine().lua(),*m_script_name,l_iErrorCode);
					return;
				}
			}
			else {
				ai().script_engine().print_output(ai().script_engine().lua(),*m_script_name,l_iErrorCode);
				return;
			}
#endif
		}
	}
};

#ifdef DEBUG

class CCC_DrawGameGraphAll : public IConsole_Command {
public:
				 CCC_DrawGameGraphAll	(LPCSTR N) : IConsole_Command(N)
	{
		bEmptyArgsHandled = true;
	}

	virtual void Execute				(LPCSTR args)
	{
		if (!ai().get_level_graph())
			return;

		ai().level_graph().setup_current_level	(-1);
	}
};

class CCC_DrawGameGraphCurrent : public IConsole_Command {
public:
				 CCC_DrawGameGraphCurrent	(LPCSTR N) : IConsole_Command(N)
	{
		bEmptyArgsHandled = true;
	}

	virtual void Execute					(LPCSTR args)
	{
		if (!ai().get_level_graph())
			return;

		ai().level_graph().setup_current_level	(
			ai().level_graph().level_id()
		);
	}
};

class CCC_DrawGameGraphLevel : public IConsole_Command {
public:
				 CCC_DrawGameGraphLevel	(LPCSTR N) : IConsole_Command(N)
	{
	}

	virtual void Execute					(LPCSTR args)
	{
		if (!ai().get_level_graph())
			return;

		string256			S;
		S[0]				= 0;
		sscanf				(args,"%s",S);

		if (!*S) {
			ai().level_graph().setup_current_level	(-1);
			return;
		}

		const GameGraph::SLevel	*level = ai().game_graph().header().level(S,true);
		if (!level) {
			Msg				("! There is no level %s in the game graph",S);
			return;
		}

		ai().level_graph().setup_current_level	(level->id());
	}
};

class CCC_ScriptDbg : public IConsole_Command {
public:
	CCC_ScriptDbg(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args) {
		
		if(strstr(cName,"script_debug_break")==cName ){
		
		CScriptDebugger* d = ai().script_engine().debugger();
		if(d){
			if(d->Active())
				d->initiateDebugBreak();
			else
				Msg("Script debugger not active.");
		}else
			Msg("Script debugger not present.");
		}
		else if(strstr(cName,"script_debug_stop")==cName ){
			ai().script_engine().stopDebugger();
		}
		else if(strstr(cName,"script_debug_restart")==cName ){
			ai().script_engine().restartDebugger();
		};
	};
	

	virtual void	Info	(TInfo& I)		
	{
		if(strstr(cName,"script_debug_break")==cName )
			xr_strcpy(I,"initiate script debugger [DebugBreak] command"); 

		else if(strstr(cName,"script_debug_stop")==cName )
			xr_strcpy(I,"stop script debugger activity"); 

		else if(strstr(cName,"script_debug_restart")==cName )
			xr_strcpy(I,"restarts script debugger or start if no script debugger presents"); 
	}
};

class CCC_DumpInfos : public IConsole_Command {
public:
	CCC_DumpInfos	(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void	Execute				(LPCSTR args) {
		CActor* A =smart_cast<CActor*>(Level().CurrentEntity());
		if(A)
			A->DumpInfo();
	}
	virtual void	Info	(TInfo& I)		
	{
		xr_strcpy(I,"dumps all infoportions that actor have"); 
	}
};
#include "map_manager.h"
class CCC_DumpMap : public IConsole_Command {
public:
	CCC_DumpMap	(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void	Execute				(LPCSTR args) {
		Level().MapManager().Dump();
	}
	virtual void	Info	(TInfo& I)		
	{
		xr_strcpy(I,"dumps all currentmap locations"); 
	}

};

#include "alife_graph_registry.h"
class CCC_DumpCreatures : public IConsole_Command {
public:
	CCC_DumpCreatures	(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void	Execute				(LPCSTR args) {
		
		typedef CSafeMapIterator<ALife::_OBJECT_ID,CSE_ALifeDynamicObject>::_REGISTRY::const_iterator const_iterator;

		const_iterator I = ai().alife().graph().level().objects().begin();
		const_iterator E = ai().alife().graph().level().objects().end();
		for ( ; I != E; ++I) {
			CSE_ALifeCreatureAbstract *obj = smart_cast<CSE_ALifeCreatureAbstract *>(I->second);
			if (obj) {
				Msg("\"%s\",",obj->name_replace());
			}
		}		

	}
	virtual void	Info	(TInfo& I)		
	{
		xr_strcpy(I,"dumps all creature names"); 
	}

};

class CCC_DebugNode : public IConsole_Command {
public:
	CCC_DebugNode(LPCSTR N) : IConsole_Command(N)  { };

	virtual void Execute(LPCSTR args) {

		string128 param1, param2;
		_GetItem(args,0,param1,' ');
		_GetItem(args,1,param2,' ');

		u32 value1;
		u32 value2;
		
		sscanf(param1,"%u",&value1);
		sscanf(param2,"%u",&value2);
		
		if ((value1 > 0) && (value2 > 0)) {
			g_bDebugNode		= TRUE;
			g_dwDebugNodeSource	= value1;
			g_dwDebugNodeDest	= value2;
		} else {
			g_bDebugNode = FALSE;
		}
	}
};

class CCC_ShowMonsterInfo : public IConsole_Command {
public:
				CCC_ShowMonsterInfo(LPCSTR N) : IConsole_Command(N)  { };

	virtual void Execute(LPCSTR args) {

		string128 param1, param2;
		_GetItem(args,0,param1,' ');
		_GetItem(args,1,param2,' ');

		CObject			*obj = Level().Objects.FindObjectByName(param1);
		CBaseMonster	*monster = smart_cast<CBaseMonster *>(obj);
		if (!monster)	return;
		
		u32				value2;
		
		sscanf			(param2,"%u",&value2);
		monster->set_show_debug_info (u8(value2));
	}
};
class CCC_DbgPhTrackObj : public IConsole_Command {
public:
	CCC_DbgPhTrackObj(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args/**/) {
			ph_dbg_draw_mask1.set(ph_m1_DbgTrackObject,TRUE);
			PH_DBG_SetTrackObject(args);
			//CObject* O= Level().Objects.FindObjectByName(args);
			//if(O)
			//{
			//	PH_DBG_SetTrackObject(*(O->cName()));
			//	ph_dbg_draw_mask1.set(ph_m1_DbgTrackObject,TRUE);
			//}

		}
	
	//virtual void	Info	(TInfo& I)		
	//{
	//	xr_strcpy(I,"restart game fast"); 
	//}
};
#endif

class CCC_PHIterations : public CCC_Integer {
public:
		CCC_PHIterations(LPCSTR N) :
		CCC_Integer(N,&phIterations,15,50)
		{};
	  virtual void	Execute	(LPCSTR args)
	  {
		  CCC_Integer::Execute	(args);
		  dWorldSetQuickStepNumIterations(NULL,phIterations);
	  }
};

#ifdef DEBUG
class CCC_PHGravity : public IConsole_Command {
public:
		CCC_PHGravity(LPCSTR N) :
		IConsole_Command(N)
		{};
	  virtual void	Execute	(LPCSTR args)
	  {
		  if(!ph_world)	return;
#ifndef DEBUG
		  if (g_pGameLevel && Level().game && GameID() != GAME_SINGLE)
		  {
			  Msg("Command is not available in Multiplayer");
			  return;
		  }
#endif
		  ph_world->SetGravity(float(atof(args)));
	  }
	  virtual void	Status	(TStatus& S)
	{	
		if(ph_world)
			xr_sprintf	(S,"%3.5f",ph_world->Gravity());
		else
			xr_sprintf	(S,"%3.5f",default_world_gravity);
		while	(xr_strlen(S) && ('0'==S[xr_strlen(S)-1]))	S[xr_strlen(S)-1] = 0;
	}
	
};
#endif // DEBUG

class CCC_PHFps : public IConsole_Command {
public:
	CCC_PHFps(LPCSTR N) :
	  IConsole_Command(N)
	  {};
	  virtual void	Execute	(LPCSTR args)
	  {
		  float				step_count = (float)atof(args);
		  clamp				(step_count,50.f,200.f);
		  CPHWorld::SetStep(1.f/step_count);
	  }
	  virtual void	Status	(TStatus& S)
	  {	
		 	xr_sprintf	(S,"%3.5f",1.f/fixed_step);	  
	  }

};

#ifdef DEBUG
extern void print_help(lua_State *L);

struct CCC_LuaHelp : public IConsole_Command {
	CCC_LuaHelp(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args) {
		print_help(ai().script_engine().lua());
	}
};

struct CCC_ShowSmartCastStats : public IConsole_Command {
	CCC_ShowSmartCastStats(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args) {
		show_smart_cast_stats();
	}
};

struct CCC_ClearSmartCastStats : public IConsole_Command {
	CCC_ClearSmartCastStats(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args) {
		clear_smart_cast_stats();
	}
};
#endif

#	include "game_graph.h"
struct CCC_JumpToLevel : public IConsole_Command {
	CCC_JumpToLevel(LPCSTR N) : IConsole_Command(N)  {};

	virtual void Execute(LPCSTR args) {
		if (!ai().get_alife()) {
			Msg				("! ALife simulator is needed to perform specified command!");
			return;
		}
		string256		level;
		sscanf(args,"%s",level);

		GameGraph::LEVEL_MAP::const_iterator	I = ai().game_graph().header().levels().begin();
		GameGraph::LEVEL_MAP::const_iterator	E = ai().game_graph().header().levels().end();
		for ( ; I != E; ++I)
			if (!xr_strcmp((*I).second.name(),level)) {
				ai().alife().jump_to_level(level);
				return;
			}
		Msg							("! There is no level \"%s\" in the game graph!",level);
	}

	virtual void	Save(IWriter *F) {};
	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		if (!ai().get_alife())
		{
			Msg("! ALife simulator is needed to perform specified command!");
			return;
		}

		GameGraph::LEVEL_MAP::const_iterator	itb = ai().game_graph().header().levels().begin();
		GameGraph::LEVEL_MAP::const_iterator	ite = ai().game_graph().header().levels().end();
		for (; itb != ite; ++itb)
		{
			tips.push_back((*itb).second.name());
		}
	}
};

#include "GamePersistent.h"

class CCC_MainMenu : public IConsole_Command {
public:
	CCC_MainMenu(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args) {

		bool bWhatToDo = TRUE;
		if( 0==xr_strlen(args) ){
			bWhatToDo = !MainMenu()->IsActive();
		};

		if( EQ(args,"on")||EQ(args,"1") )
			bWhatToDo = TRUE;

		if( EQ(args,"off")||EQ(args,"0") )
			bWhatToDo = FALSE;

		MainMenu()->Activate( bWhatToDo );
	}
};

struct CCC_StartTimeSingle : public IConsole_Command {
	CCC_StartTimeSingle(LPCSTR N) : IConsole_Command(N) {};
	virtual void	Execute	(LPCSTR args)
	{
		u32 year = 1, month = 1, day = 1, hours = 0, mins = 0, secs = 0, milisecs = 0;
		sscanf				(args,"%d.%d.%d %d:%d:%d.%d",&year,&month,&day,&hours,&mins,&secs,&milisecs);
		year				= _max(year,1);
		month				= _max(month,1);
		day					= _max(day,1);
		g_qwStartGameTime	= generate_time	(year,month,day,hours,mins,secs,milisecs);

		if (!g_pGameLevel)
			return;

		if (!Level().Server)
			return;

		if (!Level().Server->game)
			return;

		Level().Server->game->SetGameTimeFactor(g_qwStartGameTime,g_fTimeFactor);
	}

	virtual void	Status	(TStatus& S)
	{
		u32 year = 1, month = 1, day = 1, hours = 0, mins = 0, secs = 0, milisecs = 0;
		split_time	(g_qwStartGameTime, year, month, day, hours, mins, secs, milisecs);
		xr_sprintf		(S,"%d.%d.%d %d:%d:%d.%d",year,month,day,hours,mins,secs,milisecs);
	}
};

struct CCC_TimeFactorSingle : public CCC_Float {
	CCC_TimeFactorSingle(LPCSTR N, float* V, float _min=0.f, float _max=1.f) : CCC_Float(N,V,_min,_max) {};

	virtual void	Execute	(LPCSTR args)
	{
		CCC_Float::Execute	(args);
		
		if (!g_pGameLevel)
			return;

		if (!Level().Server)
			return;

		if (!Level().Server->game)
			return;

		Level().Server->game->SetGameTimeFactor(g_fTimeFactor);
	}
};

#ifdef DEBUG
class CCC_RadioGroupMask2;
class CCC_RadioMask :public CCC_Mask
{
	CCC_RadioGroupMask2		*group;
public:
	CCC_RadioMask(LPCSTR N, Flags32* V, u32 M):
	  CCC_Mask(N,V,M)
	 {
		group=NULL;
	 }
		void	SetGroup	(CCC_RadioGroupMask2		*G)
	{
		group=G													;
	}
virtual	void	Execute		(LPCSTR args)						;
	
IC		void	Set			(BOOL V)
	  {
		  value->set(mask,V)									;
	  }

};

class CCC_RadioGroupMask2 
{
	CCC_RadioMask *mask0;
	CCC_RadioMask *mask1;
public:
	CCC_RadioGroupMask2(CCC_RadioMask *m0,CCC_RadioMask *m1)
	  {
		mask0=m0;mask1=m1;
		mask0->SetGroup(this);
		mask1->SetGroup(this);
	  }
	void	Execute	(CCC_RadioMask& m,LPCSTR args)
	{
		BOOL value=m.GetValue();
		if(value)
		{
			mask0->Set(!value);mask1->Set(!value);
		}
		m.Set(value);
	}
};


void	CCC_RadioMask::Execute	(LPCSTR args)
{
	CCC_Mask::Execute(args);
	VERIFY2(group,"CCC_RadioMask: group not set");
	group->Execute(*this,args);
}

#define CMD_RADIOGROUPMASK2(p1,p2,p3,p4,p5,p6)		\
{\
static CCC_RadioMask x##CCC_RadioMask1(p1,p2,p3);		Console->AddCommand(&x##CCC_RadioMask1);\
static CCC_RadioMask x##CCC_RadioMask2(p4,p5,p6);		Console->AddCommand(&x##CCC_RadioMask2);\
static CCC_RadioGroupMask2 x##CCC_RadioGroupMask2(&x##CCC_RadioMask1,&x##CCC_RadioMask2);\
}

struct CCC_DbgBullets : public CCC_Integer {
	CCC_DbgBullets(LPCSTR N, int* V, int _min=0, int _max=999) : CCC_Integer(N,V,_min,_max) {};

	virtual void	Execute	(LPCSTR args)
	{
		extern FvectorVec g_hit[];
		g_hit[0].clear();
		g_hit[1].clear();
		g_hit[2].clear();
		CCC_Integer::Execute	(args);
	}
};

#include "attachable_item.h"
#include "attachment_owner.h"
class CCC_TuneAttachableItem : public IConsole_Command
{
public		:
	CCC_TuneAttachableItem(LPCSTR N):IConsole_Command(N){};
	virtual void	Execute	(LPCSTR args)
	{
		if( CAttachableItem::m_dbgItem){
			CAttachableItem::m_dbgItem = NULL;	
			Msg("CCC_TuneAttachableItem switched to off");
			return;
		};

		CObject* obj = Level().CurrentViewEntity();	VERIFY(obj);
		CAttachmentOwner* owner = smart_cast<CAttachmentOwner*>(obj);
		shared_str ssss = args;
		CAttachableItem* itm = owner->attachedItem(ssss);
		if(itm){
			CAttachableItem::m_dbgItem = itm;
			Msg("CCC_TuneAttachableItem switched to ON for [%s]",args);
		}else
			Msg("CCC_TuneAttachableItem cannot find attached item [%s]",args);
	}

	virtual void	Info	(TInfo& I)
	{	
		xr_sprintf(I,"allows to change bind rotation and position offsets for attached item, <section_name> given as arguments");
	}
};

class CCC_Crash : public IConsole_Command {
public:
	CCC_Crash(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR /**args/**/) {
		VERIFY3					(false,"This is a test crash","Do not post it as a bug");
		int						*pointer = 0;
		*pointer				= 0;
	}
};

#ifdef DEBUG_MEMORY_MANAGER

class CCC_MemAllocShowStats : public IConsole_Command {
public:
	CCC_MemAllocShowStats(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR) {
		mem_alloc_show_stats	();
	}
};

class CCC_MemAllocClearStats : public IConsole_Command {
public:
	CCC_MemAllocClearStats(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR) {
		mem_alloc_clear_stats	();
	}
};

#endif // DEBUG_MEMORY_MANAGER

class CCC_DumpModelBones : public IConsole_Command {
public:
	CCC_DumpModelBones	(LPCSTR N) : IConsole_Command(N)
	{
	}
	
	virtual void Execute(LPCSTR arguments)
	{
		if (!arguments || !*arguments) {
			Msg					("! no arguments passed");
			return;
		}

		string_path				name;
		string_path				fn;

		if (0==strext(arguments))
			strconcat			(sizeof(name),name,arguments,".ogf");
		else
			xr_strcpy			(name,sizeof(name),arguments);

		if (!FS.exist(arguments) && !FS.exist(fn, "$level$", name) && !FS.exist(fn, "$game_meshes$", name)) {
			Msg					("! Cannot find visual \"%s\"",arguments);
			return;
		}

		IRenderVisual			*visual = Render->model_Create(arguments);
		IKinematics				*kinematics = smart_cast<IKinematics*>(visual);
		if (!kinematics) {
			Render->model_Delete(visual);
			Msg					("! Invalid visual type \"%s\" (not a IKinematics)",arguments);
			return;
		}

		Msg						("bones for model \"%s\"",arguments);
		for (u16 i=0, n=kinematics->LL_BoneCount(); i<n; ++i)
			Msg					("%s",*kinematics->LL_GetData(i).name);
		
		Render->model_Delete	(visual);
	}
};

extern void show_animation_stats	();

class CCC_ShowAnimationStats : public IConsole_Command {
public:
	CCC_ShowAnimationStats(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR)
	{
		show_animation_stats	();
	}
};

#endif // DEBUG

class CCC_DumpObjects : public IConsole_Command {
public:
	CCC_DumpObjects(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR)
	{
		Level().Objects.dump_all_objects();
	}
};

class CCC_GSCheckForUpdates : public IConsole_Command {
public:
	CCC_GSCheckForUpdates(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR arguments)
	{
		if (!MainMenu()) return;
		/*
		CGameSpy_Available GSA;
		shared_str result_string;
		if (!GSA.CheckAvailableServices(result_string))
		{
			Msg(*result_string);
//			return;
		};
		CGameSpy_Patching GameSpyPatching;
		*/
		bool InformOfNoPatch = true;
		if (arguments && *arguments) {
			int bInfo = 1;
			sscanf	(arguments,"%d", &bInfo);
			InformOfNoPatch = (bInfo != 0);
		}
		
//		GameSpyPatching.CheckForPatch(InformOfNoPatch);
		
		MainMenu()->GetGS()->m_pGS_Patching->CheckForPatch(InformOfNoPatch);
	}
};



class CCC_Net_SV_GuaranteedPacketMode : public CCC_Integer {
protected:
	int		*value_blin;
public:
	CCC_Net_SV_GuaranteedPacketMode(LPCSTR N, int* V, int _min=0, int _max=2) :
	  CCC_Integer(N,V,_min,_max),
		  value_blin(V)
	  {};

	  virtual void	Execute	(LPCSTR args)
	  {
		  CCC_Integer::Execute(args);
	  }
};

// AXR: New Commands

class CCC_Spawn : public IConsole_Command {
public:
	CCC_Spawn(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR args) {
		if (!g_pGameLevel) return;

		int count = 1;
		char	Name[128];	Name[0] = 0;
		sscanf(args, "%s %d", Name, &count);

		if (GameID() != eGameIDSingle)
		{
			Msg("For this game type entity-spawning is disabled.");
			return;
		};

		if (count > 50)
		{
			Msg("! [g_spawn]: Cancel the command. Maximum value of the second argument: 50. Count is: %d", count);
			return;
		}

		if (!pSettings->section_exist(Name))
		{
			Msg("! Section [%s] isn`t exist...", Name);
			return;
		}

		collide::rq_result RQ = Level().GetPickResult(Device.vCameraPosition, Device.vCameraDirection, 1000.0f, Level().CurrentControlEntity());
		Fvector pos = Fvector(Device.vCameraPosition).add(Fvector(Device.vCameraDirection).mul(RQ.range));

		if (auto tpGame = smart_cast<game_sv_Single*>(Level().Server->game))
		{
			for (int i = 0; i < count; ++i)
			{
				CSE_Abstract* entity = tpGame->alife().spawn_item(Name, pos, Actor()->ai_location().level_vertex_id(), Actor()->ai_location().game_vertex_id(), ALife::_OBJECT_ID(-1));

				if (CSE_ALifeAnomalousZone* anom = smart_cast<CSE_ALifeAnomalousZone*>(entity))
				{
					CShapeData::shape_def _shape;
					_shape.data.sphere.P.set(0.0f, 0.0f, 0.0f);
					_shape.data.sphere.R = 3.0f;
					_shape.type = CShapeData::cfSphere;
					anom->assign_shapes(&_shape, 1);
					anom->m_space_restrictor_type = RestrictionSpace::eRestrictorTypeNone;
				}
			}
		}
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		for (auto sect : pSettings->sections())
		{
			if (sect->line_exist("class") && sect->line_exist("$spawn"))
				tips.push_back(sect->Name.c_str());
		}
	}

	virtual void	Info(TInfo& I)
	{
		strcpy(I, "name,team,squad,group");
	}
};
// g_spawn

class CCC_Giveinfo : public IConsole_Command {
public:
	CCC_Giveinfo(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR info_id)
	{
		if (!g_pGameLevel) return;

		char	Name[128];	Name[0] = 0;
		CActor* actor = smart_cast<CActor*>(Level().CurrentEntity());

		if (actor)
			actor->OnReceiveInfo(info_id);
		else
			Msg("! [g_info] : Actor not found!");
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		if (!ai().get_alife())
		{
			Msg("! ALife simulator is needed to perform specified command!");
			return;
		}
		for (const auto& it : *CInfoPortion::Items())
		{
			auto& name = it.id;
			tips.push_back(name);
		}
		std::sort(tips.begin(), tips.end());
	}
};

class CCC_Disinfo : public IConsole_Command {
public:
	CCC_Disinfo(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR info_id)
	{
		if (!g_pGameLevel) return;

		char	Name[128];	Name[0] = 0;
		CActor* actor = smart_cast<CActor*>(Level().CurrentEntity());

		if (actor)
			actor->OnDisableInfo(info_id);
		else
			Msg("! [g_info] : Actor not found!");
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		if (!ai().get_alife())
		{
			Msg("! ALife simulator is needed to perform specified command!");
			return;
		}

		for (const auto& it : *CInfoPortion::Items())
		{
			auto& name = it.id;
			tips.push_back(name);
		}

		std::sort(tips.begin(), tips.end());
	}
};

class CCC_Spawn_to_inv : public IConsole_Command {
public:
	CCC_Spawn_to_inv(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR args) {
		if (!g_pGameLevel)
		{
			Log("Error: No game level!");
			return;
		}

		int count = 1;
		char	Name[128];	Name[0] = 0;
		sscanf(args, "%s %d", Name, &count);

		if (count > 250)
		{
			Msg("! [g_spawn_to_inventory]: Cancel the command. Maximum value of the second argument: 250. Cound is: %d", count);
			return;
		}

		if (!pSettings->section_exist(Name))
		{
			Msg("! Section [%s] isn`t exist...", Name);
			return;
		}

		if (!pSettings->line_exist(Name, "class") || !pSettings->line_exist(Name, "inv_weight") || !pSettings->line_exist(Name, "visual"))
		{
			Msg("!Failed to load section!");
			return;
		}

		for (int i = 0; i < count; ++i)
			Level().spawn_item(Name, Actor()->Position(), false, Actor()->ID());
	}
	virtual void	Info(TInfo& I)
	{
		strcpy(I, "name,team,squad,group");
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		for (auto sect : pSettings->sections()) {
			if (sect->line_exist("class") && sect->line_exist("inv_weight"))
				tips.push_back(sect->Name.c_str());
		}
	}
};

// Change weather immediately
class CCC_SetWeather : public IConsole_Command
{
public:
	CCC_SetWeather(LPCSTR N) : IConsole_Command(N) {};
	virtual void Execute(LPCSTR args)
	{
		if (!xr_strlen(args))
			return;
		if (!g_pGamePersistent)
			return;
		//if (!Device.editor())
		g_pGamePersistent->Environment().SetWeather(args, true);
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		if (FS.path_exist("$game_weathers$"))
			get_files_list(tips, "$game_weathers$", ".ltx");
		else
		{
			for (auto& it : g_pGamePersistent->Environment().WeatherCycles)
				tips.push_back(it.first);
		}
	}
};

class CCC_UiHud_Mode : public CCC_Token
{
public:
	CCC_UiHud_Mode(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N, V, T) {};

	virtual void	Execute(LPCSTR args) {
		CCC_Token::Execute(args);

		if (g_pGamePersistent && g_pGameLevel && Level().game)
		{
			if (*value >= 1 && *value <= 3)
			{
				HUD().OnScreenResolutionChanged();
			}
		}
	}
};

extern CUIXml* g_gameTaskXml;

class CCC_GiveTask : public IConsole_Command
{
public:
	CCC_GiveTask(LPCSTR N) : IConsole_Command(N) {};
	virtual void Execute(LPCSTR task)
	{
		if (!g_pGameLevel)
		{
			Log("Error: No game level!");
			return;
		}

		CActor* actor = smart_cast<CActor*>(Level().CurrentEntity());
		if (actor)
			actor->GameTaskManager().GiveGameTaskToActor(task, 0, true);
		else
			Msg("! [g_task] : Actor not found!");
	}

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		if (!ai().get_alife())
		{
			Msg("! ALife simulator is needed to perform specified command!");
			return;
		}

		if (g_gameTaskXml)
		{
			const int tag_num = g_gameTaskXml->GetNodesNum(g_gameTaskXml->GetLocalRoot(), "game_task");

			for (int i = 0; i < tag_num; i++)
			{
				XML_NODE* l_root = g_gameTaskXml->NavigateToNode("game_task", i);
				g_gameTaskXml->SetLocalRoot(l_root);

				if (auto name = g_gameTaskXml->ReadAttrib(l_root, "id", nullptr))
				{
					tips.emplace_back(name);
				}
			}

			std::sort(tips.begin(), tips.end());
		}
	}
};

class CCC_GiveMoney : public IConsole_Command
{
public:
	CCC_GiveMoney(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR money)
	{
		if (!g_pGameLevel)
		{
			Log("Error: No game level!");
			return;
		}

		CActor* actor = smart_cast<CActor*>(Level().CurrentEntity());
		int	m_iMoney = (int)atoi(money);
		if (actor)
		{
			Actor()->set_money(Actor()->get_money() + m_iMoney, false);
		}
		else
			Msg("! [g_money] : Actor not found!");
	}
};

struct DumpTxrsForPrefetching : public IConsole_Command {
	DumpTxrsForPrefetching(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		MainMenu()->ReportTxrsForPrefetching();
	}
};

// kill
class CCC_KillEntity : public IConsole_Command {
public:
	CCC_KillEntity(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args)
	{
		if (!g_pGameLevel) return;

		char story_id_to_kill[128];
		story_id_to_kill[0] = 0;

		sscanf(args, "%s", story_id_to_kill);

		collide::rq_result& RQ = HUD().GetCurrentRayQuery();

		if (story_id_to_kill[0] != 0)
		{
			u16 id_to_kill{};

			luabind::functor<u16> m_functor;
			if (ai().script_engine().functor("mfs_functions.get_id_by_sid", m_functor));
			id_to_kill = m_functor(story_id_to_kill);

			if (!id_to_kill)
			{
				Msg("! [kill] : Invalid story_id or NPC offline! story_id: %s", story_id_to_kill);
				return;
			}

			CEntityAlive* entity_to_kill = smart_cast<CEntityAlive*>(Level().Objects.net_Find(id_to_kill));

			if (entity_to_kill)
			{
				if (!entity_to_kill->g_Alive())
				{
					Msg("! [kill] : This entity is already dead!");
					return;
				}

				entity_to_kill->KillEntity(entity_to_kill->ID());
			}
			else
				Msg("! [kill] : Entity with id [%s] not found!", story_id_to_kill);
		}
		else if (RQ.O && story_id_to_kill[0] == 0)
		{
			CEntityAlive* entity_to_kill = smart_cast<CEntityAlive*>(RQ.O);

			if (entity_to_kill)
			{
				if (!entity_to_kill->g_Alive())
				{
					Msg("! [kill] : This entity is already dead!");
					return;
				}

				entity_to_kill->KillEntity(entity_to_kill->ID());
			}
			else
				Msg("! [kill] : Is not EntityAlive!");
		}
		else
			Msg("! [kill] : Empty entity to kill or is not EntityAlive!");
	}

	virtual void	Info(TInfo& I)
	{
		strcpy(I, "name,team,squad,group");
	}
};

struct CCC_ReloadSystemLtx : public IConsole_Command
{
	CCC_ReloadSystemLtx(LPCSTR N) : IConsole_Command(N)
	{
		bEmptyArgsHandled = true;
	};

	virtual void Execute(LPCSTR args)
	{
		string_path fname;
		FS.update_path(fname, "$game_config$", "system.ltx");
		CInifile::Destroy(pSettings);
		pSettings = new CInifile(fname, TRUE);
		CHECK_OR_EXIT(0 != pSettings->section_count(), make_string("Cannot find file %s.\nReinstalling application may fix this problem.", fname));
		Msg("system.ltx was reloaded.");
	}
};

struct CCC_ReloadAdvancedXRayCfg : public IConsole_Command
{
	CCC_ReloadAdvancedXRayCfg(LPCSTR N) : IConsole_Command(N)
	{
		bEmptyArgsHandled = true;
	};

	virtual void Execute(LPCSTR args)
	{
		string_path fname;
		FS.update_path(fname, "$game_config$", "AdvancedXRay.ltx");
		CInifile::Destroy(pAdvancedSettings);
		pAdvancedSettings = new CInifile(fname, TRUE);
		CHECK_OR_EXIT(0 != pAdvancedSettings->section_count(), make_string("Cannot find file %s.\nReinstalling application may fix this problem.", fname));
		GameConstants::LoadConstants();
		Msg("AdvancedXRay.ltx was reloaded.");
	}
};

class CCC_GameLanguage : public CCC_Token
{
public:
	CCC_GameLanguage(pcstr N) : CCC_Token(N, (u32*)&CStringTable::LanguageID, nullptr) {}

	void Execute(pcstr args)// override
	{
		CCC_Token::Execute(args);
		CStringTable().ReloadLanguage();

		if (!g_pGameLevel)
			return;

		for (u16 id = 0; id < 0xffff; id++)
		{
			CObject* gameObj = Level().Objects.net_Find(id);
			if (gameObj)
			{
				if (CInventoryItem* invItem = gameObj->cast_inventory_item())
					invItem->ReloadNames();
			}
		}
	}

	xr_token* GetToken() noexcept override
	{
		tokens = CStringTable().GetLanguagesToken();
		if (!tokens) // Prevent failure without usage Nifty counters
		{
			Msg("GetToken: token missing");
			CStringTable().Destroy();
			CStringTable().Init();

			tokens = CStringTable().GetLanguagesToken();
		}
		return CCC_Token::GetToken();
	}
};

struct path_excluder_predicate
{
	explicit path_excluder_predicate(xr_auth_strings_t const* ignore) :
		m_ignore(ignore)
	{
	}
	bool xr_stdcall is_allow_include(LPCSTR path)
	{
		if (!m_ignore)
			return true;

		return allow_to_include_path(*m_ignore, path);
	}
	xr_auth_strings_t const* m_ignore;
};


class CCC_ReloadWeather : public IConsole_Command
{
public:
	CCC_ReloadWeather(LPCSTR N) : IConsole_Command(N)
	{
		bEmptyArgsHandled = true;
	};

	virtual void Execute(LPCSTR args)
	{
		if (!g_pGameLevel)
		{
			Log("Error: No game level!");
			return;
		}

		if (!g_pGamePersistent->Environment().GetWeather().size())
		{
			Log("CCC_ReloadWeather error: No weather in game!");
			return;
		}

		g_pGamePersistent->DestroyEnvironment();

		Msg("CCC_ReloadWeather: Environment destroyed");
		Msg("CCC_ReloadWeather: Start to destroy configs");
		CInifile** s = (CInifile**)(&pSettings);
		xr_delete(*s);
		xr_delete(pGameIni);
		Msg("CCC_ReloadWeather: Start to rescan configs");
		FS.get_path("$game_config$")->m_Flags.set(FS_Path::flNeedRescan, TRUE);
		FS.get_path("$game_scripts$")->m_Flags.set(FS_Path::flNeedRescan, TRUE);
		FS.rescan_pathes();

		Msg("CCC_ReloadWeather: Start to create configs");
		string_path					fname;
		FS.update_path(fname, "$game_config$", "system.ltx");
		Msg("CCC_ReloadWeather: Updated path to system.ltx is %s", fname);

		pSettings = xr_new<CInifile>(fname, TRUE);
		CHECK_OR_EXIT(0 != pSettings->section_count(), make_string("CCC_ReloadWeather: Cannot find file %s.\nReinstalling application may fix this problem.", fname));

		xr_auth_strings_t			tmp_ignore_pathes;
		xr_auth_strings_t			tmp_check_pathes;
		fill_auth_check_params(tmp_ignore_pathes, tmp_check_pathes);

		path_excluder_predicate			tmp_excluder(&tmp_ignore_pathes);
		CInifile::allow_include_func_t	tmp_functor;
		tmp_functor.bind(&tmp_excluder, &path_excluder_predicate::is_allow_include);
		pSettingsAuth = xr_new<CInifile>(
			fname,
			TRUE,
			TRUE,
			FALSE,
			0,
			tmp_functor
			);

		FS.update_path(fname, "$game_config$", "game.ltx");
		pGameIni = xr_new<CInifile>(fname, TRUE);
		CHECK_OR_EXIT(0 != pGameIni->section_count(), make_string("CCC_ReloadWeather: Cannot find file %s.\nReinstalling application may fix this problem.", fname));

		Msg("CCC_ReloadWeather: Create environment");
		g_pGamePersistent->CreateEnvironment();

		Msg("CCC_ReloadWeather: Call level_weathers.restart_weather_manager");
		luabind::functor<void>	lua_function;
		string256		fn;
		xr_strcpy(fn, "level_weathers.restart_weather_manager");
		if (ai().script_engine().functor<void>(fn, lua_function))
			lua_function();
		else
			Msg("CCC_ReloadWeather: Can't find function %s", fn);

		Msg("CCC_ReloadWeather: Reload weather done!");
	}
};

void CCC_RegisterCommands()
{
	// options
	g_OptConCom.Init();

	CMD1(CCC_MemStats,			"stat_memory"			);
	// game
	psActorFlags.set(AF_ALWAYSRUN, true);
	CMD3(CCC_Mask,				"g_always_run",			&psActorFlags,	AF_ALWAYSRUN);
	CMD1(CCC_GameDifficulty,	"g_game_difficulty"		);

	CMD3(CCC_Mask,				"g_backrun",			&psActorFlags,	AF_RUN_BACKWARD);

	// alife
#ifdef DEBUG
	CMD1(CCC_ALifePath,			"al_path"				);		// build path
#endif // DEBUG
	
	CMD1(CCC_ALifeSave,			"save"					);		// save game
	CMD1(CCC_ALifeLoadFrom,		"load"					);		// load game from ...
	CMD1(CCC_LoadLastSave,		"load_last_save"		);		// load last saved game from ...

	CMD1(CCC_FlushLog,			"flush"					);		// flush log
	CMD1(CCC_ClearLog,			"clear_log"					);

	CMD1(CCC_ALifeSwitchDistance,	"al_switch_distance"	);		// set switch distance

#ifndef MASTER_GOLD
	CMD1(CCC_ALifeTimeFactor,		"al_time_factor"		);		// set time factor
	CMD1(CCC_ALifeProcessTime,		"al_process_time"		);		// set process time
	CMD1(CCC_ALifeObjectsPerUpdate,	"al_objects_per_update"	);		// set process time
	CMD1(CCC_ALifeSwitchFactor,		"al_switch_factor"		);		// set switch factor
#endif // MASTER_GOLD


	CMD3(CCC_Mask,				"hud_weapon",			&psHUD_Flags,	HUD_WEAPON);
	CMD3(CCC_Mask,				"hud_info",				&psHUD_Flags,	HUD_INFO);

	CMD3(CCC_Mask,				"hud_draw",				&psHUD_Flags,	HUD_DRAW);

	// hud
	psHUD_Flags.set(HUD_CROSSHAIR,		true);
	psHUD_Flags.set(HUD_WEAPON,			true);
	psHUD_Flags.set(HUD_DRAW,			true);
	psHUD_Flags.set(HUD_INFO,			true);

	CMD3(CCC_Mask,				"hud_crosshair",		&psHUD_Flags,	HUD_CROSSHAIR);
	CMD3(CCC_Mask,				"hud_crosshair_dist",	&psHUD_Flags,	HUD_CROSSHAIR_DIST);

	CMD4(CCC_Float,				"hud_fov",				&psHUD_FOV_def,	0.25f,	1.0f);
	CMD4(CCC_Float,				"cam_fov",				&g_fov,			5.0f,	180.0f);

	// Demo
	CMD1(CCC_DemoPlay,			"demo_play"				);
	CMD1(CCC_DemoRecord,		"demo_record"			);

#ifndef MASTER_GOLD
	// ai
	CMD3(CCC_Mask,				"mt_ai_vision",			&g_mt_config,	mtAiVision);
	CMD3(CCC_Mask,				"mt_level_path",		&g_mt_config,	mtLevelPath);
	CMD3(CCC_Mask,				"mt_detail_path",		&g_mt_config,	mtDetailPath);
	CMD3(CCC_Mask,				"mt_object_handler",	&g_mt_config,	mtObjectHandler);
	CMD3(CCC_Mask,				"mt_sound_player",		&g_mt_config,	mtSoundPlayer);
	CMD3(CCC_Mask,				"mt_bullets",			&g_mt_config,	mtBullets);
	CMD3(CCC_Mask,				"mt_script_gc",			&g_mt_config,	mtLUA_GC);
	CMD3(CCC_Mask,				"mt_level_sounds",		&g_mt_config,	mtLevelSounds);
	CMD3(CCC_Mask,				"mt_alife",				&g_mt_config,	mtALife);
#endif // MASTER_GOLD

#ifdef DEBUG
	CMD4(CCC_Integer,			"lua_gcstep",			&psLUA_GCSTEP,	1, 1000);
	CMD3(CCC_Mask,				"ai_debug",				&psAI_Flags,	aiDebug);
	CMD3(CCC_Mask,				"ai_dbg_brain",			&psAI_Flags,	aiBrain);
	CMD3(CCC_Mask,				"ai_dbg_motion",		&psAI_Flags,	aiMotion);
	CMD3(CCC_Mask,				"ai_dbg_frustum",		&psAI_Flags,	aiFrustum);
	CMD3(CCC_Mask,				"ai_dbg_funcs",			&psAI_Flags,	aiFuncs);
	CMD3(CCC_Mask,				"ai_dbg_alife",			&psAI_Flags,	aiALife);
	CMD3(CCC_Mask,				"ai_dbg_lua",			&psAI_Flags,	aiLua);
	CMD3(CCC_Mask,				"ai_dbg_goap",			&psAI_Flags,	aiGOAP);
	CMD3(CCC_Mask,				"ai_dbg_goap_script",	&psAI_Flags,	aiGOAPScript);
	CMD3(CCC_Mask,				"ai_dbg_goap_object",	&psAI_Flags,	aiGOAPObject);
	CMD3(CCC_Mask,				"ai_dbg_cover",			&psAI_Flags,	aiCover);
	CMD3(CCC_Mask,				"ai_dbg_anim",			&psAI_Flags,	aiAnimation);
	CMD3(CCC_Mask,				"ai_dbg_vision",		&psAI_Flags,	aiVision);
	CMD3(CCC_Mask,				"ai_dbg_monster",		&psAI_Flags,	aiMonsterDebug);
	CMD3(CCC_Mask,				"ai_dbg_stalker",		&psAI_Flags,	aiStalker);
	CMD3(CCC_Mask,				"ai_stats",				&psAI_Flags,	aiStats);
	CMD3(CCC_Mask,				"ai_dbg_destroy",		&psAI_Flags,	aiDestroy);
	CMD3(CCC_Mask,				"ai_dbg_serialize",		&psAI_Flags,	aiSerialize);
	CMD3(CCC_Mask,				"ai_dbg_dialogs",		&psAI_Flags,	aiDialogs);
	CMD3(CCC_Mask,				"ai_dbg_infoportion",	&psAI_Flags,	aiInfoPortion);

	CMD3(CCC_Mask,				"ai_draw_game_graph",				&psAI_Flags,	aiDrawGameGraph				);
	CMD3(CCC_Mask,				"ai_draw_game_graph_stalkers",		&psAI_Flags,	aiDrawGameGraphStalkers		);
	CMD3(CCC_Mask,				"ai_draw_game_graph_objects",		&psAI_Flags,	aiDrawGameGraphObjects		);

	CMD3(CCC_Mask,				"ai_nil_object_access",	&psAI_Flags,	aiNilObjectAccess);

	CMD3(CCC_Mask,				"ai_draw_visibility_rays",	&psAI_Flags,	aiDrawVisibilityRays);
	CMD3(CCC_Mask,				"ai_animation_stats",		&psAI_Flags,	aiAnimationStats);

#ifdef DEBUG_MEMORY_MANAGER
	CMD3(CCC_Mask,				"debug_on_frame_gather_stats",				&psAI_Flags,	aiDebugOnFrameAllocs);
	CMD4(CCC_Float,				"debug_on_frame_gather_stats_frequency",	&debug_on_frame_gather_stats_frequency, 0.f, 1.f);
	CMD1(CCC_MemAllocShowStats,	"debug_on_frame_show_stats");
	CMD1(CCC_MemAllocClearStats,"debug_on_frame_clear_stats");
#endif // DEBUG_MEMORY_MANAGER

	CMD1(CCC_DumpModelBones,	"debug_dump_model_bones");

	CMD1(CCC_DrawGameGraphAll,		"ai_draw_game_graph_all");
	CMD1(CCC_DrawGameGraphCurrent,	"ai_draw_game_graph_current_level");
	CMD1(CCC_DrawGameGraphLevel,	"ai_draw_game_graph_level");

	CMD4(CCC_Integer,			"ai_dbg_inactive_time",	&g_AI_inactive_time, 0, 1000000);
	
	CMD1(CCC_DebugNode,			"ai_dbg_node");
	CMD1(CCC_ScriptDbg,			"script_debug_break");
	CMD1(CCC_ScriptDbg,			"script_debug_stop");
	CMD1(CCC_ScriptDbg,			"script_debug_restart");
	
	CMD1(CCC_ShowMonsterInfo,	"ai_monster_info");
	CMD1(CCC_DebugFonts,		"debug_fonts");
	CMD1(CCC_TuneAttachableItem,"dbg_adjust_attachable_item");

	CMD1(CCC_ShowAnimationStats,"ai_show_animation_stats");
#endif // DEBUG
	
#ifndef MASTER_GOLD
	CMD3(CCC_Mask,				"ai_ignore_actor",		&psAI_Flags,	aiIgnoreActor);
#endif // MASTER_GOLD

	// Physics
	CMD1(CCC_PHFps,				"ph_frequency"																					);
	CMD1(CCC_PHIterations,		"ph_iterations"																					);

#ifdef DEBUG
	CMD1(CCC_PHGravity,			"ph_gravity"																					);
	CMD4(CCC_FloatBlock,		"ph_timefactor",				&phTimefactor				,			0.0001f	,1000.f			);
	CMD4(CCC_FloatBlock,		"ph_break_common_factor",		&phBreakCommonFactor		,			0.f		,1000000000.f	);
	CMD4(CCC_FloatBlock,		"ph_rigid_break_weapon_factor",	&phRigidBreakWeaponFactor	,			0.f		,1000000000.f	);
	CMD4(CCC_Integer,			"ph_tri_clear_disable_count",	&ph_tri_clear_disable_count	,			0,		255				);
	CMD4(CCC_FloatBlock,		"ph_tri_query_ex_aabb_rate",	&ph_tri_query_ex_aabb_rate	,			1.01f	,3.f			);
#endif // DEBUG

	CMD3(CCC_Mask,		"g_autopickup",			&psActorFlags,	AF_AUTOPICKUP);


#ifdef DEBUG
	CMD1(CCC_LuaHelp,				"lua_help");
	CMD1(CCC_ShowSmartCastStats,	"show_smart_cast_stats");
	CMD1(CCC_ClearSmartCastStats,	"clear_smart_cast_stats");

	CMD3(CCC_Mask,		"dbg_draw_actor_alive",		&dbg_net_Draw_Flags,	dbg_draw_actor_alive);
	CMD3(CCC_Mask,		"dbg_draw_actor_dead",		&dbg_net_Draw_Flags,	dbg_draw_actor_dead );
	CMD3(CCC_Mask,		"dbg_draw_customzone",		&dbg_net_Draw_Flags,	dbg_draw_customzone );
	CMD3(CCC_Mask,		"dbg_draw_lchangers",		&dbg_net_Draw_Flags,	dbg_draw_lchangers );
	CMD3(CCC_Mask,		"dbg_draw_teamzone",		&dbg_net_Draw_Flags,	dbg_draw_teamzone);
	CMD3(CCC_Mask,		"dbg_draw_invitem",			&dbg_net_Draw_Flags,	dbg_draw_invitem);
	CMD3(CCC_Mask,		"dbg_draw_actor_phys",		&dbg_net_Draw_Flags,	dbg_draw_actor_phys);
	CMD3(CCC_Mask,		"dbg_draw_customdetector",	&dbg_net_Draw_Flags,	dbg_draw_customdetector);
	CMD3(CCC_Mask,		"dbg_destroy",				&dbg_net_Draw_Flags,	dbg_destroy);
	CMD3(CCC_Mask,		"dbg_draw_autopickupbox",	&dbg_net_Draw_Flags,	dbg_draw_autopickupbox);
	CMD3(CCC_Mask,		"dbg_draw_rp",				&dbg_net_Draw_Flags,	dbg_draw_rp);
	CMD3(CCC_Mask,		"dbg_draw_climbable",		&dbg_net_Draw_Flags,	dbg_draw_climbable);
	CMD3(CCC_Mask,		"dbg_draw_skeleton",		&dbg_net_Draw_Flags,	dbg_draw_skeleton);


	CMD3(CCC_Mask,		"dbg_draw_ph_contacts",			&ph_dbg_draw_mask,	phDbgDrawContacts);
	CMD3(CCC_Mask,		"dbg_draw_ph_enabled_aabbs",	&ph_dbg_draw_mask,	phDbgDrawEnabledAABBS);
	CMD3(CCC_Mask,		"dbg_draw_ph_intersected_tries",&ph_dbg_draw_mask,	phDBgDrawIntersectedTries);
	CMD3(CCC_Mask,		"dbg_draw_ph_saved_tries",		&ph_dbg_draw_mask,	phDbgDrawSavedTries);
	CMD3(CCC_Mask,		"dbg_draw_ph_tri_trace",		&ph_dbg_draw_mask,	phDbgDrawTriTrace);
	CMD3(CCC_Mask,		"dbg_draw_ph_positive_tries",	&ph_dbg_draw_mask,	phDBgDrawPositiveTries);
	CMD3(CCC_Mask,		"dbg_draw_ph_negative_tries",	&ph_dbg_draw_mask,	phDBgDrawNegativeTries);
	CMD3(CCC_Mask,		"dbg_draw_ph_tri_test_aabb",	&ph_dbg_draw_mask,	phDbgDrawTriTestAABB);
	CMD3(CCC_Mask,		"dbg_draw_ph_tries_changes_sign",&ph_dbg_draw_mask,	phDBgDrawTriesChangesSign);
	CMD3(CCC_Mask,		"dbg_draw_ph_tri_point"			,&ph_dbg_draw_mask,	phDbgDrawTriPoint);
	CMD3(CCC_Mask,		"dbg_draw_ph_explosion_position",&ph_dbg_draw_mask,	phDbgDrawExplosionPos);
	CMD3(CCC_Mask,		"dbg_draw_ph_statistics"		,&ph_dbg_draw_mask,	phDbgDrawObjectStatistics);
	CMD3(CCC_Mask,		"dbg_draw_ph_mass_centres"		,&ph_dbg_draw_mask,	phDbgDrawMassCenters);
	CMD3(CCC_Mask,		"dbg_draw_ph_death_boxes"		,&ph_dbg_draw_mask,	phDbgDrawDeathActivationBox);
	CMD3(CCC_Mask,		"dbg_draw_ph_hit_app_pos"		,&ph_dbg_draw_mask,	phHitApplicationPoints);
	CMD3(CCC_Mask,		"dbg_draw_ph_cashed_tries_stats",&ph_dbg_draw_mask,	phDbgDrawCashedTriesStat);
	CMD3(CCC_Mask,		"dbg_draw_ph_car_dynamics"		,&ph_dbg_draw_mask,	phDbgDrawCarDynamics);
	CMD3(CCC_Mask,		"dbg_draw_ph_car_plots"			,&ph_dbg_draw_mask,	phDbgDrawCarPlots);
	CMD3(CCC_Mask,		"dbg_ph_ladder"					,&ph_dbg_draw_mask,	phDbgLadder);
	CMD3(CCC_Mask,		"dbg_draw_ph_explosions"		,&ph_dbg_draw_mask,	phDbgDrawExplosions);
	CMD3(CCC_Mask,		"dbg_draw_car_plots_all_trans"	,&ph_dbg_draw_mask,	phDbgDrawCarAllTrnsm);
	CMD3(CCC_Mask,		"dbg_draw_ph_zbuffer_disable"	,&ph_dbg_draw_mask,	phDbgDrawZDisable);
	CMD3(CCC_Mask,		"dbg_ph_obj_collision_damage"	,&ph_dbg_draw_mask,	phDbgDispObjCollisionDammage);
	CMD_RADIOGROUPMASK2("dbg_ph_ai_always_phmove",&ph_dbg_draw_mask,phDbgAlwaysUseAiPhMove,"dbg_ph_ai_never_phmove",&ph_dbg_draw_mask,phDbgNeverUseAiPhMove);
	CMD3(CCC_Mask,		"dbg_ph_ik"						,&ph_dbg_draw_mask,	phDbgIK);
	CMD3(CCC_Mask,		"dbg_ph_ik_off"					,&ph_dbg_draw_mask1,phDbgIKOff);
	CMD3(CCC_Mask,		"dbg_draw_ph_ik_goal"			,&ph_dbg_draw_mask,	phDbgDrawIKGoal);
	CMD3(CCC_Mask,		"dbg_ph_ik_limits"				,&ph_dbg_draw_mask,	phDbgIKLimits);
	CMD3(CCC_Mask,		"dbg_ph_character_control"		,&ph_dbg_draw_mask,	phDbgCharacterControl);
	CMD3(CCC_Mask,		"dbg_draw_ph_ray_motions"		,&ph_dbg_draw_mask,	phDbgDrawRayMotions);
	CMD4(CCC_Float,		"dbg_ph_vel_collid_damage_to_display",&dbg_vel_collid_damage_to_display,	0.f, 1000.f);
	CMD4(CCC_DbgBullets,"dbg_draw_bullet_hit",			&g_bDrawBulletHit,	0, 1)	;
	CMD1(CCC_DbgPhTrackObj,"dbg_track_obj");
	CMD3(CCC_Mask,		"dbg_ph_actor_restriction"		,&ph_dbg_draw_mask1,ph_m1_DbgActorRestriction);
	CMD3(CCC_Mask,		"dbg_draw_ph_hit_anims"			,&ph_dbg_draw_mask1,phDbgHitAnims);
	CMD3(CCC_Mask,		"dbg_draw_ph_ik_limits"			,&ph_dbg_draw_mask1,phDbgDrawIKLimits);
#endif



#ifdef DEBUG
	CMD4(CCC_Integer,	"string_table_error_msg",	&CStringTable::m_bWriteErrorsToLog,	0,	1);

	CMD1(CCC_DumpInfos,				"dump_infos");
	CMD1(CCC_DumpMap,				"dump_map");
	CMD1(CCC_DumpCreatures,			"dump_creatures");

#endif

	CMD3(CCC_Mask,			"cl_dynamiccrosshair",	&psHUD_Flags,	HUD_CROSSHAIR_DYNAMIC);
	CMD1(CCC_MainMenu,		"main_menu"				);

#ifndef MASTER_GOLD
	CMD1(CCC_StartTimeSingle,	"start_time_single");
	CMD4(CCC_TimeFactorSingle,	"time_factor_single", &g_fTimeFactor, 0.f,flt_max);
#endif // MASTER_GOLD


	g_uCommonFlags.zero();
	g_uCommonFlags.set(flAiUseTorchDynamicLights, TRUE);

	CMD3(CCC_Mask,		"ai_use_torch_dynamic_lights",	&g_uCommonFlags, flAiUseTorchDynamicLights);


#ifndef MASTER_GOLD
	CMD4(CCC_Vector3,		"psp_cam_offset",				&CCameraLook2::m_cam_offset, Fvector().set(-1000,-1000,-1000),Fvector().set(1000,1000,1000));
#endif // MASTER_GOLD

	CMD1(CCC_GSCheckForUpdates, "check_for_updates");
#ifdef DEBUG
	CMD1(CCC_DumpObjects,							"dump_all_objects");
	CMD3(CCC_String, "stalker_death_anim", dbg_stalker_death_anim, 32);
	CMD4(CCC_Integer, "death_anim_velocity", &b_death_anim_velocity, FALSE,	TRUE );
	CMD4(CCC_Integer,	"show_wnd_rect",				&g_show_wnd_rect, 0, 1);
	CMD4(CCC_Integer,	"show_wnd_rect_all",			&g_show_wnd_rect2, 0, 1);
	CMD1(CCC_Crash,		"crash"						);
	CMD4(CCC_Integer,		"dbg_show_ani_info",	&g_ShowAnimationInfo,	0, 1)	;
	CMD4(CCC_Integer,		"dbg_dump_physics_step", &g_bDebugDumpPhysicsStep, 0, 1);
#endif
	*g_last_saved_game	= 0;

	CMD4(CCC_Integer,		"keypress_on_start",		&g_keypress_on_start, 0, 1);

	CMD4(CCC_Integer,		"g_sleep_time",				&psActorSleepTime, 1, 24);
	// AXR: New Commands

	if (bDeveloperMode)
	{
		CMD1(CCC_Spawn,			"g_spawn");
		CMD1(CCC_Spawn_to_inv,	"g_spawn_to_inventory");
		CMD1(CCC_Giveinfo,		"g_info");
		CMD1(CCC_Disinfo,		"d_info");
		CMD1(CCC_GiveTask,		"g_task");
		CMD1(CCC_GiveMoney,		"g_money");
		CMD1(CCC_KillEntity,	"kill");
		CMD1(CCC_ReloadSystemLtx, "reload_system_ltx");
		CMD1(CCC_ReloadAdvancedXRayCfg, "reload_axr_cfg");
		CMD1(CCC_ReloadWeather, "reload_weather");
		CMD3(CCC_Mask,			"g_god",					&psActorFlags,				AF_GODMODE);
		CMD3(CCC_Mask,			"g_unlimitedammo",			&psActorFlags,				AF_UNLIMITEDAMMO);
		CMD1(CCC_SetWeather,	"set_weather");
		CMD1(CCC_JumpToLevel,	"jump_to_level");
		CMD1(CCC_Script,		"run_script");
		CMD1(CCC_ScriptCommand,	"run_string");
		CMD1(CCC_TimeFactor,	"time_factor");
		CMD1(DumpTxrsForPrefetching, "ui_textures_for_prefetching");//Prints the list of UI textures, which caused stutterings during game
		CMD4(CCC_Integer,		"dbg_show_material_info",	&g_dbgShowMaterialInfo,		0, 1);
	}

	// adjust mode support
	CMD4(CCC_Integer,			"hud_adjust_mode",			&hud_adj_mode,		0, 7);
	//CMD4(CCC_Integer,			"hud_adjust_item_index",	&g_bHudAdjustItemIdx,	0, 1);
	//CMD4(CCC_Float,				"hud_adjust_delta_value",	&g_bHudAdjustDeltaPos,	0.0005f, 1.f);
	//CMD4(CCC_Float,				"hud_adjust_delta_rot",		&g_bHudAdjustDeltaRot,	0.0005f, 10.f);

	CMD3(CCC_Mask,			"ph_corpse_collision",		&psActorFlags,				AF_COLLISION);
	CMD3(CCC_Mask,			"g_crouch_toggle",			&psActorFlags,				AF_CROUCH_TOGGLE);

	//M.F.S. Crosshair Type
	CMD3(CCC_Token,			"g_crosshair_type",			&crosshair_type,			crosshair_type_token);

	CMD4(CCC_Integer,		"quick_save_counter",		&quick_save_counter,		0, 25);
	CMD3(CCC_UiHud_Mode,	"hud_type",					&ui_hud_type,				qhud_type_token);
	CMD1(CCC_DebugFonts, "debug_fonts");
	CMD4(CCC_Integer,		"g_advanced_crosshair",		&g_advanced_crosshair,		0, 1);
	CMD3(CCC_Token,			"g_death_cam_mode",			&death_camera_mode,			death_camera_mode_token);

	CMD1(CCC_GameLanguage,	"g_language");
	//Custom commands for scripts

	i_script_cmd_name.clear();
	b_script_cmd_name.clear();

	for (int i = 0; i < I_SCRIPT_CMDS_COUNT; ++i)
	{
		xr_string buff = "i_script_cmd_";
		buff += std::to_string(i).c_str();
		i_script_cmd_name.emplace_back(std::move(buff));
		const xr_string& cmd = i_script_cmd_name.back();
		CMD4_X(CCC_Integer, cmd.c_str(), &i_script_cmd[i], 0, 64);
	}

	for (int i = 0; i < B_SCRIPT_CMDS_COUNT; ++i)
	{
		xr_string buff = "b_script_cmd_";
		buff += std::to_string(i).c_str();
		b_script_cmd_name.emplace_back(std::move(buff));
		const xr_string& cmd = b_script_cmd_name.back();
		CMD4_X(CCC_Integer, cmd.c_str(), &b_script_cmd[i], 0, 1);
	}

	//Custom commands for scripts end

	//register_mp_console_commands					();
}
