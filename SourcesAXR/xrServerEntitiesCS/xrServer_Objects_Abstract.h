////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects_Abstract.h
//	Created 	: 19.09.2002
//  Modified 	: 18.06.2004
//	Author		: Oles Shyshkovtsov, Alexander Maksimchuk, Victor Reutskiy and Dmitriy Iassenev
//	Description : Server objects
////////////////////////////////////////////////////////////////////////////

#ifndef xrServer_Objects_AbstractH
#define xrServer_Objects_AbstractH

#pragma pack(push,4)
#include "xrServer_Space.h"
#	include "../xrCDB/xrCDB.h"
#include "ShapeData.h"
#include "gametype_chooser.h"

class NET_Packet;
class CDUInterface;

#ifndef _EDITOR
    #ifndef XRGAME_EXPORTS
		#include "../../engine/xrSound/Sound.h"
    #endif
#endif

#include "xrEProps.h"

#ifndef XRGAME_EXPORTS
	#include "..\..\Include/xrRender\DrawUtils.h"
#else
	#include "..\Include/xrRender\DrawUtils.h"
#endif
#pragma warning(push)
#pragma warning(disable:4005)



class ISE_Shape{
public:
	virtual void 			assign_shapes			(CShapeData::shape_def* shapes, u32 cnt)=0;
};

SERVER_ENTITY_DECLARE_BEGIN0(CSE_Visual)
    void 					OnChangeVisual			(PropValue* sender);  
    void 					OnChangeAnim			(PropValue* sender);  

public:
	shared_str						visual_name;
	shared_str						startup_animation;
	enum{
		flObstacle					= (1<<0)
	};
	Flags8							flags;
public:
									CSE_Visual				(LPCSTR name=0);
	virtual							~CSE_Visual				();

	void							visual_read				(NET_Packet& P, u16 version);
	void							visual_write			(NET_Packet& P);

    void							set_visual				(LPCSTR name, bool load=true);
	LPCSTR							get_visual				() const {return *visual_name;};
#ifndef XRGAME_EXPORTS
	virtual void					FillProps				(LPCSTR pref, PropItemVec &items);
#endif // #ifndef XRGAME_EXPORTS

	virtual CSE_Visual* 	visual					() = 0;
};
add_to_type_list(CSE_Visual)
#define script_type_list save_type_list(CSE_Visual)

SERVER_ENTITY_DECLARE_BEGIN0(CSE_Motion)
	void 	OnChangeMotion	(PropValue* sender);  
public:
	shared_str						motion_name;
public:
									CSE_Motion 				(LPCSTR name=0);
	virtual							~CSE_Motion				();

	void							motion_read				(NET_Packet& P);
	void							motion_write			(NET_Packet& P);

    void							set_motion				(LPCSTR name);
	LPCSTR							get_motion				() const {return *motion_name;};

#ifndef XRGAME_EXPORTS
	virtual void					FillProps				(LPCSTR pref, PropItemVec &items);
#endif // #ifndef XRGAME_EXPORTS

	virtual CSE_Motion* 	motion					() = 0;
};
add_to_type_list(CSE_Motion)
#define script_type_list save_type_list(CSE_Motion)

struct ISE_AbstractLEOwner{
	virtual void			get_bone_xform			(LPCSTR name, Fmatrix& xform) = 0;
};

#pragma pack(push,1)
struct visual_data {
	Fmatrix		matrix;
	CSE_Visual	*visual;
}; // struct visual_data
#pragma pack(pop)

struct ISE_Abstract {
public:
	enum {
		flUpdateProperties			= u32(1 << 0),
		flVisualChange				= u32(1 << 1),
		flVisualAnimationChange		= u32(1 << 2),
		flMotionChange				= u32(1 << 3),
		flVisualAnimationPauseChange= u32(1 << 4),
	};
	Flags32							m_editor_flags;
	IC	void						set_editor_flag			(u32 mask)	{m_editor_flags.set	(mask,TRUE);}

public:
	virtual void			Spawn_Write				(NET_Packet &tNetPacket, BOOL bLocal) = 0;
	virtual BOOL			Spawn_Read				(NET_Packet &tNetPacket) = 0;
#ifndef XRGAME_EXPORTS
	virtual void			FillProp				(LPCSTR pref, PropItemVec &items) = 0;
	virtual void 			on_render				(CDUInterface* du, ISE_AbstractLEOwner* owner, bool bSelected, const Fmatrix& parent,int priority, bool strictB2F) = 0;
	virtual	visual_data*	visual_collection		() const = 0;
	virtual	u32				visual_collection_size	() const = 0;
#endif // #ifndef XRGAME_EXPORTS
	virtual LPCSTR			name					() const = 0;
	virtual void			set_name				(LPCSTR) = 0;
	virtual LPCSTR			name_replace			() const = 0;
	virtual void			set_name_replace		(LPCSTR) = 0;
	virtual Fvector&		position				() = 0;
	virtual Fvector&		angle					() = 0;
	virtual Flags16&		flags					() = 0;
	virtual ISE_Shape*  	shape					() = 0;
	virtual CSE_Visual* 	visual					() = 0;
	virtual CSE_Motion* 	motion					() = 0;
	virtual bool			validate				() = 0;
};

#pragma warning(pop)

#pragma pack(pop)
#endif // xrServer_Objects_AbstractH