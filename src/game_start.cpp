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

	if (Refinerys.size() < 2) {
		return discard_event();
	}

	for (const auto& refinery : Refinerys) {
		if (refinery->build_progress < 1.0) {
			return discard_event();
		}
	}

	std::cout << "transit: " << Refinerys.size() << std::endl;
	return transit<GameStart_BuildArmy>();
}

sc::result GameStart_Refinery::react(const BuildingConstructed& event) {
	auto actions = context<StateMachine>().Actions;
	auto observation = context<StateMachine>().Observation;
	std::cout << "building built!" << std::endl;
	// if refinery add workers
	if (event.unit->unit_type == UNIT_TYPEID::TERRAN_REFINERY) {
		std::cout << "refinery building built" << std::endl;
		Units Workers = observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SCV));
		for (size_t i = 0; i < 3; ++i) {
			actions()->UnitCommand(Workers[workers_offset + i], ABILITY_ID::HARVEST_GATHER, event.unit);
		}
		workers_offset += 3;
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
		std::cout << "len of start pos: " << observation()->GetGameInfo().enemy_start_locations.size() << std::endl;
		return transit<GameStart_Refinery>();
	}

	TryBuildBarracks(actions, observation);
	return discard_event();
}

// Build an army until we have 10 marines, then try an 
// early rush attack
sc::result GameStart_BuildArmy::react(const StepEvent& event) {
	auto observation = context<StateMachine>().Observation;

	// wait for scouter to find enemy base
	if (scouter == nullptr || scouter->is_alive) {
		return discard_event();
	}

	context<MainState>().enemy_pos = scouting;
	std::cout << "enemy position index: " << scouting << std::endl;
	

	if (CountUnitType(observation, UNIT_TYPEID::TERRAN_MARINE) > 12) {
		return transit<EarlyRushState>();
	}
	else {
		return discard_event();
	}
}

sc::result GameStart_BuildArmy::react(const MarineIdle& event) {
	auto Actions = context<StateMachine>().Actions;
	auto Observation = context<StateMachine>().Observation;
	auto game_info = Observation()->GetGameInfo();
	if (scouter == nullptr) {
		scouter = event.unit;
		Actions()->UnitCommand(scouter, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[scouting]);
	}
	else if (scouter == event.unit) {
		// move scouter to next position
		++scouting;
		//move marine to enemy base location 
		Actions()->UnitCommand(scouter, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[scouting]);
	}
	return discard_event();
}
