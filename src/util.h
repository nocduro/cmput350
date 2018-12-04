#pragma once
#include <sc2api/sc2_api.h>
#include "bot.h"

using namespace sc2;

bool TryBuildSupplyDepot(action_t Actions, observation_t Observation);
bool TryBuildRefinery(action_t Actions, observation_t Observation);
bool TryBuildBarracks(action_t Actions, observation_t Observation);
size_t CountUnitType(observation_t Observation, UNIT_TYPEID unit_type);
bool FarmGas(action_t Actions, observation_t Observation);
bool GetRandomUnit(const Unit*& unit_out, observation_t Observation, UnitTypeID unit_type);