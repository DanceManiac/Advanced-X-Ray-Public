#pragma once
#include "hud_item_object.h"
#include "HudSound.h"
#include "ai_sounds.h"

struct dContact;
struct SGameMtl;
class CMissile : public CHudItemObject
{
	typedef CHudItemObject inherited;
public:
	enum EMissileStates
	{
		eThrowStart = eLastBaseState + 1,
		eReady,
		eThrow,
		eThrowEnd,
	};

							CMissile					();
	virtual					~CMissile					();

	virtual BOOL			AlwaysTheCrow				()				{ return TRUE; }
	virtual void			render_item_ui				();
	virtual bool			render_item_ui_query		();

	virtual void			reinit						();
	virtual CMissile*		cast_missile				()				{return this;}

	virtual void 			Load						(LPCSTR section);
	virtual BOOL 			net_Spawn					(CSE_Abstract* DC);
	virtual void 			net_Destroy					();

	virtual void 			UpdateCL					();
	virtual void 			shedule_Update				(u32 dt);

	virtual void 			OnH_A_Chield				();
	virtual void 			OnH_B_Independent			(bool just_before_destroy);

	virtual void 			OnEvent						(NET_Packet& P, u16 type);

	virtual void 			OnAnimationEnd				(u32 state);
	virtual void			OnMotionMark				(u32 state, const motion_marks& M);

	virtual void 			Show();
	virtual void 			Hide();

	virtual void 			Throw();
	virtual void 			Destroy();

	virtual bool 			Action						(s32 cmd, u32 flags);

//.	IC u32		 			State						()				{return m_state;}
	virtual void 			State						(u32 state, u32 oldState = 0);
	virtual void 			OnStateSwitch				(u32 S, u32 oldState = 0);
	virtual void			PlayAnimIdle				();
	virtual void			GetBriefInfo				(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count);

protected:
	virtual void			UpdateXForm						();
	void					UpdatePosition					(const Fmatrix& trans);
	void					spawn_fake_missile				();

	//инициализация если вещь в активном слоте или спрятана на OnH_B_Chield
	virtual void			OnActiveItem		();
	virtual void			OnHiddenItem		();

	//для сети
	virtual void			net_Relcase			(CObject* O );
protected:

	bool					m_throw;
	
	//время уничтожения
	u32						m_dwDestroyTime;
	u32						m_dwDestroyTimeMax;

	Fvector					m_throw_direction;
	Fmatrix					m_throw_matrix;

	CMissile*				m_fake_missile;

	//параметры броска
	
	float m_fMinForce, m_fConstForce, m_fMaxForce, m_fForceGrowSpeed;
//private:
	bool					m_constpower;
	float					m_fThrowForce;

	bool					m_bIsContactGrenade;
	CGameObject*			m_pOwner;
protected:
	//относительная точка и направление вылета гранаты
	Fvector					m_vThrowPoint;
	Fvector					m_vThrowDir;
	//для HUD
	Fvector					m_vHudThrowPoint;
	Fvector					m_vHudThrowDir;

	//звук анимации "играния"
	ESoundTypes				m_eSoundPlaying;

	bool					m_throwMotionMarksAvailable;
protected:
			void			setup_throw_params		();
public:
	virtual void			activate_physic_shell	();
	virtual void			setup_physic_shell		();
	virtual void			create_physic_shell		();
	IC		void			set_destroy_time		(u32 delta_destroy_time) {m_dwDestroyTime = delta_destroy_time + Device.dwTimeGlobal;}

protected:
	u32						m_ef_weapon_type;

public:
	virtual u32				ef_weapon_type			() const;
	IC		u32				destroy_time			() const {return m_dwDestroyTime;};
	static	void			ExitContactCallback		(bool& do_colide,bool bo1,dContact& c,SGameMtl *material_1,SGameMtl* material_2);
	virtual u16				bone_count_to_synchronize	() const;
};