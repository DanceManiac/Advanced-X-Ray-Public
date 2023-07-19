#include "pch_script.h"
#include "f1.h"
#include "Battery.h"
#include "AnomalyDetector.h"
#include "RepairKit.h"
#include "AntigasFilter.h"

CF1::CF1(void) {
}

CF1::~CF1(void) {
}

using namespace luabind;

#pragma optimize("s",on)
void CF1::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CF1,CGameObject>("CF1")
			.def(constructor<>()),
		class_<CBattery, CGameObject>("CBattery")
			.def(constructor<>()),
		class_<CAntigasFilter, CGameObject>("CAntigasFilter")
			.def(constructor<>()),
		class_<CRepairKit, CGameObject>("CRepairKit")
			.def(constructor<>()),
		class_<CDetectorAnomaly, CGameObject>("CDetectorAnomaly")
			.def(constructor<>())
	];
}
