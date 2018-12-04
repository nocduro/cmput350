#include <sc2api/sc2_api.h>
#include "state.h"
#include "util.h"

sc::result BuildTanks::react(const StepEvent& event) {
	auto Observation = context<StateMachine>().Observation;
	auto Actions = context<StateMachine>().Actions;
	// build a factory if we don't have one

	if (CountUnitType(Observation, UNIT_TYPEID::TERRAN_TECHLAB) < 1) {
		auto barracks = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_BARRACKS)).front();
		const Unit* worker;
		if (GetRandomUnit(worker, Observation, UNIT_TYPEID::TERRAN_SCV)) {
			std::cout << "worker: " << worker->unit_type << std::endl;
			std::cout << "try build techlab" << std::endl;
			std::cout << "barracks: " << barracks->pos.x << std::endl;
			Actions()->UnitCommand(worker, ABILITY_ID::BUILD_TECHLAB_BARRACKS, barracks);
		}
	}

	if (CountUnitType(Observation, UNIT_TYPEID::TERRAN_FACTORY) < 1) {
		TryBuildStructure(Actions, Observation, ABILITY_ID::BUILD_FACTORY);
		return discard_event();
	}

	return discard_event();
}