////////////////////////////////////////////////////////////////////////////
//	Module 		: level_graph_debug2.cpp
//	Created 	: 02.10.2001
//  Modified 	: 11.11.2003
//	Author		: Oles Shihkovtsov, Dmitriy Iassenev
//	Description : Level graph debug functions
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"

#ifndef AI_COMPILER

#include "level_graph.h"
#include "../xrEngine/customhud.h"
#include "ai_space.h"
#include "ui_base.h"
#include "game_graph.h"
#include "game_sv_single.h"
#include "custommonster.h"
#include "ai/stalker/ai_stalker.h"
#include "xrserver_objects_alife_monsters.h"
#include "cover_point.h"
#include "cover_manager.h"
#include "cover_evaluators.h"
#include "team_base_zone.h"
#include "alife_simulator.h"
#include "alife_graph_registry.h"
#include "alife_object_registry.h"
#include "game_cl_base.h"
#include "space_restriction_manager.h"
#include "space_restriction.h"
#include "space_restrictor.h"
#include "space_restriction_base.h"
#include "detail_path_manager.h"
#include "memory_manager.h"
#include "enemy_manager.h"
#include "memory_space.h"
#include "level.h"
#include "ai_object_location.h"
#include "movement_manager.h"
#include "graph_engine.h"
#include "debug_renderer.h"
#include "smart_cover_object.h"

typedef Fvector2 t_node_tc[4];
static const float dtc = 0.25f;
static t_node_tc node_tc[16] = {
	{ { 0.f + 0 * dtc, 0.25f + 0 * dtc }, { 0.25f + 0 * dtc, 0.25f + 0 * dtc }, { 0.25f + 0 * dtc, 0.f + 0 * dtc },
		{ 0.f + 0 * dtc, 0.f + 0 * dtc } },
	{ { 0.f + 1 * dtc, 0.25f + 0 * dtc }, { 0.25f + 1 * dtc, 0.25f + 0 * dtc }, { 0.25f + 1 * dtc, 0.f + 0 * dtc },
		{ 0.f + 1 * dtc, 0.f + 0 * dtc } },
	{ { 0.f + 2 * dtc, 0.25f + 0 * dtc }, { 0.25f + 2 * dtc, 0.25f + 0 * dtc }, { 0.25f + 2 * dtc, 0.f + 0 * dtc },
		{ 0.f + 2 * dtc, 0.f + 0 * dtc } },
	{ { 0.f + 3 * dtc, 0.25f + 0 * dtc }, { 0.25f + 3 * dtc, 0.25f + 0 * dtc }, { 0.25f + 3 * dtc, 0.f + 0 * dtc },
		{ 0.f + 3 * dtc, 0.f + 0 * dtc } },

	{ { 0.f + 0 * dtc, 0.25f + 1 * dtc }, { 0.25f + 0 * dtc, 0.25f + 1 * dtc }, { 0.25f + 0 * dtc, 0.f + 1 * dtc },
		{ 0.f + 0 * dtc, 0.f + 1 * dtc } },
	{ { 0.f + 1 * dtc, 0.25f + 1 * dtc }, { 0.25f + 1 * dtc, 0.25f + 1 * dtc }, { 0.25f + 1 * dtc, 0.f + 1 * dtc },
		{ 0.f + 1 * dtc, 0.f + 1 * dtc } },
	{ { 0.f + 2 * dtc, 0.25f + 1 * dtc }, { 0.25f + 2 * dtc, 0.25f + 1 * dtc }, { 0.25f + 2 * dtc, 0.f + 1 * dtc },
		{ 0.f + 2 * dtc, 0.f + 1 * dtc } },
	{ { 0.f + 3 * dtc, 0.25f + 1 * dtc }, { 0.25f + 3 * dtc, 0.25f + 1 * dtc }, { 0.25f + 3 * dtc, 0.f + 1 * dtc },
		{ 0.f + 3 * dtc, 0.f + 1 * dtc } },

	{ { 0.f + 0 * dtc, 0.25f + 2 * dtc }, { 0.25f + 0 * dtc, 0.25f + 2 * dtc }, { 0.25f + 0 * dtc, 0.f + 2 * dtc },
		{ 0.f + 0 * dtc, 0.f + 2 * dtc } },
	{ { 0.f + 1 * dtc, 0.25f + 2 * dtc }, { 0.25f + 1 * dtc, 0.25f + 2 * dtc }, { 0.25f + 1 * dtc, 0.f + 2 * dtc },
		{ 0.f + 1 * dtc, 0.f + 2 * dtc } },
	{ { 0.f + 2 * dtc, 0.25f + 2 * dtc }, { 0.25f + 2 * dtc, 0.25f + 2 * dtc }, { 0.25f + 2 * dtc, 0.f + 2 * dtc },
		{ 0.f + 2 * dtc, 0.f + 2 * dtc } },
	{ { 0.f + 3 * dtc, 0.25f + 2 * dtc }, { 0.25f + 3 * dtc, 0.25f + 2 * dtc }, { 0.25f + 3 * dtc, 0.f + 2 * dtc },
		{ 0.f + 3 * dtc, 0.f + 2 * dtc } },

	{ { 0.f + 0 * dtc, 0.25f + 3 * dtc }, { 0.25f + 0 * dtc, 0.25f + 3 * dtc }, { 0.25f + 0 * dtc, 0.f + 3 * dtc },
		{ 0.f + 0 * dtc, 0.f + 3 * dtc } },
	{ { 0.f + 1 * dtc, 0.25f + 3 * dtc }, { 0.25f + 1 * dtc, 0.25f + 3 * dtc }, { 0.25f + 1 * dtc, 0.f + 3 * dtc },
		{ 0.f + 1 * dtc, 0.f + 3 * dtc } },
	{ { 0.f + 2 * dtc, 0.25f + 3 * dtc }, { 0.25f + 2 * dtc, 0.25f + 3 * dtc }, { 0.25f + 2 * dtc, 0.f + 3 * dtc },
		{ 0.f + 2 * dtc, 0.f + 3 * dtc } },
	{ { 0.f + 3 * dtc, 0.25f + 3 * dtc }, { 0.25f + 3 * dtc, 0.25f + 3 * dtc }, { 0.25f + 3 * dtc, 0.f + 3 * dtc },
		{ 0.f + 3 * dtc, 0.f + 3 * dtc } },
};

void CLevelGraph::draw_nodes()
{
	CGameObject* O = smart_cast<CGameObject*>(Level().CurrentEntity());

	if (!O)
		return;

	Fvector POSITION = O->Position();
	POSITION.y += 0.5f;

	Fvector P = POSITION;
	u32 ID = O->ai_location().level_vertex_id();

	CGameFont* F = UI().Font().pFontDI;
	F->SetHeightI(.02f);
	F->OutI(0.f, 0.5f, "%f,%f,%f", VPUSH(P));

	svector<u32, 128> linked;
	{
		const_iterator i, e;
		begin(ID, i, e);

		for (; i != e; ++i)
			linked.push_back(value(ID, i));
	}

	float st = 0.98f * header().cell_size() / 2;
	float tt = 0.01f;
	Fvector DUP;
	DUP.set(0, 1, 0);

	F->SetColor(color_rgba(255, 255, 255, 255));

	Fvector min_position, max_position;
	max_position = min_position = Device.vCameraPosition;
	min_position.sub(30.f);
	max_position.add(30.f);

	CLevelGraph::const_vertex_iterator I, E;

	if (valid_vertex_position(min_position))
		I = std::lower_bound(begin(), end(), vertex_position(min_position).xz(), &vertex::predicate2);
	else
		I = begin();

	if (valid_vertex_position(max_position))
	{
		E = std::upper_bound(begin(), end(), vertex_position(max_position).xz(), &vertex::predicate);
		if (E != end()) ++E;
	}
	else
		E = end();

	u32 DvbSize = 1536 * 1024;
	u32 PointSize = 24;
	u32 PointCount = 6;
	u32 nodeBatch = DvbSize / PointSize / PointCount;

	UIRender->StartPrimitive(nodeBatch * PointCount, IUIRender::ptTriList, IUIRender::pttLIT);

	u32 iNode = 0;
	for (; I != E; ++I)
	{
		const CLevelGraph::CVertex& N = *I;
		Fvector PC = vertex_position(N);
		u32 Nid = vertex_id(I);

		if (Device.vCameraPosition.distance_to(PC) > 30)
			continue;

		float sr = header().cell_size();
		if (!::Render->ViewBase.testSphere_dirty(PC, sr))
			continue;

		u32 color = color_xrgb(255, 255, 255);

		BOOL bHL = FALSE;
		if (Nid == u32(ID))
		{
			bHL = TRUE;
			color = color_xrgb(0, 255, 0);
		}
		else
		{
			for (u32 t = 0; t < linked.size(); ++t)
			{
				if (linked[t] == Nid)
				{
					bHL = TRUE;
					color = color_xrgb(0, 128, 0);
					break;
				}
			}
		}

		if (!m_access_mask[Nid])
			color = color_xrgb(255, 0, 0);

		int k = 0;

		if (valid_vertex_id(N.link(0)))
			k |= 1 << 0;

		if (valid_vertex_id(N.link(1)))
			k |= 1 << 1;

		if (valid_vertex_id(N.link(2)))
			k |= 1 << 2;

		if (valid_vertex_id(N.link(3)))
			k |= 1 << 3;

		// unpack plane
		Fplane PL;
		Fvector vNorm;
		pvDecompress(vNorm, N.plane());
		PL.build(PC, vNorm);

		Fvector v, v1, v2, v3, v4;
		v.set(PC.x - st, PC.y, PC.z - st);
		PL.intersectRayPoint(v, DUP, v1); v1.mad(v1, PL.n, tt);
		v.set(PC.x + st, PC.y, PC.z - st);
		PL.intersectRayPoint(v, DUP, v2); v2.mad(v2, PL.n, tt);
		v.set(PC.x + st, PC.y, PC.z + st);
		PL.intersectRayPoint(v, DUP, v3); v3.mad(v3, PL.n, tt);
		v.set(PC.x - st, PC.y, PC.z + st);
		PL.intersectRayPoint(v, DUP, v4); v4.mad(v4, PL.n, tt);

		UIRender->PushPoint(v3.x, v3.y, v3.z, color, node_tc[k][2].x, node_tc[k][2].y);
		UIRender->PushPoint(v2.x, v2.y, v2.z, color, node_tc[k][1].x, node_tc[k][1].y);
		UIRender->PushPoint(v1.x, v1.y, v1.z, color, node_tc[k][0].x, node_tc[k][0].y);

		UIRender->PushPoint(v4.x, v4.y, v4.z, color, node_tc[k][3].x, node_tc[k][3].y);
		UIRender->PushPoint(v3.x, v3.y, v3.z, color, node_tc[k][2].x, node_tc[k][2].y);
		UIRender->PushPoint(v1.x, v1.y, v1.z, color, node_tc[k][0].x, node_tc[k][0].y);

		iNode++;
		if (iNode == nodeBatch)
		{
			UIRender->CacheSetXformWorld(Fidentity);
			UIRender->SetShader(*sh_debug);
			UIRender->FlushPrimitive();

			UIRender->StartPrimitive(nodeBatch * PointCount, IUIRender::ptTriList, IUIRender::pttLIT);
			iNode = 0;
		}

		float sc = header().cell_size() / 16;
		u32 CC = color_xrgb(0, 0, 255);
		Level().debug_renderer().draw_aabb(PC, sc, sc, sc, CC);

		if (bHL)
		{
			Fvector T;
			Fvector4 S;
			T.set(PC);
			T.y += 0.3f;
			Device.mFullTransform.transform(S, T);

			if (S.z < 0 || S.z < 0)
				continue;

			if (S.x < -1.f || S.x > 1.f || S.y < -1.f || S.y > 1.f)
				continue;

			F->SetHeightI(0.05f / _sqrt(_abs(S.w)));
			F->SetColor(0xffffffff);
			F->OutI(S.x, -S.y, "~%d", Nid);
		}
	}

	if (iNode > 0)
	{
		UIRender->CacheSetXformWorld(Fidentity);
		UIRender->SetShader(*sh_debug);
		UIRender->FlushPrimitive();
	}
}

void CLevelGraph::draw_restrictions	()
{
	CSpaceRestrictionManager::SPACE_RESTRICTIONS::const_iterator	I = Level().space_restriction_manager().restrictions().begin();
	CSpaceRestrictionManager::SPACE_RESTRICTIONS::const_iterator	E = Level().space_restriction_manager().restrictions().end();

	CRandom R;

	for ( ; I != E; ++I) {
		if (!(*I).second->m_ref_count)
			continue;
		if (!(*I).second->initialized()) continue;

		u8 b = u8(R.randI(255));
		u8 g = u8(R.randI(255));
		u8 r = u8(R.randI(255));

		xr_vector<u32>::const_iterator	i = (*I).second->border().begin();
		xr_vector<u32>::const_iterator	e = (*I).second->border().end();
		for ( ; i != e; ++i) {
			Fvector temp = ai().level_graph().vertex_position(*i);
			temp.y += .1f;
			Level().debug_renderer().draw_aabb(temp,.05f,.05f,.05f,color_xrgb(r,g,b));
		}

#ifdef USE_FREE_IN_RESTRICTIONS
		CSpaceRestriction::FREE_IN_RESTRICTIONS::const_iterator II = (*I).second->m_free_in_restrictions.begin();
		CSpaceRestriction::FREE_IN_RESTRICTIONS::const_iterator EE = (*I).second->m_free_in_restrictions.end();
		for ( ; II != EE; ++II) {
			xr_vector<u32>::const_iterator	i = (*II).m_restriction->border().begin();
			xr_vector<u32>::const_iterator	e = (*II).m_restriction->border().end();
			for ( ; i != e; ++i) {
				Fvector temp = ai().level_graph().vertex_position(*i);
				temp.y += .1f;
				Level().debug_renderer().draw_aabb(temp,.05f,.05f,.05f,color_xrgb(255,0,0));
			}
			{
				xr_vector<u32>::const_iterator	i = (*II).m_restriction->border().begin();
				xr_vector<u32>::const_iterator	e = (*II).m_restriction->border().end();
				for ( ; i != e; ++i) {
					Fvector temp = ai().level_graph().vertex_position(*i);
					temp.y += .1f;
					Level().debug_renderer().draw_aabb(temp,.05f,.05f,.05f,color_xrgb(0,255,0));
				}
			}
		}
#endif
	}
}

#ifdef DEBUG
void CLevelGraph::draw_covers	()
{
	float					half_size = ai().level_graph().header().cell_size()*.5f;
	xr_vector<CCoverPoint*>	nearest;
	nearest.reserve			(1000);
	ai().cover_manager().covers().nearest(Device.vCameraPosition,5.f,nearest);
	xr_vector<CCoverPoint*>::const_iterator	I = nearest.begin();
	xr_vector<CCoverPoint*>::const_iterator	E = nearest.end();
	for ( ; I != E; ++I) {
		Fvector				position = (*I)->position();
		position.y			+= 1.5f;
		Level().debug_renderer().draw_aabb	(position,half_size - .01f,1.f,ai().level_graph().header().cell_size()*.5f-.01f,color_xrgb(0*255,255,0*255));

		CVertex				*v = vertex((*I)->level_vertex_id());
		Fvector				direction;
		float				best_value = -1.f;

		u32 j = 0;
		for (u32 i=0; i<36; ++i) {
			float				value = high_cover_in_direction(float(10*i)/180.f*PI,v);
			direction.setHP		(float(10*i)/180.f*PI,0);
			direction.normalize	();
			direction.mul		(value*half_size);
			direction.add		(position);
			direction.y			= position.y;
			Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(0,0,255));
			value				= compute_high_square(float(10*i)/180.f*PI,PI/2.f,v);
			if (value > best_value) {
				best_value		= value;
				j				= i;
			}
		}

		direction.set		(position.x - half_size*float(v->high_cover(0))/15.f,position.y,position.z);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		direction.set		(position.x,position.y,position.z + half_size*float(v->high_cover(1))/15.f);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		direction.set		(position.x + half_size*float(v->high_cover(2))/15.f,position.y,position.z);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		direction.set		(position.x,position.y,position.z - half_size*float(v->high_cover(3))/15.f);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		float				value = high_cover_in_direction(float(10*j)/180.f*PI,v);
		direction.setHP		(float(10*j)/180.f*PI,0);
		direction.normalize	();
		direction.mul		(value*half_size);
		direction.add		(position);
		direction.y			= position.y;
		Level().debug_renderer().draw_line	(Fidentity,position,direction,color_xrgb(0,0,0));

		// low
		{
		position			= (*I)->position();
		position.y			+= 0.6f;
		Level().debug_renderer().draw_aabb	(position,half_size - .01f,1.f,ai().level_graph().header().cell_size()*.5f-.01f,color_xrgb(0*255,255,0*255));

		CVertex				*v = vertex((*I)->level_vertex_id());
		Fvector				direction;
		float				best_value = -1.f;

		u32 j = 0;
		for (u32 i=0; i<36; ++i) {
			float				value = low_cover_in_direction(float(10*i)/180.f*PI,v);
			direction.setHP		(float(10*i)/180.f*PI,0);
			direction.normalize	();
			direction.mul		(value*half_size);
			direction.add		(position);
			direction.y			= position.y;
			Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(0,0,255));
			value				= compute_low_square(float(10*i)/180.f*PI,PI/2.f,v);
			if (value > best_value) {
				best_value		= value;
				j				= i;
			}
		}

		direction.set		(position.x - half_size*float(v->low_cover(0))/15.f,position.y,position.z);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		direction.set		(position.x,position.y,position.z + half_size*float(v->low_cover(1))/15.f);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		direction.set		(position.x + half_size*float(v->low_cover(2))/15.f,position.y,position.z);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		direction.set		(position.x,position.y,position.z - half_size*float(v->low_cover(3))/15.f);
		Level().debug_renderer().draw_line(Fidentity,position,direction,color_xrgb(255,0,0));

		float				value = low_cover_in_direction(float(10*j)/180.f*PI,v);
		direction.setHP		(float(10*j)/180.f*PI,0);
		direction.normalize	();
		direction.mul		(value*half_size);
		direction.add		(position);
		direction.y			= position.y;
		Level().debug_renderer().draw_line	(Fidentity,position,direction,color_xrgb(0,0,0));
		}
	}
}

void CLevelGraph::draw_objects	()
{
	u32					I = 0;
	u32					E = Level().Objects.o_count	();
	for ( ; I < E; ++I) {
		CObject			*_O = Level().Objects.o_get_by_iterator(I);
		CTeamBaseZone	*team_base_zone = smart_cast<CTeamBaseZone*>(_O);
		if (team_base_zone) {
			team_base_zone->OnRender();
			continue;
		}

		CCustomMonster	*tpCustomMonster = smart_cast<CCustomMonster*>(_O);
		if (tpCustomMonster) {
			tpCustomMonster->OnRender();
			if (!tpCustomMonster->get_movement().detail().path().empty()) {
				Fvector				temp = tpCustomMonster->get_movement().detail().path()[tpCustomMonster->get_movement().detail().path().size() - 1].position;
				Level().debug_renderer().draw_aabb	(temp,1.f,1.f,1.f,color_xrgb(0,0,255));
			}
		}

		smart_cover::object	*smart_cover = smart_cast<smart_cover::object*>(_O);
		if (smart_cover) {
			smart_cover->OnRender	();
			continue;
		}
	}
}

#ifdef DEBUG
#ifndef AI_COMPILER
void CLevelGraph::draw_debug_node()
{
	if (g_bDebugNode) {
		Fvector pos_src, pos_dest;

		if (ai().level_graph().valid_vertex_id(g_dwDebugNodeSource)) {
			pos_src		= ai().level_graph().vertex_position(g_dwDebugNodeSource);
			pos_dest	= pos_src;
			pos_dest.y	+= 10.0f;

			Level().debug_renderer().draw_aabb(pos_src,0.35f,0.35f,0.35f,color_xrgb(0,0,255));
			Level().debug_renderer().draw_line(Fidentity,pos_src,pos_dest,color_xrgb(0,0,255));
		}

		if (ai().level_graph().valid_vertex_id(g_dwDebugNodeDest)) {
			pos_src		= ai().level_graph().vertex_position(g_dwDebugNodeDest);
			pos_dest	= pos_src;
			pos_dest.y	+= 10.0f;

			Level().debug_renderer().draw_aabb(pos_src,0.35f,0.35f,0.35f,color_xrgb(255,0,0));
			Level().debug_renderer().draw_line(Fidentity,pos_src,pos_dest,color_xrgb(255,0,0));
		}
	}
}
#endif
#endif

#endif // AI_COMPILER
#endif // DEBUG