#include "stdafx.h"
#include "import_ses.hpp"

//const char* import_ses::user_name()
//{
//	return			(Core.UserName);
//}

void import_ses::LuaLog(const char* caMessage)
{
	Msg("![Script]: %s", caMessage);
}

LUACORE const char* import_ses::user_name() 
{ 
	return (Core.UserName); 
}