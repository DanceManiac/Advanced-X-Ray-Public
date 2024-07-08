////////////////////////////////////////////////////////////////////////////
//	Module 		: SleepingBag.cpp
//	Created 	: 01.10.2023
//  Modified 	: 01.10.2023
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Sleeping bag
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch_script.h"
#include "SleepingBag.h"

#include "ai_space.h"
#include "script_engine.h"

CSleepingBag::CSleepingBag()
{
}

CSleepingBag::~CSleepingBag()
{
}

void CSleepingBag::StartSleep()
{
	luabind::functor<void> funct;
	if (ai().script_engine().functor("mfs_functions.sleeping_bag_sleep", funct))
		funct();
}