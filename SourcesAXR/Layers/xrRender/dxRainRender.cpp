#include "stdafx.h"
#include "dxRainRender.h"

#include "../../xrEngine/Rain.h"
#include "../../xrEngine/x_ray.h"

int current_items;

dxRainRender::dxRainRender()
{
	current_items = 0;

	if (!bWinterMode)
	{
		IReader* F = FS.r_open("$game_meshes$", "dm\\rain.dm");
		VERIFY3(F, "Can't open file.", "dm\\rain.dm");

		DM_Drop = ::RImplementation.model_CreateDM(F);

		//
#if defined(USE_DX11)
		if (ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON))
			SH_Rain.create("effects\\rain_screen_space", "fx\\fx_rain");
		else
#endif
			SH_Rain.create("effects\\rain", "fx\\fx_rain");

		hGeom_Rain.create(FVF::F_LIT, RCache.Vertex.Buffer(), RCache.QuadIB);
		hGeom_Drops.create(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, RCache.Vertex.Buffer(), RCache.Index.Buffer());

#if defined(USE_DX11)
		if (RImplementation.o.ssfx_rain)
			SH_Splash.create("effects\\rain_splash", "fx\\fx_rain");
#endif

		FS.r_close(F);
	}
	else
	{
		IReader* F = FS.r_open("$game_meshes$", "dm\\snow.dm");
		VERIFY3(F, "Can't open file.", "dm\\snow.dm");

		DM_Drop = ::RImplementation.model_CreateDM(F);

		//
		SH_Rain.create("effects\\snow", "fx\\fx_snow");
		hGeom_Rain.create(FVF::F_LIT, RCache.Vertex.Buffer(), RCache.QuadIB);
		hGeom_Drops.create(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, RCache.Vertex.Buffer(), RCache.Index.Buffer());

		FS.r_close(F);
	}
}

dxRainRender::~dxRainRender()
{
	::RImplementation.model_Delete(DM_Drop);
}

void dxRainRender::Copy(IRainRender &_in)
{
	*this = *(dxRainRender*)&_in;
}

#include "../../xrEngine/iGame_persistent.h"

void dxRainRender::Render(CEffect_Rain &owner)
{
	float	factor				= g_pGamePersistent->Environment().CurrentEnv->rain_density;

	if (factor<EPS_L)
		return;

	float	particles_multiplier	= bWinterMode ? 2.0f : 1.0f;
	
	float	_drop_len			= owner.drop_length;
	float	_drop_width			= owner.drop_width;
	float	_drop_speed			= 1.0f;
	ref_shader&	_splash_SH		= DM_Drop->shader;
	static shared_str s_shader_setup	= "ssfx_rain_setup";

	float	wind_direction		= 0.0f;
	float	wind_power			= 0.0f;
	if (g_pGamePersistent->Environment().CurrentEnv->wind_velocity)
	{
		wind_direction	= g_pGamePersistent->Environment().CurrentEnv->wind_direction;
		wind_power		= g_pGamePersistent->Environment().CurrentEnv->wind_velocity;
	}

	Fvector	wind_vector;
	wind_vector.set(_sin(wind_direction), 0.0f, _cos(wind_direction));

	// SSS Rain shader is available
#if defined(USE_DX11)
	if (ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON) && RImplementation.o.ssfx_rain && !bWinterMode)
	{
		_drop_len	= ps_ssfx_rain_1.x;
		_drop_width	= ps_ssfx_rain_1.y;
		_drop_speed	= ps_ssfx_rain_1.z;
		_splash_SH	= SH_Splash;
	}
#endif

	u32	desired_items	= iFloor(particles_multiplier * 0.01f * (1.f + factor * 99.0f) * m_imax_desired_items);

	// Get to the desired items
	if (current_items < desired_items)
		current_items += desired_items - current_items;

	// visual
	float		factor_visual	= factor/2.f+.5f;
	Fvector3	f_rain_color	= g_pGamePersistent->Environment().CurrentEnv->rain_color;
	u32			u_rain_color	= color_rgba_f(f_rain_color.x,f_rain_color.y,f_rain_color.z,factor_visual);

	// born _new_ if needed
	float	b_radius_wrap_sqr	= _sqr((m_fsource_radius * 1.5f));
	if (owner.items.size() < current_items)
	{
		while (owner.items.size() < current_items)
		{
			CEffect_Rain::Item	one;
			owner.Born(one, m_fsource_radius, _drop_speed);
			owner.items.push_back(one);
		}
	}

	// build source plane
	Fplane	src_plane;
	Fvector	norm	= {0.f,-1.f,0.f};
	Fvector	upper;
	upper.set(Device.vCameraPosition.x,Device.vCameraPosition.y+m_fsource_offset,Device.vCameraPosition.z);
	src_plane.build(upper,norm);

	// perform update
	u32			vOffset;
	FVF::LIT	*verts		= (FVF::LIT*)RCache.Vertex.Lock(desired_items*4,hGeom_Rain->vb_stride,vOffset);
	FVF::LIT	*start		= verts;
	const Fvector&	vEye	= Device.vCameraPosition;
	
	for (u32 I = 0; I < current_items; I++)
	{
		// physics and time control
		CEffect_Rain::Item&	one	= owner.items.at(I);

		if (one.dwTime_Hit < Device.dwTimeGlobal)
		{
			owner.Hit(one.Phit);
			if (current_items > desired_items)
				current_items--; // Hit something
		}

		if (one.dwTime_Life < Device.dwTimeGlobal)
		{
			owner.Born(one, m_fsource_radius, _drop_speed);
			if (current_items > desired_items)
				current_items--; // Out of life
		}

		float	dt	= Device.fTimeDelta;

		if (bWinterMode)
		{
			Fvector	wind_effect;
			wind_effect.mul(wind_vector, wind_power * 0.5f * dt);

			Fvector	movement;
			movement.set(one.D.x + wind_effect.x, one.D.y, one.D.z + wind_effect.z);
			one.P.mad(movement, one.fSpeed * (0.75 + (wind_power * 0.2f)) * dt);
		}
		else
			one.P.mad(one.D, one.fSpeed * dt);

		Device.Statistic->TEST1.Begin();
		Fvector	wdir;
		wdir.set(one.P.x-vEye.x,0,one.P.z-vEye.z);
		float	wlen	= wdir.square_magnitude();
		if (wlen>b_radius_wrap_sqr)
		{
			wlen	= _sqrt(wlen);
			if ((one.P.y-vEye.y)<m_fsink_offset)
			{
				one.invalidate();
			}
			else
			{
				Fvector	inv_dir, src_p;
				inv_dir.invert(one.D);
				wdir.div(wlen);
				one.P.mad(one.P, wdir, -(wlen+m_fsource_radius));
				if (src_plane.intersectRayPoint(one.P,inv_dir,src_p))
				{
					float	dist_sqr	= one.P.distance_to_sqr(src_p);
					float	height		= m_fmax_distance;
					if (owner.RayPick(src_p,one.D,height,collide::rqtBoth))
					{	
						if (_sqr(height)<=dist_sqr)
							one.invalidate();
						else
							owner.RenewItem(one,height-_sqrt(dist_sqr),TRUE);
					}
					else
					{
						owner.RenewItem(one,m_fmax_distance-_sqrt(dist_sqr),FALSE);
					}
				}
				else
				{
					one.invalidate();
				}
			}
		}
		Device.Statistic->TEST1.End();

		// Build line
		Fvector&	pos_head	= one.P;
		Fvector		pos_trail;

		if (!bWinterMode)
		{
			if (ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON))
				pos_trail.mad(pos_head, one.D, -_drop_len * factor_visual);
			else
				pos_trail.mad(pos_head, one.D, -owner.drop_length * factor_visual);
		}
		else
			pos_trail.mad(pos_head, one.D, -owner.drop_length * 5.5f);

		// Culling
		Fvector	sC, lineD;
		float	sR;
		sC.sub(pos_head,pos_trail);
		lineD.normalize(sC);
		sC.mul(.5f);
		sR	= sC.magnitude();
		sC.add(pos_trail);
		if (!::Render->ViewBase.testSphere_dirty(sC,sR))
			continue;

		constexpr Fvector2 UV[2][4]={
			{{0,1},{0,0},{1,1},{1,0}},
			{{1,0},{1,1},{0,0},{0,1}}
		};

		// Everything OK - build vertices
		Fvector	P, lineTop, camDir;
		camDir.sub(sC,vEye);
		camDir.normalize();
		lineTop.crossproduct(camDir,lineD);
		float	w	= ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON) && !bWinterMode ? _drop_width : owner.drop_width;
		u32		s	= one.uv_set;
		P.mad(pos_trail,lineTop,-w);	verts->set(P,u_rain_color,UV[s][0].x,UV[s][0].y);	verts++;
		P.mad(pos_trail,lineTop,w);		verts->set(P,u_rain_color,UV[s][1].x,UV[s][1].y);	verts++;
		P.mad(pos_head, lineTop,-w);	verts->set(P,u_rain_color,UV[s][2].x,UV[s][2].y);	verts++;
		P.mad(pos_head, lineTop,w);		verts->set(P,u_rain_color,UV[s][3].x,UV[s][3].y);	verts++;
	}
	
	u32	vCount	= (u32)(verts-start);
	RCache.Vertex.Unlock(vCount,hGeom_Rain->vb_stride);

	// Render if needed
	if (vCount)
	{
		RCache.set_CullMode(CULL_NONE);
		RCache.set_xform_world(Fidentity);
		RCache.set_Shader(SH_Rain);
		RCache.set_Geometry(hGeom_Rain);
		RCache.Render(D3DPT_TRIANGLELIST,vOffset,0,vCount,0,vCount/2);
		RCache.set_CullMode(CULL_CCW);
		RCache.set_c(s_shader_setup, ps_ssfx_rain_2);
	}

	// Particles
	CEffect_Rain::Particle*	P	= owner.particle_active;
	if (0==P)	return;

	{
		float	dt		= Device.fTimeDelta;
		_IndexStream& _IS	= RCache.Index;

		if (ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON) && !bWinterMode)
		{
			RCache.set_Shader(_splash_SH);
			RCache.set_c(s_shader_setup, ps_ssfx_rain_3);
		}
		else
		{
			RCache.set_Shader(DM_Drop->shader);
		}

		Fmatrix	mXform, mScale;
		u32		pcount	= 0;
		u32		v_offset, i_offset;
		u32		vCount_Lock	= m_iparticles_cache*DM_Drop->number_vertices;
		u32		iCount_Lock	= m_iparticles_cache*DM_Drop->number_indices;
		IRender_DetailModel::fvfVertexOut* v_ptr= (IRender_DetailModel::fvfVertexOut*)RCache.Vertex.Lock(vCount_Lock, hGeom_Drops->vb_stride, v_offset);
		u16*	i_ptr	= _IS.Lock(iCount_Lock, i_offset);
		
		while (P)
		{
			CEffect_Rain::Particle*	next	= P->next;

			P->time	-= dt;
			if (P->time<0)
			{
				owner.p_free(P);
				P	= next;
				continue;
			}

			if (::Render->ViewBase.testSphere_dirty(P->bounds.P, P->bounds.R))
			{
				float	scale	= P->time/m_fparticles_time;
				mScale.scale(scale,scale,scale);
				mXform.mul_43(P->mXForm,mScale);

				DM_Drop->transfer(mXform,v_ptr,u_rain_color,i_ptr,pcount*DM_Drop->number_vertices);
				v_ptr	+= DM_Drop->number_vertices;
				i_ptr	+= DM_Drop->number_indices;
				pcount++;

				if (pcount >= m_iparticles_cache)
				{
					u32	dwNumPrimitives	= iCount_Lock/3;
					RCache.Vertex.Unlock(vCount_Lock,hGeom_Drops->vb_stride);
					_IS.Unlock(iCount_Lock);
					RCache.set_Geometry(hGeom_Drops);
					RCache.Render(D3DPT_TRIANGLELIST,v_offset,0,vCount_Lock,i_offset,dwNumPrimitives);

					v_ptr	= (IRender_DetailModel::fvfVertexOut*)RCache.Vertex.Lock(vCount_Lock,hGeom_Drops->vb_stride,v_offset);
					i_ptr	= _IS.Lock(iCount_Lock,i_offset);

					pcount	= 0;
				}
			}

			P	= next;
		}

		vCount_Lock		= pcount*DM_Drop->number_vertices;
		iCount_Lock		= pcount*DM_Drop->number_indices;
		u32	dwNumPrimitives	= iCount_Lock/3;
		RCache.Vertex.Unlock(vCount_Lock,hGeom_Drops->vb_stride);
		_IS.Unlock(iCount_Lock);
		if (pcount)
		{
			RCache.set_Geometry(hGeom_Drops);
			RCache.Render(D3DPT_TRIANGLELIST,v_offset,0,vCount_Lock,i_offset,dwNumPrimitives);
		}
	}
}

const Fsphere& dxRainRender::GetDropBounds() const
{
	return DM_Drop->bv_sphere;
}