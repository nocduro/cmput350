#include <sc2api/sc2_api.h>
#include "state.h"
#include "util.h"

// Run this state until we've successfully built a refinery,
// then move onto builing an army
sc::result GameStart_Refinery::react(const StepEvent& event) {
	auto actions = context<StateMachine>().Actions;
	auto observation = context<StateMachine>().Observation;
    
	// if we have less than 2 refineries, try to build another one
	if (CountUnitType(observation, UNIT_TYPEID::TERRAN_REFINERY) < 2) {
		TryBuildRefinery(actions, observation);
	}

    Units Refinerys = observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_REFINERY));

	if (Refinerys.empty()) {
		return discard_event();
	}

	for (const auto& refinery : Refinerys) {
		if (refinery->build_progress < 1.0) {
			return discard_event();
		}
		if (refinery->assigned_harvesters != refinery->ideal_harvesters) {
			return discard_event();
		}
	}

	std::cout << "transit: " << Refinerys.size() << std::endl;
	return transit<EarlyRushState>();
}

sc::result GameStart_Refinery::react(const UnitCreated& event) {
	auto actions = context<StateMachine>().Actions;
	auto observation = context<StateMachine>().Observation;
	std::cout << "unit built!" << std::endl;
	// if refinery add workers
	if (event.unit->unit_type == UNIT_TYPEID::TERRAN_REFINERY) {
		Units Workers = observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SCV));
		for (size_t i = 0; i < 2; ++i) {
			actions()->UnitCommand(Workers[workers_offset + i], ABILITY_ID::HARVEST_GATHER, event.unit);
		}
		workers_offset += 7;
	}

	return discard_event();
}

sc::result GameStart_BuildBarracks::react(const StepEvent& event) {
	auto actions = context<StateMachine>().Actions;
	auto observation = context<StateMachine>().Observation;

	if (CountUnitType(observation, UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
		TryBuildSupplyDepot(actions, observation);
		return discard_event();
	}
    

	if (CountUnitType(observation, UNIT_TYPEID::TERRAN_BARRACKS) > 1) {
		return transit<GameStart_Refinery>();
	}

	TryBuildBarracks(actions, observation);
	return discard_event();
}

// Build an army until we have 10 marines, then try an 
// early rush attack
sc::result GameStart_BuildArmy::react(const StepEvent& event) {
	auto observation = context<StateMachine>().Observation;
	if (CountUnitType(observation, UNIT_TYPEID::TERRAN_MARINE) > 10) {
		return transit<EarlyRushState>();
	}
	else {
		return discard_event();
	}
}
