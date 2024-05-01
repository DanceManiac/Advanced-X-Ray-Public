#pragma once

#include "../xrEngine/CustomHUD.h"
#include "HitMarker.h"
#include "UI.h"

class CContextMenu;
class CHUDTarget;


class CHUDManager :
	public CCustomHUD
{
	friend class CUI;
private:
	CUI*					pUI;
	CHitMarker				HitMarker;
	CHUDTarget*				m_pHUDTarget;
	bool					b_online;
public:
							CHUDManager			();
	virtual					~CHUDManager		();
	virtual		void		OnEvent				(EVENT E, u64 P1, u64 P2);

	virtual		void		Load				();
	
	virtual		void		Render_First		();
	virtual		void		Render_Last			();	   
	virtual		void		OnFrame				();

	virtual		void		RenderUI			();

	virtual		IC CUI*		GetUI				(){return pUI;}

				void		Hit					(int idx, float power, const Fvector& dir);
				void		net_Relcase			( CObject* obj );
	//текущий предмет на который смотрит HUD
	collide::rq_result&		GetCurrentRayQuery	();


	//устанвка внешнего вида прицела в зависимости от текущей дисперсии
	void					SetCrosshairDisp	(float dispf, float disps = 0.f);
	void					ShowCrosshair		(bool show);

	void					SetHitmarkType		(LPCSTR tex_name);
	virtual void			OnScreenResolutionChanged();
	virtual void			OnDisconnected		();
	virtual void			OnConnected			();
	virtual	void			RenderActiveItemUI	();
	virtual	bool			RenderActiveItemUIQuery();
	//Lain: added
				void		SetRenderable       (bool renderable) { m_Renderable = renderable; }
				bool		IsRenderable        () { return m_Renderable; }
private:
	bool					m_Renderable;
};
