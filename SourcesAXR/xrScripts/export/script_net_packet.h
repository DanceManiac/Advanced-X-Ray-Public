////////////////////////////////////////////////////////////////////////////
//	Module 		: script_net packet.h
//	Created 	: 06.02.2004
//  Modified 	: 24.06.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script net packet class
////////////////////////////////////////////////////////////////////////////
#pragma once
#include "script_export_space.h"
class NET_Packet;

using CScriptNetPacket = class_exporter_api<NET_Packet>;
add_to_type_list(CScriptNetPacket)
#undef script_type_list
#define script_type_list save_type_list(CScriptNetPacket)

__declspec(dllexport) extern u16	script_server_object_version();