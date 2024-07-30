////////////////////////////////////////////////////////////////////////////
//	Created		: 23.06.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef DOORS_MANAGER_H_INCLUDED
#define DOORS_MANAGER_H_INCLUDED

#include <boost/noncopyable.hpp>
#include "quadtree.h"
#include "doors.h"

class CPhysicObject;

namespace doors {

class actor;
class CDoor;

class manager : private boost::noncopyable {
public:
				manager					( Fbox const& bounding_box );
				~manager				( );
	CDoor*		register_door			( CPhysicObject& object );
	void		unregister_door			(CDoor*& door );
	void		on_door_is_open			(CDoor* door );
	void		on_door_is_closed		(CDoor* door );
	bool		is_door_locked			(CDoor const* door ) const;
	void		lock_door				(CDoor* door );
	void		unlock_door				(CDoor* door );
	bool		is_door_blocked			(CDoor* door ) const;

public:
	bool		actualize_doors_state	( actor& npc, float average_speed );

private:
	friend class doors::actor;
	void		open_door				(CDoor* door );
	void		close_door				(CDoor* door );
//	void		check_bug_door			( ) const;

private:
	typedef CQuadTree<CDoor>				doors_tree_type;

private:
	doors_tree_type	m_doors;
	doors_type		m_nearest_doors;
}; // class manager

} // namespace doors

#endif // #ifndef DOORS_MANAGER_H_INCLUDED