// Engine.cpp: implementation of the CEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Engine.h"

CEngine				Engine;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEngine::CEngine()
{
	
}

CEngine::~CEngine()
{
	
}

extern	void msCreate		(LPCSTR name);

void CEngine::Initialize	(void)
{
	ZoneScoped;

	// Other stuff
	Engine.Sheduler.Initialize			( );
	// 
#ifdef DEBUG
	msCreate							("game");
#endif
}

typedef void __cdecl ttapi_Done_func(void);

void CEngine::Destroy	()
{
	ZoneScoped;

	Engine.Sheduler.Destroy				( );
#ifdef DEBUG_MEMORY_MANAGER
	extern void	dbg_dump_leaks_prepare	( );
	if (Memory.debug_mode)				dbg_dump_leaks_prepare	();
#endif // DEBUG_MEMORY_MANAGER
	Engine.External.Destroy				( );
}
