//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "../xrEngine/GameMtlLib.h"

void DestroySounds(SoundVec& lst)
{
	for (SoundIt it=lst.begin(); lst.end() != it; ++it)	
		it->destroy();
}
/*
void DestroyMarks(ShaderVec& lst)
{
	for (ShaderIt it=lst.begin(); lst.end() != it; ++it)
		it->destroy();
}
*/

void DestroyPSs(PSVec& lst)
{
//	for (PSIt it=lst.begin(); lst.end() != it; ++it)
//		Device.Resources->Delete(*it);
}

void CreateSounds(SoundVec& lst, LPCSTR buf)
{
	string256 tmp;
	int cnt = _GetItemCount(buf);

	lst.resize(cnt);

	for (int k = 0; k < cnt; ++k)
	{
#ifdef DEBUG
		if (cnt > GAMEMTL_SUBITEM_COUNT)
			Msg("[GameMtlLib_Engine (CreateSounds())]: Items count ([%d]) specified for the material [%s] exceeds the limit from GAMEMTL_SUBITEM_COUNT ([%d]). Check your gamemtl.xr.", cnt, _GetItem(buf, k, tmp), GAMEMTL_SUBITEM_COUNT);
#endif

		lst[k].create(_GetItem(buf, k, tmp), st_Effect, sg_SourceType);
	}
}

void CreateMarks(IWallMarkArray *pMarks, LPCSTR buf)
{
	string256 tmp;
	int cnt =_GetItemCount(buf);

	for (int k = 0; k < cnt; ++k)
	{
#ifdef DEBUG
		if (cnt > GAMEMTL_SUBITEM_COUNT)
			Msg("[GameMtlLib_Engine (CreateMarks())]: Items count ([%d]) specified for the material [%s] exceeds the limit from GAMEMTL_SUBITEM_COUNT ([%d]). Check your gamemtl.xr.", cnt, _GetItem(buf, k, tmp), GAMEMTL_SUBITEM_COUNT);
#endif

		pMarks->AppendMark(_GetItem(buf, k, tmp));
	}
}


void CreatePSs(PSVec& lst, LPCSTR buf)
{
	string256 tmp;
	int cnt = _GetItemCount(buf);

	for (int k = 0; k < cnt; ++k)
	{
#ifdef DEBUG
		if (cnt > GAMEMTL_SUBITEM_COUNT)
			Msg("[GameMtlLib_Engine (CreatePSs())]: Items count ([%d]) specified for the material [%s] exceeds the limit from GAMEMTL_SUBITEM_COUNT ([%d]). Check your gamemtl.xr.", cnt, _GetItem(buf, k, tmp), GAMEMTL_SUBITEM_COUNT);
#endif

		lst.push_back(_GetItem(buf, k, tmp));
	}
}

SGameMtlPair::~SGameMtlPair()
{
	// destroy all media
	DestroySounds	(BreakingSounds);
	DestroySounds	(StepSounds);
	DestroySounds	(CollideSounds);
	DestroyPSs		(CollideParticles);
//	DestroyMarks	(CollideMarks);
	//RenderFactory->DestroyGameMtlPair(m_pCollideMarks);
	//m_pCollideMarks->
}

void SGameMtlPair::Load(IReader& fs)
{
	shared_str				buf;

	R_ASSERT(fs.find_chunk(GAMEMTLPAIR_CHUNK_PAIR));
    mtl0				= fs.r_u32();
    mtl1				= fs.r_u32();
    ID					= fs.r_u32();
    ID_parent			= fs.r_u32();
    OwnProps.assign		(fs.r_u32());
 
    R_ASSERT(fs.find_chunk(GAMEMTLPAIR_CHUNK_BREAKING));
    fs.r_stringZ			(buf); 		CreateSounds		(BreakingSounds,*buf);
    
    R_ASSERT(fs.find_chunk(GAMEMTLPAIR_CHUNK_STEP));
    fs.r_stringZ			(buf);		CreateSounds		(StepSounds,*buf);
    
	R_ASSERT(fs.find_chunk(GAMEMTLPAIR_CHUNK_COLLIDE));
    fs.r_stringZ			(buf);		CreateSounds		(CollideSounds,*buf);
    fs.r_stringZ			(buf);		CreatePSs			(CollideParticles,*buf);
    fs.r_stringZ			(buf);		
	//CreateMarks			(CollideMarks,*buf);
	CreateMarks			(&*m_pCollideMarks,*buf);
}
