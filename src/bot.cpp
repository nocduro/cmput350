#include <sc2api/sc2_api.h>
#include <string>
#include <iostream>

using namespace sc2;

class Bot : public Agent {
public:
	virtual void OnGameStart() final {
		std::cout << "Can I get a pogchamp in the chat?" << std::endl;
		assignScout();
	}

	virtual void OnStep() final {
		updateSupplies();
		if (enemypos < 0) {
			scout();
		}
		switch (stage) {
		case 0:
			if (Observation()->GetMinerals() >= 100) {
				TryBuildSupplyDepot();
			}
			if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) == 1) {
				std::cout << "first depot" << std::endl;
				++stage;
			}
			break;
		case 1:
			TryBuildBarracks();
		
			if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 1) {
				std::cout << "first RAX" << std::endl;
				++stage;
			}
			break;
		case 2:
			UpgradeCC();
			if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) == 0 && makeSCV) {
				trainMarine(1);
				makeSCV = false;
			}
			
			std::cout << "orbital + marine" << std::endl;
			++stage;
			
			break;

		case 3:
			
			if (Observation()->GetMinerals() >= 450) {
				if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) < 4) {
					TryBuildBarracks();
				}
			}
			makeSCV = true;
			break;
		}
		//supplies now has our current supplies



	
	}

	virtual void OnUnitIdle(const Unit* unit) final {
		switch (unit->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			if (makeSCV) {
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
			}
			break;
		}
		case UNIT_TYPEID::TERRAN_SCV: {
			const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
			if (!mineral_target) {
				break;
			}
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
			break;
		}
		case UNIT_TYPEID::TERRAN_BARRACKS: {
			//idle until told
			break;
		}
		case UNIT_TYPEID::TERRAN_MARINE: {
			const GameInfo& game_info = Observation()->GetGameInfo();
			break;
		}
		default: {
			break;
		}
		}
	}
private:
	void trainMarine(int count) {
		Units rax = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_BARRACKS));
		for (size_t i = 0; i < count; ++i) {
			Actions()->UnitCommand(rax.front(), ABILITY_ID::TRAIN_MARINE);
		}
	}
	void UpgradeCC() {
		//upgrade our base to an orbital command
		Units cc= Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_COMMANDCENTER));
		if (cc.size() != 0) {
			Actions()->UnitCommand(cc.front(), ABILITY_ID::MORPH_ORBITALCOMMAND);
		}
	}
	void updateSupplies() {
		supplies = Observation()->GetFoodUsed();
	}
	void assignScout() {
		Units SCVs = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SCV));
		if (scouter == NULL) {
			scouter = SCVs.front();
		}
	}

	void scout() {
		const GameInfo& game_info = Observation()->GetGameInfo();
		if (enemypos < 0) { //check if we already found enemy base
			// start scouting
			if (scouter->is_alive && scouting < 3) {
				Actions()->UnitCommand(scouter, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[scouting]); //move marine to enemy base location
				if (Point2D(scouter->pos) == game_info.enemy_start_locations[scouting]) {
					++scouting; //move to next target next time
				}
			}
			else if (!scouter->is_alive) {
				enemypos = scouting;
			}
		}
	}

	size_t CountUnitType(UNIT_TYPEID unit_type) {
		return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
	}

	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV) {
		const ObservationInterface* observation = Observation();

		// If a unit already is building a supply structure of this type, do nothing.
		// Also get an scv to build the structure.
		const Unit* unit_to_build = nullptr;
		Units units = observation->GetUnits(Unit::Alliance::Self);
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

		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,
			Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));

		return true;
	}

	bool TryBuildSupplyDepot() {
		// Try and build a depot. Find a random SCV and give it the order.
		return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
	}

	const Unit* FindNearestMineralPatch(const Point2D& start) {
		Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
		float distance = std::numeric_limits<float>::max();
		const Unit* target = nullptr;
		for (const auto& u : units) {
			if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
				float d = DistanceSquared2D(u->pos, start);
				if (d < distance) {
					distance = d;
					target = u;
				}
			}
		}
		return target;
	}

	bool TryBuildBarracks() {
		const ObservationInterface* observation = Observation();

		if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
			return false;
		}
		
		return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);
	}

	const Unit* scouter = NULL;
	int enemypos = -1;
	int scouting = 0;
	int supplies;
	bool makeSCV = true;
	int stage = 0;

};

int main(int argc, char* argv[]) {
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);
	coordinator.SetStepSize(1);

	Bot bot;
	coordinator.SetParticipants({
		CreateParticipant(Race::Terran, &bot),
		CreateComputer(Race::Zerg)
	});

	coordinator.LaunchStarcraft();
	coordinator.StartGame("CactusValleyLE.SC2Map");

	while (coordinator.Update()) {
	}

	return 0;
}