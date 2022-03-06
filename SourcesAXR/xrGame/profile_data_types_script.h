#pragma once

#include "mixed_delegate.h"

namespace gamespy_profile {
	typedef mixed_delegate<void(bool, char const*), store_operation_cb_tag>	store_operation_cb;
}

typedef gamespy_profile::store_operation_cb	gamespy_profile_store_operation_cb;

add_to_type_list(gamespy_profile_store_operation_cb)
#undef script_type_list
#define script_type_list save_type_list(gamespy_profile_store_operation_cb)

struct profile_data_script_registrator
{
	DECLARE_SCRIPT_REGISTER_FUNCTION
}; //struct profile_data_script_registrator

add_to_type_list(profile_data_script_registrator)
#undef script_type_list
#define script_type_list save_type_list(profile_data_script_registrator)