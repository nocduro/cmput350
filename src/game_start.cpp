#include "state.h"
#include "util.h"

// Run this state until we've successfully built a refinery,
// then move onto builing an army
sc::result GameStart_Refinery::react(const StepEvent& event) {
	auto actions = context<StateMachine>().Actions;
	auto observation = context<StateMachine>().Observation;
	if (TryBuildRefinery(actions, observation)) {
		return transit<GameStart_BuildArmy>();
	}
	return discard_event();
}

// Build an army until we have 10 marines, then try an 
// early rush attack
sc::result GameStart_BuildArmy::react(const StepEvent& event) {
	auto observation = context<StateMachine>().Observation();
	int num_marines = 0;
	Units my_units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto unit : my_units) {
		if (unit->unit_type == UNIT_TYPEID::TERRAN_MARINE) {
			++num_marines;
		}
	}
	if (num_marines > 10) {
		return transit<EarlyRushState>();
	}
	else {
		return discard_event();
	}
}