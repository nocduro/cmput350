#include <sc2api/sc2_api.h>
#include "state.h"
#include "util.h"

// might be useful for scouting:
// https://www.boost.org/doc/libs/1_50_0/libs/statechart/doc/tutorial.html#PostingEvents

sc::result MainState::react(const CommandCenterIdle& e) {
	// only build more SCVs when we have fewer than 16 of them
	auto Observation = context<StateMachine>().Observation;
	if (CountUnitType(Observation, UNIT_TYPEID::TERRAN_SCV) < 16) {
		context<StateMachine>().Actions()->UnitCommand(e.unit, ABILITY_ID::TRAIN_SCV);
	}
	return discard_event();
}

sc::result MainState::react(const BarracksIdle& event) {
    auto Observation = context<StateMachine>().Observation;
    auto actions = context<StateMachine>().Actions;
    if (Observation()->GetFoodUsed() >= Observation()->GetFoodCap()) {
        TryBuildSupplyDepot(actions, Observation);
    }
	context<StateMachine>().Actions()->UnitCommand(event.unit, ABILITY_ID::TRAIN_MARINE);
	return discard_event();
}

sc::result MainState::react(const SCVIdle& event) {
	return discard_event();
}

sc::result MainState::react(const MarineIdle& event) {
	return discard_event();
}
