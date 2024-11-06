#pragma once
#include "../state.h"

template<typename _Object>
class CStateBloodsuckerVampireHide : public CState<_Object> {
	typedef CState<_Object>		inherited;
	typedef CState<_Object>*	state_ptr;

public:
						CStateBloodsuckerVampireHide	(_Object *obj);

	virtual	void		reselect_state					();
	virtual void		setup_substates					();
	virtual bool		check_completion				();
	virtual void		remove_links					(CObject* object_) { inherited::remove_links(object_);}
};

#include "bloodsucker_vampire_hide_inline.h"
