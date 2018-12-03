#include <sc2api/sc2_api.h>
#include "state.h"
#include "util.h"

// Run this state until we've successfully built a refinery,
// then move onto builing an army
sc::result GameStart_Refinery::react(const StepEvent& event) {
	auto actions = context<StateMachine>().Actions;
	auto observation = context<StateMachine>().Observation;
    Units Refinerys = observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_REFINERY));
        
    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_REFINERY) >= 1){
        if(FarmGas(actions,observation)){
            std::cout <<"done farming gas" << std::endl;
        }
    }
    
    
    
    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_REFINERY) > 1){
        for (const auto& refinery: Refinerys){
            if (refinery->assigned_harvesters == refinery->ideal_harvesters){
                return transit<GameStart_BuildArmy>();
            }
        }
    } else {
        TryBuildRefinery(actions, observation);
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
