#include "state.h"
#include <sc2api/sc2_api.h>
sc::result EarlyRushState::react(const StepEvent& event) {
	auto Observation = context<StateMachine>().Observation;
	auto Actions = context<StateMachine>().Actions;
	sc2::Units marines = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
	auto game_info = Observation()->GetGameInfo();
	auto enemy_pos = context<MainState>().enemy_pos;
	Actions()->UnitCommand(marines, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[enemy_pos]);
	return transit<BuildTanks>();
}