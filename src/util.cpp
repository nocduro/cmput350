#include <sc2api/sc2_api.h>
#include <iostream>
#include "bot.h"
#include <vector>
using namespace sc2;


// finds the nearest gas patch in relation to unit
const Unit* FindNearestObject(observation_t Observation, const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
			float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	return target;
}

bool TryBuildStructure(action_t Actions, observation_t Observation, ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV) {
	// If a unit already is building a supply structure of this type, do nothing.
	// Also get an scv to build the structure.
	const Unit* unit_to_build = nullptr;
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ability_type_for_structure) {
				return false;
			}
		}

		if (unit->unit_type == unit_type) {
			unit_to_build = unit;
		}
	}

	float rx = GetRandomScalar();
	float ry = GetRandomScalar();

	if (ability_type_for_structure == ABILITY_ID::BUILD_REFINERY) {
		Actions()->UnitCommand(unit_to_build, ability_type_for_structure, FindNearestObject(Observation, unit_to_build->pos));
		std::cout << "build refinery" << std::endl;
	}
	else {
		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,
			Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
	}

	return true;
}

// attempts to build supply depot
bool TryBuildSupplyDepot(action_t Actions, observation_t Observation) {
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    
	if (Observation()->GetFoodUsed() == 14) {
		// std::cout << "BUILD SUPPLY DEPOT" << std::endl;
		return TryBuildStructure(Actions, Observation, ABILITY_ID::BUILD_SUPPLYDEPOT);
	}

	// If we are not supply capped, don't build a supply depot.
	if (Observation()->GetFoodUsed() <= Observation()->GetFoodCap() - 2)
		return false;
    
	// Try and build a depot. Find a random SCV and give it the order.
	return TryBuildStructure(Actions, Observation, ABILITY_ID::BUILD_SUPPLYDEPOT);
}

size_t CountUnitType(observation_t Observation, UNIT_TYPEID unit_type) {
	return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

bool TryBuildBarracks(action_t Actions, observation_t Observation) {
	// can't build barracks without at least one supply depot
	if (CountUnitType(Observation, UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
		std::cout << "trying to build supply depot before barracks" << std::endl;
		TryBuildSupplyDepot(Actions, Observation);
		return false;
	}

	// build first barracks only when supply is at 16
	if (Observation()->GetFoodUsed() < 16) {
		return false;
	}

	return TryBuildStructure(Actions, Observation, ABILITY_ID::BUILD_BARRACKS); // conditions were passed and we delegate to TryBuildStructure()
}

// attempts to build a refinery on gas patch
bool TryBuildRefinery(action_t Actions, observation_t Observation) {
	return TryBuildStructure(Actions, Observation, ABILITY_ID::BUILD_REFINERY);
}

bool FarmGas(action_t Actions, observation_t Observation){
    const ObservationInterface* observation = Observation();
    Units Refinerys = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_REFINERY));
    Units Workers = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SCV));
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    for (const auto& refinery:Refinerys){
        //for (const auto& worker: Workers){
        size_t workers_needed = refinery->ideal_harvesters - refinery->assigned_harvesters;
        for (size_t i =0 ; i < workers_needed - 1; i++){
            if (refinery->assigned_harvesters < refinery->ideal_harvesters){
                //Actions()->UnitCommand(worker, ABILITY_ID::HARVEST_GATHER, refinery);
                Actions()->UnitCommand(Workers[i], ABILITY_ID::HARVEST_GATHER, refinery);
                std::cout<< "here test" << std::endl;
                
                
            }
            else if (refinery->assigned_harvesters == refinery->ideal_harvesters){
                return true;
            }
        }
    }
     
    
    return false;
}

bool GetRandomUnit(const Unit*& unit_out, observation_t Observation, UnitTypeID unit_type) {
	Units units = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
	if (!units.empty()) {
		unit_out = GetRandomEntry(units);
		return true;
	}
	return false;
}

bool GetRandomMineralSCV(const Unit*& unit_out, observation_t Observation) {
	Units units = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SCV));

	for (const Unit* unit : units) {
		auto tag = unit->orders.front().target_unit_tag;
		if (tag == NullTag) {
			continue;
		}
		auto target = Observation()->GetUnit(tag);
		if (target->mineral_contents > 0) {
			unit_out = unit;
			return true;
		}
	}
	return false;
}