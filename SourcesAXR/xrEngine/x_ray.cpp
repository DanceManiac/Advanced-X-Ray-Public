//-----------------------------------------------------------------------------
// File: x_ray.cpp
//
// Programmers:
//	Oles		- Oles Shishkovtsov
//	AlexMX		- Alexander Maksimchuk
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "igame_level.h"
#include "igame_persistent.h"
#include "ILoadingScreen.h"

#include "dedicated_server_only.h"
#include "no_single.h"
#include "../xrNetServer/NET_AuthCheck.h"

#include "xr_input.h"
#include "xr_ioconsole.h"
#include "x_ray.h"
#include "std_classes.h"
#include "GameFont.h"
#include "resource.h"
#include "LightAnimLibrary.h"
#include "../xrcdb/ispatial.h"
#include "CopyProtection.h"
#include "Text_Console.h"
#include <process.h>
#include <locale.h>

#include "xrSash.h"

#include "securom_api.h"
#include "Rain.h"
#include "..\Layers\xrAPI\xrGameManager.h"
#include "Render.h"

#include "DiscordRichPresense.h"
#include <atlimage.h>

// Always request high performance GPU
extern "C"
{
	// https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus.htm
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; // NVIDIA Optimus

	// https://gpuopen.com/amdpowerxpressrequesthighperformance/
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; // PowerXpress or Hybrid Graphics
}

//---------------------------------------------------------------------
ENGINE_API CInifile* pGameIni		= NULL;
XRAPI_API extern EGamePath GCurrentGame;
BOOL	g_bIntroFinished			= FALSE;
extern	void	Intro				( void* fn );
extern	void	Intro_DSHOW			( void* fn );
extern	int PASCAL IntroDSHOW_wnd	(HINSTANCE hInstC, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow);
//int		max_load_stage = 0;

const TCHAR* c_szSplashClass = _T("SplashWindow");

// computing build id
XRCORE_API	LPCSTR	build_date;
XRCORE_API	u32		build_id;

#ifdef MASTER_GOLD
#	define NO_MULTI_INSTANCES
#endif // #ifdef MASTER_GOLD

ENGINE_API bool CallOfPripyatMode = false;
ENGINE_API bool ClearSkyMode = false;
ENGINE_API bool bDeveloperMode = false;
ENGINE_API bool bWinterMode = false;
ENGINE_API bool bDofWeather = false;
ENGINE_API bool bLowlandFogWeather = false;
ENGINE_API bool bWeatherColorDragging = false;
ENGINE_API Fvector4 ps_ssfx_wpn_dof_1 = { .0f, .0f, .0f, .0f };
ENGINE_API float ps_ssfx_wpn_dof_2 = 1.0f;
ENGINE_API int ps_rs_loading_stages = 0;

static LPSTR month_id[12] = {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static int days_in_month[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int start_day	= 31;	// 31
static int start_month	= 1;	// January
static int start_year	= 1999;	// 1999

// binary hash, mainly for copy-protection

#ifndef DEDICATED_SERVER

#include "../xrGameSpy/gamespy/md5c.c"
#include <ctype.h>
#include <wincodec.h>
#include <thread>

#define DEFAULT_MODULE_HASH "3CAABCFCFF6F3A810019C6A72180F166"
static char szEngineHash[33] = DEFAULT_MODULE_HASH;

PROTECT_API char * ComputeModuleHash( char * pszHash )
{
	SECUROM_MARKER_HIGH_SECURITY_ON(3)

	char szModuleFileName[ MAX_PATH ];
	HANDLE hModuleHandle = NULL , hFileMapping = NULL;
	LPVOID lpvMapping = NULL;
	MEMORY_BASIC_INFORMATION MemoryBasicInformation;

	if ( ! GetModuleFileName( NULL , szModuleFileName , MAX_PATH ) )
		return pszHash;

	hModuleHandle = CreateFile( szModuleFileName , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , 0 , NULL );

	if ( hModuleHandle == INVALID_HANDLE_VALUE )
		return pszHash;

	hFileMapping = CreateFileMapping( hModuleHandle , NULL , PAGE_READONLY , 0 , 0 , NULL );

	if ( hFileMapping == NULL ) {
		CloseHandle( hModuleHandle );
		return pszHash;
	}

	lpvMapping = MapViewOfFile( hFileMapping , FILE_MAP_READ , 0 , 0 , 0 );

	if ( lpvMapping == NULL ) {
		CloseHandle( hFileMapping );
		CloseHandle( hModuleHandle );
		return pszHash;
	}

	ZeroMemory( &MemoryBasicInformation , sizeof( MEMORY_BASIC_INFORMATION ) );

	VirtualQuery( lpvMapping , &MemoryBasicInformation , sizeof( MEMORY_BASIC_INFORMATION ) );

	if ( MemoryBasicInformation.RegionSize ) {
		char szHash[33];
		MD5Digest( ( unsigned char *)lpvMapping , (unsigned int) MemoryBasicInformation.RegionSize , szHash );
		MD5Digest( ( unsigned char *)szHash , 32 , pszHash );
		for ( int i = 0 ; i < 32 ; ++i )
			pszHash[ i ] = (char)toupper( pszHash[ i ] );
	}

	UnmapViewOfFile( lpvMapping );
	CloseHandle( hFileMapping );
	CloseHandle( hModuleHandle );

	SECUROM_MARKER_HIGH_SECURITY_OFF(3)

	return pszHash;
}
#endif // DEDICATED_SERVER

void compute_build_id	()
{
	build_date			= __DATE__;

	int					days;
	int					months = 0;
	int					years;
	string16			month;
	string256			buffer;
	xr_strcpy				(buffer,__DATE__);
	sscanf				(buffer,"%s %d %d",month,&days,&years);

	for (int i=0; i<12; i++) {
		if (_stricmp(month_id[i],month))
			continue;

		months			= i;
		break;
	}

	build_id			= (years - start_year)*365 + days - start_day;

	for (int i=0; i<months; ++i)
		build_id		+= days_in_month[i];

	for (int i=0; i<start_month-1; ++i)
		build_id		-= days_in_month[i];
}
//---------------------------------------------------------------------
// 2446363
// umbt@ukr.net
//////////////////////////////////////////////////////////////////////////
struct _SoundProcessor	: public pureFrame
{
	virtual void	_BCL	OnFrame	( )
	{
		//Msg							("------------- sound: %d [%3.2f,%3.2f,%3.2f]",u32(Device.dwFrame),VPUSH(Device.vCameraPosition));
		Device.Statistic->Sound.Begin();
		::Sound->update				(Device.vCameraPosition,Device.vCameraDirection,Device.vCameraTop);
		Device.Statistic->Sound.End	();
	}
}	SoundProcessor;

//////////////////////////////////////////////////////////////////////////
// global variables
ENGINE_API	CApplication*	pApp			= NULL;
static		HWND			logoWindow		= NULL;

			int				doLauncher		();
			void			doBenchmark		(LPCSTR name);
ENGINE_API	bool			g_bBenchmark	= false;
string512	g_sBenchmarkName;


ENGINE_API	string512		g_sLaunchOnExit_params;
ENGINE_API	string512		g_sLaunchOnExit_app;
ENGINE_API	string_path		g_sLaunchWorkingFolder;
// -------------------------------------------
// startup point
void InitEngine		()
{
	Engine.Initialize			( );
	while (!g_bIntroFinished)	Sleep	(100);
	Device.Initialize			( );
	CheckCopyProtection			( );
}

struct path_excluder_predicate
{
	explicit path_excluder_predicate(xr_auth_strings_t const * ignore) :
		m_ignore(ignore)
	{
	}
	bool xr_stdcall is_allow_include(LPCSTR path)
	{
		if (!m_ignore)
			return true;
		
		return allow_to_include_path(*m_ignore, path);
	}
	xr_auth_strings_t const *	m_ignore;
};

template <typename T>
void InitConfig(T& config, pcstr name, bool fatal = true,
	bool readOnly = true, bool loadAtStart = true, bool saveAtEnd = true,
	u32 sectCount = 0, const CInifile::allow_include_func_t& allowIncludeFunc = nullptr)
{
	string_path fname;
	FS.update_path(fname, "$game_config$", name);
	config = xr_new<CInifile>(fname, readOnly, loadAtStart, saveAtEnd, sectCount, allowIncludeFunc);

	CHECK_OR_EXIT(config->section_count() || !fatal,
		make_string("Cannot find file %s.\nReinstalling application may fix this problem.", fname));
}

PROTECT_API void InitSettings()
{
	Msg("# Compute Module Hash: %s", ComputeModuleHash(szEngineHash));

	string_path fname;
	FS.update_path(fname, "$game_config$", "system.ltx");
	pSettings = xr_new<CInifile>(fname, TRUE);
	CHECK_OR_EXIT(0 != pSettings->section_count(), make_string("Cannot find file %s.\nReinstalling application may fix this problem.", fname));

	xr_auth_strings_t tmp_ignore_pathes, tmp_check_pathes;
	fill_auth_check_params(tmp_ignore_pathes, tmp_check_pathes);

	path_excluder_predicate tmp_excluder(&tmp_ignore_pathes);
	CInifile::allow_include_func_t includeFilter;
	includeFilter.bind(&tmp_excluder, &path_excluder_predicate::is_allow_include);

	InitConfig(pSettings, "system.ltx");
	InitConfig(pAdvancedSettings, "AdvancedXRay.ltx");
	InitConfig(pSettingsAuth, "system.ltx", true, true, true, false, 0, includeFilter);
	InitConfig(pGameIni, "game.ltx");

	pcstr EngineMode = READ_IF_EXISTS(pAdvancedSettings, r_string, "global", "engine_mode", "cop");
	bDeveloperMode = READ_IF_EXISTS(pAdvancedSettings, r_bool, "global", "developer_mode", false);
	bWinterMode = READ_IF_EXISTS(pAdvancedSettings, r_bool, "environment", "winter_mode", false);
	bDofWeather = READ_IF_EXISTS(pAdvancedSettings, r_bool, "environment", "weather_dof", false);
	bLowlandFogWeather = READ_IF_EXISTS(pAdvancedSettings, r_bool, "environment", "lowland_fog_from_weather", false);
	bWeatherColorDragging = READ_IF_EXISTS(pAdvancedSettings, r_bool, "environment", "weather_color_dragging", false);

	Msg("# Engine Mode: %s", EngineMode);
	Msg("# Developer Mode: %d", bDeveloperMode);
	Msg("# Winter Mode: %d", bWinterMode);

	if (xr_strcmp("cop", EngineMode) == 0)
	{
		CallOfPripyatMode = true;
		ClearSkyMode = false;
	}
	else if (xr_strcmp("cs", EngineMode) == 0)
	{
		CallOfPripyatMode = false;
		ClearSkyMode = true;
	}
}
PROTECT_API void InitConsole	()
{
	SECUROM_MARKER_SECURITY_ON(5)

#ifdef DEDICATED_SERVER
	{
		Console						= xr_new<CTextConsole>	();		
	}
#else
	//	else
	{
		Console						= xr_new<CConsole>	();
	}
#endif
	Console->Initialize			( );

	xr_strcpy						(Console->ConfigFile,"user.ltx");
	if (strstr(Core.Params,"-ltx ")) {
		string64				c_name;
		sscanf					(strstr(Core.Params,"-ltx ")+5,"%[^ ] ",c_name);
		xr_strcpy					(Console->ConfigFile,c_name);
	}

	SECUROM_MARKER_SECURITY_OFF(5)
}

PROTECT_API void InitInput		()
{
	BOOL bCaptureInput			= !strstr(Core.Params,"-i");

	pInput						= xr_new<CInput>		(bCaptureInput);
}
void destroyInput	()
{
	xr_delete					( pInput		);
}

PROTECT_API void InitSound1		()
{
	CSound_manager_interface::_create				(0);
}

PROTECT_API void InitSound2		()
{
	CSound_manager_interface::_create				(1);
}

void destroySound	()
{
	CSound_manager_interface::_destroy				( );
}

void destroySettings()
{
	CInifile** s				= (CInifile**)(&pSettings);
	xr_delete(*s);
	CInifile** sa = (CInifile**)(&pSettingsAuth);
	xr_delete(*sa);
	xr_delete					( pGameIni		);
}

void destroyConsole	()
{
	Console->Execute			("cfg_save");
	Console->Destroy			();
	xr_delete					(Console);
}

void destroyEngine	()
{
	Device.Destroy				( );
	Engine.Destroy				( );
}

void execUserScript				( )
{
	Console->Execute			("default_controls");
	Console->ExecuteScript		(Console->ConfigFile);
}

void slowdownthread	( void* )
{
//	Sleep		(30*1000);
	for (;;)	{
		if (Device.Statistic->fFPS<30) Sleep(1);
		if (Device.mt_bMustExit)	return;
		if (0==pSettings)			return;
		if (0==Console)				return;
		if (0==pInput)				return;
		if (0==pApp)				return;
	}
}
void CheckPrivilegySlowdown		( )
{
#ifdef DEBUG
	if	(strstr(Core.Params,"-slowdown"))	{
		thread_spawn(slowdownthread,"slowdown",0,0);
	}
	if	(strstr(Core.Params,"-slowdown2x"))	{
		thread_spawn(slowdownthread,"slowdown",0,0);
		thread_spawn(slowdownthread,"slowdown",0,0);
	}
#endif // DEBUG
}

void Startup()
{
	InitSound1		();
	execUserScript	();
	InitSound2		();

	// ...command line for auto start
	{
		LPCSTR	pStartup			= strstr				(Core.Params,"-start ");
		if (pStartup)				Console->Execute		(pStartup+1);
	}
	{
		LPCSTR	pStartup			= strstr				(Core.Params,"-load ");
		if (pStartup)				Console->Execute		(pStartup+1);
	}

	// Initialize APP
//#ifndef DEDICATED_SERVER
	ShowWindow( Device.m_hWnd , SW_SHOWNORMAL );
	Device.Create				( );
//#endif
	LALib.OnCreate				( );
	pApp						= xr_new<CApplication>	();
	g_pGamePersistent			= (IGame_Persistent*)	NEW_INSTANCE (CLSID_GAME_PERSISTANT);
	g_SpatialSpace				= xr_new<ISpatial_DB>	();
	g_SpatialSpacePhysic		= xr_new<ISpatial_DB>	();
	
	g_discord.Initialize();

	// Destroy LOGO
	DestroyWindow				(logoWindow);
	logoWindow					= NULL;

	// Main cycle
	CheckCopyProtection			( );
	Memory.mem_usage();
	Device.Run					( );

	// Destroy APP
	xr_delete(g_SpatialSpacePhysic);
	xr_delete(g_SpatialSpace);
	DEL_INSTANCE(g_pGamePersistent);
	xr_delete(pApp);
	Engine.Event.Dump();

	g_discord.Shutdown();

	// Destroying
	if (!g_bBenchmark && !g_SASH.IsRunning())
		destroySettings();

	LALib.OnDestroy();

	if (!g_bBenchmark && !g_SASH.IsRunning())
		destroyConsole();
	else
		Console->Destroy();

	destroySound();
	destroyEngine();
}

IStream* CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType)
{
	IStream* ipStream = NULL;

	HRSRC hrsrc = FindResource(NULL, lpName, lpType);
	if (hrsrc == NULL)
		goto Return;

	DWORD dwResourceSize = SizeofResource(NULL, hrsrc);
	HGLOBAL hglbImage = LoadResource(NULL, hrsrc);
	if (hglbImage == NULL)
		goto Return;

	LPVOID pvSourceResourceData = LockResource(hglbImage);
	if (pvSourceResourceData == NULL)
		goto Return;

	HGLOBAL hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
	if (hgblResourceData == NULL)
		goto Return;

	LPVOID pvResourceData = GlobalLock(hgblResourceData);

	if (pvResourceData == NULL)
		goto FreeData;

	CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);

	GlobalUnlock(hgblResourceData);

	if (SUCCEEDED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream)))
		goto Return;

FreeData:
	GlobalFree(hgblResourceData);

Return:
	return ipStream;
}

void RegisterWindowClass(HINSTANCE hInst)
{
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = c_szSplashClass;
	RegisterClass(&wc);
}

HWND CreateSplashWindow(HINSTANCE hInst)
{
	HWND hwndOwner = CreateWindow(c_szSplashClass, NULL, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hInst, NULL);
	return CreateWindowEx(WS_EX_LAYERED, c_szSplashClass, NULL, WS_POPUP | WS_VISIBLE,
		0, 0, 0, 0, hwndOwner, NULL, hInst, NULL);
}

HWND WINAPI ShowSplash(HINSTANCE hInstance, int nCmdShow)
{
	MSG msg;
	HWND hWnd;

	//image
	CImage img;                             //объект изображения

	WCHAR path[MAX_PATH];

	GetModuleFileNameW(NULL, path, MAX_PATH);
	std::wstring ws(path);

	std::string splash_path(ws.begin(), ws.end());
	splash_path = splash_path.erase(splash_path.find_last_of('\\'), splash_path.size() - 1);
	splash_path += "\\splash.png";

	img.Load(splash_path.c_str());              //загрузка сплеша

	int splashWidth = img.GetWidth();            //фиксируем ширину картинки
	int splashHeight = img.GetHeight();            //фиксируем высоту картинки

	if (splashWidth == 0 || splashHeight == 0)  //если картинки нет на диске, то грузим из ресурсов
	{
		img.Destroy();
		img.Load(CreateStreamOnResource(MAKEINTRESOURCE(IDB_PNG1), _T("PNG")));//загружаем сплеш
		splashWidth = img.GetWidth();
		splashHeight = img.GetHeight();
	}

	//float temp_x_size = 860.f;
	//float temp_y_size = 461.f;
	int scr_x = GetSystemMetrics(SM_CXSCREEN);
	int scr_y = GetSystemMetrics(SM_CYSCREEN);

	int pos_x = (scr_x / 2) - (splashWidth / 2);
	int pos_y = (scr_y / 2) - (splashHeight / 2);

	//if (!RegClass(SplashProc, szClass, COLOR_WINDOW)) return FALSE;
	hWnd = CreateSplashWindow(hInstance);

	if (!hWnd) return FALSE;

	HDC hdcScreen = GetDC(NULL);
	HDC hDC = CreateCompatibleDC(hdcScreen);

	HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, splashWidth, splashHeight);
	HBITMAP hBmpOld = (HBITMAP)SelectObject(hDC, hBmp);
	//рисуем картиночку
	for (int i = 0; i < img.GetWidth(); i++)
	{
		for (int j = 0; j < img.GetHeight(); j++)
		{
			BYTE* ptr = (BYTE*)img.GetPixelAddress(i, j);
			ptr[0] = ((ptr[0] * ptr[3]) + 127) / 255;
			ptr[1] = ((ptr[1] * ptr[3]) + 127) / 255;
			ptr[2] = ((ptr[2] * ptr[3]) + 127) / 255;
		}
	}

	img.AlphaBlend(hDC, 0, 0, splashWidth, splashHeight, 0, 0, splashWidth, splashHeight);

	//alpha
	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	POINT ptPos = { 0, 0 };
	SIZE sizeWnd = { splashWidth, splashHeight };
	POINT ptSrc = { 0, 0 };
	HWND hDT = GetDesktopWindow();

	if (hDT)
	{
		RECT rcDT;
		GetWindowRect(hDT, &rcDT);
		ptPos.x = (rcDT.right - splashWidth) / 2;
		ptPos.y = (rcDT.bottom - splashHeight) / 2;
	}

	UpdateLayeredWindow(hWnd, hdcScreen, &ptPos, &sizeWnd, hDC, &ptSrc, 0, &blend, ULW_ALPHA);

	return hWnd;
}

void SetSplashImage(HWND hwndSplash, HBITMAP hbmpSplash)
{
	// get the size of the bitmap

	BITMAP bm;
	GetObject(hbmpSplash, sizeof(bm), &bm);
	SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

	// get the primary monitor's info

	POINT ptZero = { 0 };
	HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorinfo = { 0 };
	monitorinfo.cbSize = sizeof(monitorinfo);
	GetMonitorInfo(hmonPrimary, &monitorinfo);

	// center the splash screen in the middle of the primary work area

	const RECT& rcWork = monitorinfo.rcWork;
	POINT ptOrigin;
	ptOrigin.x = rcWork.left + (rcWork.right - rcWork.left - sizeSplash.cx) / 2;
	ptOrigin.y = rcWork.top + (rcWork.bottom - rcWork.top - sizeSplash.cy) / 2;

	// create a memory DC holding the splash bitmap

	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpSplash);

	// use the source image's alpha channel for blending

	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	// paint the window (in the right location) with the alpha-blended bitmap

	UpdateLayeredWindow(hwndSplash, hdcScreen, &ptOrigin, &sizeSplash,
		hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

	// delete temporary objects

	SelectObject(hdcMem, hbmpOld);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);
}

static BOOL CALLBACK logDlgProc( HWND hw, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg ){
		case WM_DESTROY:
			break;
		case WM_CLOSE:
			DestroyWindow( hw );
			break;
		case WM_COMMAND:
			if( LOWORD(wp)==IDCANCEL )
				DestroyWindow( hw );
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
/*
void	test_rtc	()
{
	CStatTimer		tMc,tM,tC,tD;
	u32				bytes=0;
	tMc.FrameStart	();
	tM.FrameStart	();
	tC.FrameStart	();
	tD.FrameStart	();
	::Random.seed	(0x12071980);
	for		(u32 test=0; test<10000; test++)
	{
		u32			in_size			= ::Random.randI(1024,256*1024);
		u32			out_size_max	= rtc_csize		(in_size);
		u8*			p_in			= xr_alloc<u8>	(in_size);
		u8*			p_in_tst		= xr_alloc<u8>	(in_size);
		u8*			p_out			= xr_alloc<u8>	(out_size_max);
		for (u32 git=0; git<in_size; git++)			p_in[git] = (u8)::Random.randI	(8);	// garbage
		bytes		+= in_size;

		tMc.Begin	();
		memcpy		(p_in_tst,p_in,in_size);
		tMc.End		();

		tM.Begin	();
		CopyMemory(p_in_tst,p_in,in_size);
		tM.End		();

		tC.Begin	();
		u32			out_size		= rtc_compress	(p_out,out_size_max,p_in,in_size);
		tC.End		();

		tD.Begin	();
		u32			in_size_tst		= rtc_decompress(p_in_tst,in_size,p_out,out_size);
		tD.End		();

		// sanity check
		R_ASSERT	(in_size == in_size_tst);
		for (u32 tit=0; tit<in_size; tit++)			R_ASSERT(p_in[tit] == p_in_tst[tit]);	// garbage

		xr_free		(p_out);
		xr_free		(p_in_tst);
		xr_free		(p_in);
	}
	tMc.FrameEnd	();	float rMc		= 1000.f*(float(bytes)/tMc.result)/(1024.f*1024.f);
	tM.FrameEnd		(); float rM		= 1000.f*(float(bytes)/tM.result)/(1024.f*1024.f);
	tC.FrameEnd		(); float rC		= 1000.f*(float(bytes)/tC.result)/(1024.f*1024.f);
	tD.FrameEnd		(); float rD		= 1000.f*(float(bytes)/tD.result)/(1024.f*1024.f);
	Msg				("* memcpy:        %5.2f M/s (%3.1f%%)",rMc,100.f*rMc/rMc);
	Msg				("* mm-memcpy:     %5.2f M/s (%3.1f%%)",rM,100.f*rM/rMc);
	Msg				("* compression:   %5.2f M/s (%3.1f%%)",rC,100.f*rC/rMc);
	Msg				("* decompression: %5.2f M/s (%3.1f%%)",rD,100.f*rD/rMc);
}
*/
extern void	testbed	(void);

// video
/*
static	HINSTANCE	g_hInstance		;
static	HINSTANCE	g_hPrevInstance	;
static	int			g_nCmdShow		;
void	__cdecl		intro_dshow_x	(void*)
{
	IntroDSHOW_wnd		(g_hInstance,g_hPrevInstance,"GameData\\Stalker_Intro.avi",g_nCmdShow);
	g_bIntroFinished	= TRUE	;
}
*/
#define dwStickyKeysStructSize sizeof( STICKYKEYS )
#define dwFilterKeysStructSize sizeof( FILTERKEYS )
#define dwToggleKeysStructSize sizeof( TOGGLEKEYS )

struct damn_keys_filter {
	BOOL bScreenSaverState;

	// Sticky & Filter & Toggle keys

	STICKYKEYS StickyKeysStruct;
	FILTERKEYS FilterKeysStruct;
	TOGGLEKEYS ToggleKeysStruct;

	DWORD dwStickyKeysFlags;
	DWORD dwFilterKeysFlags;
	DWORD dwToggleKeysFlags;

	damn_keys_filter	()
	{
		// Screen saver stuff

		bScreenSaverState = FALSE;

		// Saveing current state
		SystemParametersInfo( SPI_GETSCREENSAVEACTIVE , 0 , ( PVOID ) &bScreenSaverState , 0 );

		if ( bScreenSaverState )
			// Disable screensaver
			SystemParametersInfo( SPI_SETSCREENSAVEACTIVE , FALSE , NULL , 0 );

		dwStickyKeysFlags = 0;
		dwFilterKeysFlags = 0;
		dwToggleKeysFlags = 0;


		ZeroMemory( &StickyKeysStruct , dwStickyKeysStructSize );
		ZeroMemory( &FilterKeysStruct , dwFilterKeysStructSize );
		ZeroMemory( &ToggleKeysStruct , dwToggleKeysStructSize );

		StickyKeysStruct.cbSize = dwStickyKeysStructSize;
		FilterKeysStruct.cbSize = dwFilterKeysStructSize;
		ToggleKeysStruct.cbSize = dwToggleKeysStructSize;

		// Saving current state
		SystemParametersInfo( SPI_GETSTICKYKEYS , dwStickyKeysStructSize , ( PVOID ) &StickyKeysStruct , 0 );
		SystemParametersInfo( SPI_GETFILTERKEYS , dwFilterKeysStructSize , ( PVOID ) &FilterKeysStruct , 0 );
		SystemParametersInfo( SPI_GETTOGGLEKEYS , dwToggleKeysStructSize , ( PVOID ) &ToggleKeysStruct , 0 );

		if ( StickyKeysStruct.dwFlags & SKF_AVAILABLE ) {
			// Disable StickyKeys feature
			dwStickyKeysFlags = StickyKeysStruct.dwFlags;
			StickyKeysStruct.dwFlags = 0;
			SystemParametersInfo( SPI_SETSTICKYKEYS , dwStickyKeysStructSize , ( PVOID ) &StickyKeysStruct , 0 );
		}

		if ( FilterKeysStruct.dwFlags & FKF_AVAILABLE ) {
			// Disable FilterKeys feature
			dwFilterKeysFlags = FilterKeysStruct.dwFlags;
			FilterKeysStruct.dwFlags = 0;
			SystemParametersInfo( SPI_SETFILTERKEYS , dwFilterKeysStructSize , ( PVOID ) &FilterKeysStruct , 0 );
		}

		if ( ToggleKeysStruct.dwFlags & TKF_AVAILABLE ) {
			// Disable FilterKeys feature
			dwToggleKeysFlags = ToggleKeysStruct.dwFlags;
			ToggleKeysStruct.dwFlags = 0;
			SystemParametersInfo( SPI_SETTOGGLEKEYS , dwToggleKeysStructSize , ( PVOID ) &ToggleKeysStruct , 0 );
		}
	}

	~damn_keys_filter	()
	{
		if ( bScreenSaverState )
			// Restoring screen saver
			SystemParametersInfo( SPI_SETSCREENSAVEACTIVE , TRUE , NULL , 0 );

		if ( dwStickyKeysFlags) {
			// Restore StickyKeys feature
			StickyKeysStruct.dwFlags = dwStickyKeysFlags;
			SystemParametersInfo( SPI_SETSTICKYKEYS , dwStickyKeysStructSize , ( PVOID ) &StickyKeysStruct , 0 );
		}

		if ( dwFilterKeysFlags ) {
			// Restore FilterKeys feature
			FilterKeysStruct.dwFlags = dwFilterKeysFlags;
			SystemParametersInfo( SPI_SETFILTERKEYS , dwFilterKeysStructSize , ( PVOID ) &FilterKeysStruct , 0 );
		}

		if ( dwToggleKeysFlags ) {
			// Restore FilterKeys feature
			ToggleKeysStruct.dwFlags = dwToggleKeysFlags;
			SystemParametersInfo( SPI_SETTOGGLEKEYS , dwToggleKeysStructSize , ( PVOID ) &ToggleKeysStruct , 0 );
		}

	}
};

#undef dwStickyKeysStructSize
#undef dwFilterKeysStructSize
#undef dwToggleKeysStructSize

#include "xr_ioc_cmd.h"

//typedef void DUMMY_STUFF (const void*,const u32&,void*);
//XRCORE_API DUMMY_STUFF	*g_temporary_stuff;

//#define TRIVIAL_ENCRYPTOR_DECODER
//#include "trivial_encryptor.h"

//#define RUSSIAN_BUILD

#if 0
void foo	()
{
	typedef std::map<int,int>	TEST_MAP;
	TEST_MAP					temp;
	temp.insert					(std::make_pair(0,0));
	TEST_MAP::const_iterator	I = temp.upper_bound(2);
	if (I == temp.end())
		OutputDebugString		("end() returned\r\n");
	else
		OutputDebugString		("last element returned\r\n");

	typedef void*	pvoid;

	LPCSTR			path = "d:\\network\\stalker_net2";
	FILE			*f = fopen(path,"rb");
	int				file_handle = _fileno(f);
	u32				buffer_size = _filelength(file_handle);
	pvoid			buffer = xr_malloc(buffer_size);
	size_t			result = fread(buffer,buffer_size,1,f);
	R_ASSERT3		(!buffer_size || (result && (buffer_size >= result)),"Cannot read from file",path);
	fclose			(f);

	u32				compressed_buffer_size = rtc_csize(buffer_size);
	pvoid			compressed_buffer = xr_malloc(compressed_buffer_size);
	u32				compressed_size = rtc_compress(compressed_buffer,compressed_buffer_size,buffer,buffer_size);

	LPCSTR			compressed_path = "d:\\network\\stalker_net2.rtc";
	FILE			*f1 = fopen(compressed_path,"wb");
	fwrite			(compressed_buffer,compressed_size,1,f1);
	fclose			(f1);
}
#endif // 0

ENGINE_API	bool g_dedicated_server	= false;

#ifndef DEDICATED_SERVER
	// forward declaration for Parental Control checks
	BOOL IsPCAccessAllowed(); 
#endif // DEDICATED_SERVER

int APIENTRY WinMain_impl(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     char *    lpCmdLine,
                     int       nCmdShow)
{
#ifdef DEDICATED_SERVER
	Debug._initialize			(true);
#else // DEDICATED_SERVER
	Debug._initialize			(false);
#endif // DEDICATED_SERVER

	if (!IsDebuggerPresent()) {

		HMODULE const kernel32	= LoadLibrary("kernel32.dll");
		R_ASSERT				(kernel32);

		typedef BOOL (__stdcall*HeapSetInformation_type) (HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T);
		HeapSetInformation_type const heap_set_information = 
			(HeapSetInformation_type)GetProcAddress(kernel32, "HeapSetInformation");
		if (heap_set_information) {
			ULONG HeapFragValue	= 2;
#ifdef DEBUG
			BOOL const result	= 
#endif // #ifdef DEBUG
				heap_set_information(
					GetProcessHeap(),
					HeapCompatibilityInformation,
					&HeapFragValue,
					sizeof(HeapFragValue)
				);
			VERIFY2				(result, "can't set process heap low fragmentation");
		}
	}

//	foo();
#ifndef DEDICATED_SERVER

	// Parental Control for Vista and upper
	if ( ! IsPCAccessAllowed() ) {
		MessageBox( NULL , "Access restricted" , "Parental Control" , MB_OK | MB_ICONERROR );
		return 1;
	}

	// Check for another instance
#ifdef NO_MULTI_INSTANCES
	#define STALKER_PRESENCE_MUTEX "Local\\STALKER-COP"
	
	HANDLE hCheckPresenceMutex = INVALID_HANDLE_VALUE;
	hCheckPresenceMutex = OpenMutex( READ_CONTROL , FALSE ,  STALKER_PRESENCE_MUTEX );
	if ( hCheckPresenceMutex == NULL ) {
		// New mutex
		hCheckPresenceMutex = CreateMutex( NULL , FALSE , STALKER_PRESENCE_MUTEX );
		if ( hCheckPresenceMutex == NULL )
			// Shit happens
			return 2;
	} else {
		// Already running
		CloseHandle( hCheckPresenceMutex );
		return 1;
	}
#endif
#else // DEDICATED_SERVER
	g_dedicated_server			= true;
#endif // DEDICATED_SERVER

	// Title window
	//IStream* pStream = CreateStreamOnResource(MAKEINTRESOURCE(IDB_PNG1), _T("PNG"));

	//WICBitmapSource* res = LoadBitmapFromStream(CreateStreamOnResource(MAKEINTRESOURCE(IDB_PNG1), _T("PNG")));

	//HBITMAP img = LoadSplashImage();
	//image.

	RegisterWindowClass(hInstance);
	//logoWindow = CreateSplashWindow(hInstance);
	logoWindow = ShowSplash(hInstance, nCmdShow);

	SendMessage(logoWindow, WM_DESTROY, 0, 0);

	//SendMessageTimeout(logoWindow, WM_DESTROY,3)
	//SetSplashImage(logoWindow, image);

	//logoWindow = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STARTUP), 0, logDlgProc);


	//HWND logoPicture = GetDlgItem(logoWindow, IDC_STATIC_LOGO);
	//HWND logoPicture2 = GetDlgItem(logoWindow, IDD_STARTUP);
	//RECT logoRect;
	//GetWindowRect(logoPicture, &logoRect);

	//SetWindowLong(logoWindow, GetWindowLong(logoWindow, GWL_EXSTYLE), GetWindowLong(logoWindow, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	//SetWindowLong(logoPicture, GetWindowLong(logoPicture, GWL_EXSTYLE), GetWindowLong(logoPicture, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

	//SetLayeredWindowAttributes(logoPicture, 0, 0, LWA_ALPHA);

	/*SetWindowPos(
		logoWindow,
#ifndef DEBUG
		HWND_TOPMOST,
#else
		HWND_NOTOPMOST,
#endif // NDEBUG
		0,
		0,
		logoRect.right - logoRect.left,
		logoRect.bottom - logoRect.top,
		SWP_NOMOVE | SWP_SHOWWINDOW// | SWP_NOSIZE
	);
	UpdateWindow(logoWindow);*/

	// AVI
	g_bIntroFinished			= TRUE;

	g_sLaunchOnExit_app[0]		= NULL;
	g_sLaunchOnExit_params[0]	= NULL;

	LPCSTR						fsgame_ltx_name = "-fsltx ";
	string_path					fsgame = "";
	//MessageBox(0, lpCmdLine, "my cmd string", MB_OK);
	if (strstr(lpCmdLine, fsgame_ltx_name)) {
		int						sz = xr_strlen(fsgame_ltx_name);
		sscanf					(strstr(lpCmdLine,fsgame_ltx_name)+sz,"%[^ ] ",fsgame);
		//MessageBox(0, fsgame, "using fsltx", MB_OK);
	}

//	g_temporary_stuff			= &trivial_encryptor::decode;
	
	compute_build_id			();

	Core._initialize			("xray",NULL, TRUE, fsgame[0] ? fsgame : NULL);

	InitSettings				();

	// Adjust player & computer name for Asian
	if ( pSettings->line_exist( "string_table" , "no_native_input" ) ) {
			xr_strcpy( Core.UserName , sizeof( Core.UserName ) , "Player" );
			xr_strcpy( Core.CompName , sizeof( Core.CompName ) , "Computer" );
	}

	if (ClearSkyMode)
	{
		GCurrentGame = EGamePath::CS_1510;
	}
	else if (CallOfPripyatMode)
	{
		GCurrentGame = EGamePath::COP_1602;
	}

#ifndef DEDICATED_SERVER
	{
		damn_keys_filter		filter;
		(void)filter;
#endif // DEDICATED_SERVER

		InitEngine				();

		InitInput				();

		InitConsole				();

		Engine.External.CreateRendererList();

		LPCSTR benchName = "-batch_benchmark ";
		if(strstr(lpCmdLine, benchName))
		{
			int sz = xr_strlen(benchName);
			string64				b_name;
			sscanf					(strstr(Core.Params,benchName)+sz,"%[^ ] ",b_name);
			doBenchmark				(b_name);
			return 0;
		}

		Msg("command line %s", lpCmdLine);
		LPCSTR sashName = "-openautomate ";
		if(strstr(lpCmdLine, sashName))
		{
			int sz = xr_strlen(sashName);
			string512				sash_arg;
			sscanf					(strstr(Core.Params,sashName)+sz,"%[^ ] ",sash_arg);
			//doBenchmark				(sash_arg);
			g_SASH.Init(sash_arg);
			g_SASH.MainLoop();
			return 0;
		}

		if (strstr(lpCmdLine,"-launcher")) 
		{
			int l_res = doLauncher();
			if (l_res != 0)
				return 0;
		};

#ifndef DEDICATED_SERVER
		if(strstr(Core.Params,"-r2a"))	
			Console->Execute			("renderer renderer_r2a");
		else
		if(strstr(Core.Params,"-r2"))	
			Console->Execute			("renderer renderer_r2");
		else
		{
			CCC_LoadCFG_custom*	pTmp = xr_new<CCC_LoadCFG_custom>("renderer ");
			pTmp->Execute				(Console->ConfigFile);
			xr_delete					(pTmp);
		}
#else
			Console->Execute			("renderer renderer_r1");
#endif
//.		InitInput					( );
		Engine.External.Initialize	( );
		Console->Execute			("stat_memory");

		Startup	 					( );
		Core._destroy				( );

		// check for need to execute something external
		if (/*xr_strlen(g_sLaunchOnExit_params) && */xr_strlen(g_sLaunchOnExit_app) ) 
		{
			//CreateProcess need to return results to next two structures
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			//We use CreateProcess to setup working folder
			char const * temp_wf = (xr_strlen(g_sLaunchWorkingFolder) > 0) ? g_sLaunchWorkingFolder : NULL;
			CreateProcess(g_sLaunchOnExit_app, g_sLaunchOnExit_params, NULL, NULL, FALSE, 0, NULL, 
				temp_wf, &si, &pi);

		}
#ifndef DEDICATED_SERVER
#ifdef NO_MULTI_INSTANCES		
		// Delete application presence mutex
		CloseHandle( hCheckPresenceMutex );
#endif
	}
	// here damn_keys_filter class instanse will be destroyed
#endif // DEDICATED_SERVER

	return						0;
}

int stack_overflow_exception_filter	(int exception_code)
{
   if (exception_code == EXCEPTION_STACK_OVERFLOW)
   {
       // Do not call _resetstkoflw here, because
       // at this point, the stack is not yet unwound.
       // Instead, signal that the handler (the __except block)
       // is to be executed.
       return EXCEPTION_EXECUTE_HANDLER;
   }
   else
       return EXCEPTION_CONTINUE_SEARCH;
}

#include <boost/crc.hpp>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     char *    lpCmdLine,
                     int       nCmdShow)
{
	//FILE* file				= 0;
	//fopen_s					( &file, "z:\\development\\call_of_prypiat\\resources\\gamedata\\shaders\\r3\\objects\\r4\\accum_sun_near_msaa_minmax.ps\\2048__1___________4_11141_", "rb" );
	//u32 const file_size		= 29544;
	//char* buffer			= (char*)malloc(file_size);
	//fread					( buffer, file_size, 1, file );
	//fclose					( file );

	//u32 const& crc			= *(u32*)buffer;

	//boost::crc_32_type		processor;
	//processor.process_block	( buffer + 4, buffer + file_size );
	//u32 const new_crc		= processor.checksum( );
	//VERIFY					( new_crc == crc );

	//free					(buffer);

	__try 
	{
		WinMain_impl		(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
	}
	__except(stack_overflow_exception_filter(GetExceptionCode()))
	{
		_resetstkoflw		();
		FATAL				("stack overflow");
	}

	return					(0);
}

LPCSTR _GetFontTexName (LPCSTR section)
{
	static char* tex_names[] = { "texture800", "texture", "texture1600", "texture2k", "texture4k" };
	int def_idx		= 1;//default 1024x768
	int idx			= def_idx;

#if 0
	u32 w = Device.dwWidth;

	if(w<=800)		idx = 0;
	else if(w<=1280)idx = 1;
	else 			idx = 2;
#else
	u32 h = Device.dwHeight;
	
	if		(h<=600)	idx = 0;
	else if (h<1024)	idx = 1;
	else if (h<1200)	idx = 2;
	else if (h<1440)	idx = 3;
	else				idx = 4;
#endif

	while(idx>=0){
		if( pSettings->line_exist(section,tex_names[idx]) )
			return pSettings->r_string(section,tex_names[idx]);
		--idx;
	}
	return pSettings->r_string(section,tex_names[def_idx]);
}

void _InitializeFont(CGameFont*& F, LPCSTR section, u32 flags)
{
	LPCSTR font_tex_name = _GetFontTexName(section);
	R_ASSERT(font_tex_name);

	LPCSTR sh_name = pSettings->r_string(section,"shader");
	if(!F){
		F = xr_new<CGameFont> (sh_name, font_tex_name, flags);
	}else
		F->Initialize(sh_name, font_tex_name);

	if (pSettings->line_exist(section,"size")){
		float sz = pSettings->r_float(section,"size");
		if (flags&CGameFont::fsDeviceIndependent)	F->SetHeightI(sz);
		else										F->SetHeight(sz);
	}
	if (pSettings->line_exist(section,"interval"))
		F->SetInterval(pSettings->r_fvector2(section,"interval"));

}

CApplication::CApplication()
{
	ll_dwReference	= 0;

	max_load_stage = 0;

	// events
	eQuit						= Engine.Event.Handler_Attach("KERNEL:quit",this);
	eStart						= Engine.Event.Handler_Attach("KERNEL:start",this);
	eStartLoad					= Engine.Event.Handler_Attach("KERNEL:load",this);
	eDisconnect					= Engine.Event.Handler_Attach("KERNEL:disconnect",this);
	eConsole					= Engine.Event.Handler_Attach("KERNEL:console",this);
	eStartMPDemo				= Engine.Event.Handler_Attach("KERNEL:start_mp_demo",this);

	// levels
	Level_Current				= u32(-1);
	Level_Scan					( );

	// Font
	pFontSystem					= NULL;

	// Register us
	Device.seqFrame.Add			(this, REG_PRIORITY_HIGH+1000);
	
	if (psDeviceFlags.test(mtSound))	Device.seqFrameMT.Add		(&SoundProcessor);
	else								Device.seqFrame.Add			(&SoundProcessor);

	Console->Show				( );

	// App Title
	loadingScreen = nullptr;
}

CApplication::~CApplication()
{
	Console->Hide				( );

	// font
	xr_delete					( pFontSystem		);

	Device.seqFrameMT.Remove	(&SoundProcessor);
	Device.seqFrame.Remove		(&SoundProcessor);
	Device.seqFrame.Remove		(this);


	// events
	Engine.Event.Handler_Detach	(eConsole,this);
	Engine.Event.Handler_Detach	(eDisconnect,this);
	Engine.Event.Handler_Detach	(eStartLoad,this);
	Engine.Event.Handler_Detach	(eStart,this);
	Engine.Event.Handler_Detach	(eQuit,this);
	Engine.Event.Handler_Detach	(eStartMPDemo,this);
	
}

extern CRenderDevice Device;

void CApplication::OnEvent(EVENT E, u64 P1, u64 P2)
{
	if (E==eQuit)
	{
		g_SASH.EndBenchmark();

		PostQuitMessage	(0);
		
		for (u32 i=0; i<Levels.size(); i++)
		{
			xr_free(Levels[i].folder);
			xr_free(Levels[i].name);
		}
	}
	else if(E==eStart) 
	{
		LPSTR		op_server		= LPSTR	(P1);
		LPSTR		op_client		= LPSTR	(P2);
		Level_Current				= u32(-1);
		R_ASSERT	(0==g_pGameLevel);
		R_ASSERT	(0!=g_pGamePersistent);

#ifdef NO_SINGLE
		Console->Execute("main_menu on");
		if (	(op_server == NULL)			||
				(!xr_strlen(op_server))		||
				(
					(	strstr(op_server, "/dm")	|| strstr(op_server, "/deathmatch") ||
						strstr(op_server, "/tdm")	|| strstr(op_server, "/teamdeathmatch") ||
						strstr(op_server, "/ah")	|| strstr(op_server, "/artefacthunt") ||
						strstr(op_server, "/cta")	|| strstr(op_server, "/capturetheartefact")
					) && 
					!strstr(op_server, "/alife")
				)
			)
#endif // #ifdef NO_SINGLE
		{		
			Console->Execute("main_menu off");
			Console->Hide();
//!			this line is commented by Dima
//!			because I don't see any reason to reset device here
//!			Device.Reset					(false);
			//-----------------------------------------------------------
			g_pGamePersistent->PreStart		(op_server);
			//-----------------------------------------------------------
			g_pGameLevel					= (IGame_Level*)NEW_INSTANCE(CLSID_GAME_LEVEL);
			pApp->LoadBegin					(); 
			g_pGamePersistent->Start		(op_server);
			g_pGameLevel->net_Start			(op_server,op_client);
			pApp->LoadEnd					(); 
		}
		xr_free							(op_server);
		xr_free							(op_client);
	} 
	else if (E==eDisconnect) 
	{
		if (g_pGameLevel) 
		{
			Console->Hide			();
			g_pGameLevel->net_Stop	();
			DEL_INSTANCE			(g_pGameLevel);
			Console->Show			();
			
			if( (FALSE == Engine.Event.Peek("KERNEL:quit")) &&(FALSE == Engine.Event.Peek("KERNEL:start")) )
			{
				Console->Execute("main_menu off");
				Console->Execute("main_menu on");
			}
		}
		R_ASSERT			(0!=g_pGamePersistent);
		g_pGamePersistent->Disconnect();
	}
	else if (E == eConsole)
	{
		LPSTR command				= (LPSTR)P1;
		Console->ExecuteCommand		( command, false );
		xr_free						(command);
	}
	else if (E == eStartMPDemo)
	{
		LPSTR demo_file				= LPSTR	(P1);

		R_ASSERT	(0==g_pGameLevel);
		R_ASSERT	(0!=g_pGamePersistent);

		Console->Execute("main_menu off");
		Console->Hide();
		Device.Reset					(false);

		g_pGameLevel					= (IGame_Level*)NEW_INSTANCE(CLSID_GAME_LEVEL);
		shared_str server_options		= g_pGameLevel->OpenDemoFile(demo_file);
		
		//-----------------------------------------------------------
		g_pGamePersistent->PreStart		(server_options.c_str());
		//-----------------------------------------------------------
		
		pApp->LoadBegin					(); 
		g_pGamePersistent->Start		("");//server_options.c_str()); - no prefetch !
		g_pGameLevel->net_StartPlayDemo	();
		pApp->LoadEnd					(); 

		xr_free						(demo_file);
	}
}

static	CTimer	phase_timer		;
extern	ENGINE_API BOOL			g_appLoaded = FALSE;

void CApplication::LoadBegin	()
{
	ll_dwReference++;
	if (1==ll_dwReference)	{

		g_appLoaded			= FALSE;

#ifndef DEDICATED_SERVER
		_InitializeFont		(pFontSystem,"ui_font_letterica18_russian",0);
#endif
		phase_timer.Start	();
		load_stage			= 0;

		CheckCopyProtection	();
	}
}

void CApplication::LoadEnd		()
{
	ll_dwReference--;
	if (0==ll_dwReference)		{
#ifndef MASTER_GOLD
		Msg						("* phase time: %d ms",phase_timer.GetElapsed_ms());
		Msg						("* phase cmem: %d K", Memory.mem_usage()/1024);
#endif
		Console->Execute		("stat_memory");
		g_appLoaded				= TRUE;
//		DUMP_PHASE;
	}
}

void CApplication::SetLoadingScreen(ILoadingScreen* newScreen)
{
	if (loadingScreen)
	{
		Log("! Trying to create new loading screen, but there is already one..");
		DestroyLoadingScreen();
	}

	loadingScreen = newScreen;
}

void CApplication::DestroyLoadingScreen()
{
	xr_delete(loadingScreen);
}

//u32 calc_progress_color(u32, u32, int, int);

PROTECT_API void CApplication::LoadDraw		()
{
	if(g_appLoaded)				return;

	Device.dwFrame				+= 1;


	Render->firstViewPort = MAIN_VIEWPORT;
	Render->lastViewPort = MAIN_VIEWPORT;
	Render->currentViewPort = MAIN_VIEWPORT;
	Render->needPresenting = true;

	if(!Device.Begin () )		return;

	if	(g_dedicated_server)
		Console->OnRender			();
	else
		load_draw_internal			();

	Device.End					();
	CheckCopyProtection			();
}

void CApplication::LoadForceFinish()
{
	if (loadingScreen)
		loadingScreen->ForceFinish();
}

void CApplication::SetLoadStageTitle(const char* _ls_title)
{
	if (((load_screen_renderer.b_need_user_input && ClearSkyMode) || ps_rs_loading_stages) && loadingScreen)
		loadingScreen->SetStageTitle(_ls_title);
}

void CApplication::LoadTitleInt(LPCSTR str1, LPCSTR str2, LPCSTR str3)
{
	if (loadingScreen)
		loadingScreen->SetStageTip(str1, str2, str3);
}
void CApplication::LoadStage()
{
	VERIFY						(ll_dwReference);
	Msg							("* phase time: %d ms",phase_timer.GetElapsed_ms());	phase_timer.Start();
	Msg							("* phase cmem: %d K", Memory.mem_usage()/1024);
	
	if (g_pGamePersistent->GameType() == 1 && !xr_strcmp(g_pGamePersistent->m_game_params.m_alife, "alife"))
		max_load_stage			= 17;
	else
		max_load_stage			= 14;
	LoadDraw					();
	++load_stage;
}

void CApplication::LoadSwitch	()
{
}

// Sequential
void CApplication::OnFrame	( )
{
	Engine.Event.OnFrame			();
	g_SpatialSpace->update			();
	g_SpatialSpacePhysic->update	();
	if (g_pGameLevel)				
		g_pGameLevel->SoundEvent_Dispatch	( );
}

void CApplication::Level_Append		(LPCSTR folder)
{
	string_path	N1,N2,N3,N4;
	strconcat	(sizeof(N1),N1,folder,"level");
	strconcat	(sizeof(N2),N2,folder,"level.ltx");
	strconcat	(sizeof(N3),N3,folder,"level.geom");
	strconcat	(sizeof(N4),N4,folder,"level.cform");
	if	(
		FS.exist("$game_levels$",N1)		&&
		FS.exist("$game_levels$",N2)		&&
		FS.exist("$game_levels$",N3)		&&
		FS.exist("$game_levels$",N4)	
		)
	{
		sLevelInfo			LI;
		LI.folder			= xr_strdup(folder);
		LI.name				= 0;
		Levels.push_back	(LI);
	}
}

void CApplication::Level_Scan()
{
	SECUROM_MARKER_PERFORMANCE_ON(8)

	for (u32 i=0; i<Levels.size(); i++)
	{
		xr_free(Levels[i].folder);
		xr_free(Levels[i].name);
	}
	Levels.clear	();


	xr_vector<char*>* folder			= FS.file_list_open		("$game_levels$",FS_ListFolders|FS_RootOnly);
//.	R_ASSERT							(folder&&folder->size());
	
	for (u32 i=0; i<folder->size(); ++i)	
		Level_Append((*folder)[i]);
	
	FS.file_list_close		(folder);

	SECUROM_MARKER_PERFORMANCE_OFF(8)
}

void gen_logo_name(string_path& dest, LPCSTR level_name, int num)
{
	if (!bWinterMode)
	{
		strconcat(sizeof(dest), dest, "intro\\intro_", level_name);
	}
	else
	{
		strconcat(sizeof(dest), dest, "intro\\intro_winter_", level_name);
	}
	
	u32 len = xr_strlen(dest);
	if(dest[len-1]=='\\')
		dest[len-1] = 0;

	string16 buff;
	xr_strcat(dest, sizeof(dest), "_");
	xr_strcat(dest, sizeof(dest), itoa(num+1, buff, 10));
}

void CApplication::Level_Set(u32 L)
{
	SECUROM_MARKER_PERFORMANCE_ON(9)

	if (L>=Levels.size())	return;
	FS.get_path	("$level$")->_set	(Levels[L].folder);

	static string_path			path;

	if(Level_Current != L)
	{
		path[0]					= 0;

		Level_Current			= L;
		
		int count				= 0;
		while(true)
		{
			string_path			temp2;
			gen_logo_name		(path, Levels[L].folder, count);
			if(FS.exist(temp2, "$game_textures$", path, ".dds") || FS.exist(temp2, "$level$", path, ".dds"))
				count++;
			else
				break;
		}

		if(count)
		{
			int num				= ::Random.randI(count);
			gen_logo_name		(path, Levels[L].folder, num);
		}
	}

	if (path[0] && loadingScreen)
		loadingScreen->SetLevelLogo(path);

	CheckCopyProtection			();

	SECUROM_MARKER_PERFORMANCE_OFF(9)
}

int CApplication::Level_ID(LPCSTR name, LPCSTR ver, bool bSet)
{
	int result = -1;

	SECUROM_MARKER_SECURITY_ON(7)

	CLocatorAPI::archives_it it		= FS.m_archives.begin();
	CLocatorAPI::archives_it it_e	= FS.m_archives.end();
	bool arch_res					= false;

	for(;it!=it_e;++it)
	{
		CLocatorAPI::archive& A		= *it;
		if(A.hSrcFile==NULL)
		{
			LPCSTR ln = A.header->r_string("header", "level_name");
			LPCSTR lv = A.header->r_string("header", "level_ver");
			if ( 0==stricmp(ln,name) && 0==stricmp(lv,ver) )
			{
				FS.LoadArchive(A);
				arch_res = true;
			}
		}
	}

	if( arch_res )
		Level_Scan							();
	
	string256		buffer;
	strconcat		(sizeof(buffer),buffer,name,"\\");
	for (u32 I=0; I<Levels.size(); ++I)
	{
		if (0==stricmp(buffer,Levels[I].folder))	
		{
			if (Levels[I].name == nullptr)
			{
				Levels[I].name = xr_strdup(name);
			}
			result = int(I);	
			break;
		}
	}

	if(bSet && result!=-1)
		Level_Set(result);

	if( arch_res )
		g_pGamePersistent->OnAssetsChanged	();

	SECUROM_MARKER_SECURITY_OFF(7)

	return result;
}

CInifile*  CApplication::GetArchiveHeader(LPCSTR name, LPCSTR ver)
{
	CLocatorAPI::archives_it it		= FS.m_archives.begin();
	CLocatorAPI::archives_it it_e	= FS.m_archives.end();

	for(;it!=it_e;++it)
	{
		CLocatorAPI::archive& A		= *it;

		LPCSTR ln = A.header->r_string("header", "level_name");
		LPCSTR lv = A.header->r_string("header", "level_ver");
		if ( 0==stricmp(ln,name) && 0==stricmp(lv,ver) )
		{
			return A.header;
		}
	}
	return NULL;
}

void CApplication::LoadAllArchives()
{
	if( FS.load_all_unloaded_archives() )
	{
		Level_Scan							();
		g_pGamePersistent->OnAssetsChanged	();
	}
}

#ifndef DEDICATED_SERVER
// Parential control for Vista and upper
typedef BOOL (*PCCPROC)( CHAR* ); 

BOOL IsPCAccessAllowed()
{
	CHAR szPCtrlChk[ MAX_PATH ] , szGDF[ MAX_PATH ] , *pszLastSlash;
	HINSTANCE hPCtrlChk = NULL;
	PCCPROC pctrlchk = NULL;
	BOOL bAllowed = TRUE;

	if ( ! GetModuleFileName( NULL , szPCtrlChk , MAX_PATH ) )
		return TRUE;

	if ( ( pszLastSlash = strrchr( szPCtrlChk , '\\' ) ) == NULL )
		return TRUE;

	*pszLastSlash = '\0';

	strcpy_s( szGDF , szPCtrlChk );

	strcat_s( szPCtrlChk , "\\pctrlchk.dll" );
	if ( GetFileAttributes( szPCtrlChk ) == INVALID_FILE_ATTRIBUTES )
		return TRUE;

	if ( ( pszLastSlash = strrchr( szGDF , '\\' ) ) == NULL )
		return TRUE;

	*pszLastSlash = '\0';

	strcat_s( szGDF , "\\Stalker-COP.exe" );
	if ( GetFileAttributes( szGDF ) == INVALID_FILE_ATTRIBUTES )
		return TRUE;

	if ( ( hPCtrlChk = LoadLibrary( szPCtrlChk ) ) == NULL )
		return TRUE;

	if ( ( pctrlchk = (PCCPROC) GetProcAddress( hPCtrlChk , "pctrlchk" ) ) == NULL ) {
		FreeLibrary( hPCtrlChk );
		return TRUE;
	}

	bAllowed = pctrlchk( szGDF );

	FreeLibrary( hPCtrlChk );

	return bAllowed;
}
#endif // DEDICATED_SERVER

//launcher stuff----------------------------
extern "C"{
	typedef int	 __cdecl LauncherFunc	(int);
}
HMODULE			hLauncher		= NULL;
LauncherFunc*	pLauncher		= NULL;

void InitLauncher(){
	if(hLauncher)
		return;
	hLauncher	= LoadLibrary	("xrLauncher.dll");
	if (0==hLauncher)	R_CHK	(GetLastError());
	R_ASSERT2		(hLauncher,"xrLauncher DLL raised exception during loading or there is no xrLauncher.dll at all");

	pLauncher = (LauncherFunc*)GetProcAddress(hLauncher,"RunXRLauncher");
	R_ASSERT2		(pLauncher,"Cannot obtain RunXRLauncher function from xrLauncher.dll");
};

void FreeLauncher(){
	if (hLauncher)	{ 
		FreeLibrary(hLauncher); 
		hLauncher = NULL; pLauncher = NULL; };
}

int doLauncher()
{
/*
	execUserScript();
	InitLauncher();
	int res = pLauncher(0);
	FreeLauncher();
	if(res == 1) // do benchmark
		g_bBenchmark = true;

	if(g_bBenchmark){ //perform benchmark cycle
		doBenchmark();
	
		// InitLauncher	();
		// pLauncher	(2);	//show results
		// FreeLauncher	();

		Core._destroy			();
		return					(1);

	};
	if(res==8){//Quit
		Core._destroy			();
		return					(1);
	}
*/
	return 0;
}

std::string ToUTF8(const char* str)
{
	std::string res;
	int result_u, result_c;
	result_u = MultiByteToWideChar(1251, 0, str, -1, 0, 0);
	if (!result_u)
		return 0;
	wchar_t* ures = new wchar_t[result_u];
	if (!MultiByteToWideChar(1251, 0, str, -1, ures, result_u)) {
		delete[] ures;
		return 0;
	}
	result_c = WideCharToMultiByte(CP_UTF8, 0, ures, -1, 0, 0, 0, 0);
	if (!result_c) {
		delete[] ures;
		return 0;
	}
	char* cres = new char[result_c];
	if (!WideCharToMultiByte(CP_UTF8, 0, ures, -1, cres, result_c, 0, 0)) {
		delete[] cres;
		return 0;
	}
	delete[] ures;
	res = (cres);
	delete[] cres;
	return res;
}

void doBenchmark(LPCSTR name)
{
	g_bBenchmark = true;
	string_path in_file;
	FS.update_path(in_file,"$app_data_root$", name);
	CInifile ini(in_file);
	int test_count = ini.line_count("benchmark");
	LPCSTR test_name,t;
	shared_str test_command;
	for(int i=0;i<test_count;++i){
		ini.r_line			( "benchmark", i, &test_name, &t);
		xr_strcpy				(g_sBenchmarkName, test_name);
		
		test_command		= ini.r_string_wb("benchmark",test_name);
		xr_strcpy			(Core.Params,*test_command);
		_strlwr_s				(Core.Params);
		
		InitInput					();
		if(i){
			//ZeroMemory(&HW,sizeof(CHW));
			//	TODO: KILL HW here!
			//  pApp->m_pRender->KillHW();
			InitEngine();
		}


		Engine.External.Initialize	( );

		xr_strcpy						(Console->ConfigFile,"user.ltx");
		if (strstr(Core.Params,"-ltx ")) {
			string64				c_name;
			sscanf					(strstr(Core.Params,"-ltx ")+5,"%[^ ] ",c_name);
			xr_strcpy				(Console->ConfigFile,c_name);
		}

		Startup	 				();
	}
}
#pragma optimize("g", off)

void CApplication::load_draw_internal()
{
	if (loadingScreen)
		loadingScreen->Update(load_stage, max_load_stage);
}