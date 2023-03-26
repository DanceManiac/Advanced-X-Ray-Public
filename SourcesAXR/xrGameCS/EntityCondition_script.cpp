#include "stdafx.h"
#include "pch_script.h"
#include "ActorCondition.h"
#include "Wound.h"

#pragma optimize("s",on)
void CEntityCondition::script_register(lua_State* L)
{
    using namespace luabind;

    module(L)
    [
        class_<CEntityCondition>("CEntityCondition")
            .def("AddWound",                    &CEntityCondition::AddWound)
            .def("ClearWounds",                 &CEntityCondition::ClearWounds)
            .def("GetWhoHitLastTimeID",         &CEntityCondition::GetWhoHitLastTimeID)
            .def("GetPower",                    &CEntityCondition::GetPower)
            .def("SetPower",                    &CEntityCondition::SetPower)
            .def("GetRadiation",                &CEntityCondition::GetRadiation)
            .def("GetPsyHealth",                &CEntityCondition::GetPsyHealth)
            .def("GetSatiety",                  &CEntityCondition::GetSatiety)
            .def("GetEntityMorale",             &CEntityCondition::GetEntityMorale)
            .def("GetHealthLost",               &CEntityCondition::GetHealthLost)
            .def("IsLimping",                   &CEntityCondition::IsLimping)
            .def("ChangeSatiety",               &CEntityCondition::ChangeSatiety)
            .def("ChangeHealth",                &CEntityCondition::ChangeHealth)
            .def("ChangePower",                 &CEntityCondition::ChangePower)
            .def("ChangeRadiation",             &CEntityCondition::ChangeRadiation)
            .def("ChangePsyHealth",             &CEntityCondition::ChangePsyHealth)
            .def("ChangeAlcohol",               &CEntityCondition::ChangeAlcohol)
            .def("SetMaxPower",                 &CEntityCondition::SetMaxPower)
            .def("GetMaxPower",                 &CEntityCondition::GetMaxPower)
            .def("ChangeEntityMorale",          &CEntityCondition::ChangeEntityMorale)
            .def("ChangeBleeding",              &CEntityCondition::ChangeBleeding)
            .def("BleedingSpeed",               &CEntityCondition::BleedingSpeed)
    ];
};

void CActorCondition::script_register(lua_State* L)
{
    using namespace luabind;

    module(L)
    [
        class_<CWound>("CWound")
            .def("TypeSize",                    &CWound::TypeSize)
            .def("BloodSize",                   &CWound::BloodSize)
            .def("AddHit",                      &CWound::AddHit)
            .def("Incarnation",                 &CWound::Incarnation)
            .def("TotalSize",                   &CWound::TotalSize)
            .def("SetBoneNum",                  &CWound::SetBoneNum)
            .def("GetBoneNum",                  &CWound::GetBoneNum)
            .def("GetParticleBoneNum",          &CWound::GetParticleBoneNum)
            .def("SetParticleBoneNum",          &CWound::SetParticleBoneNum)
            .def("SetDestroy",                  &CWound::SetDestroy)
            .def("GetDestroy",                  &CWound::GetDestroy),

        class_<CActorCondition, CEntityCondition>("CActorCondition")
            .def("WoundForEach",                &CActorCondition::WoundForEach)
            .def("GetSatiety",                  &CActorCondition::GetSatiety)
            .def("IsLimping",                   &CActorCondition::IsLimping)
            .def("IsCantWalk",                  &CActorCondition::IsCantWalk)
            .def("IsCantWalkWeight",            &CActorCondition::IsCantWalkWeight)
            .def("IsCantSprint",                &CActorCondition::IsCantSprint)
            .def_readwrite("m_MaxWalkWeight",   &CActorCondition::m_MaxWalkWeight)
    ];
};