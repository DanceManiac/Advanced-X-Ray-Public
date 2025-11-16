////////////////////////////////////////////////////////////////////////////
//	Module 		: actor_script.cpp
//	Created 	: 17.01.2008
//  Modified 	: 17.01.2008
//	Author		: Dmitriy Iassenev
//	Description : actor script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "actor.h"
#include "level_changer.h"
#include "ActorCondition.h"

#pragma optimize("s",on)
void CActor::script_register(lua_State *L)
{
    using namespace luabind;

    module(L)
        [
            class_<CActor, CGameObject>("CActor")
            .def(constructor<>())
            .def("conditions",          &CActor::conditions)

            .def("actor_moving_state",  &CActor::get_state)
            .def("is_actor_normal",     &CActor::is_actor_normal)
            .def("is_actor_crouch",     &CActor::is_actor_crouch)
            .def("is_actor_creep",      &CActor::is_actor_creep)
            .def("is_actor_climb",      &CActor::is_actor_climb)
            .def("is_actor_walking",    &CActor::is_actor_walking)
            .def("is_actor_running",    &CActor::is_actor_running)
            .def("is_actor_sprinting",  &CActor::is_actor_sprinting)
            .def("is_actor_crouching",  &CActor::is_actor_crouching)
            .def("is_actor_creeping",   &CActor::is_actor_creeping)
            .def("is_actor_climbing",   &CActor::is_actor_climbing)
            .def("is_actor_moving",     &CActor::is_actor_moving),

		class_<CLevelChanger,CGameObject>("CLevelChanger")
			.def(constructor<>())
	];
}
