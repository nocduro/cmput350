#include <sc2api/sc2_api.h>
#include <string>
#include <iostream>

using namespace sc2;

class Bot : public Agent {
public:
	virtual void OnGameStart() final {
		assignScout();
		assignBase();
	}

	virtual void OnStep() final {

		updateSupplies();
		// std::cout << supplies << std::endl;
		if (enemypos < 0) {
			scout();
		}

		// keep building supply depots after supply depot 2
		if (buildDepot == true) {
			TryBuildSupplyDepot();
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
				++rax;
				++stage;
			}
			break;
		case 2:

			if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) == 0 || makeSCV) {
				makeMarine = true;


				makeSCV = false;
			}

			
			if (CountUnitType(UNIT_TYPEID::TERRAN_ORBITALCOMMAND)==0) {
				UpgradeCC();
				if (base->orders.size() != 0) {
					// move stage if orbital command has begun
					if (base->orders.front().ability_id == 1516) {
						std::cout << "orbital command" << std::endl;
						++stage;
					}
					
				}
			}
			break;

		case 3:
			if (Observation()->GetMinerals() >= 450 || raxstarted) {
				raxstarted = true;
				if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) < 4) {
					rax++;
					
					BuildBarracksAfter(NULL, 2);
					BuildBarracksAfter(NULL, 3);
					BuildBarracksAfter(NULL, 4);
				}
			}
			makeSCV = true;

			makeMarine = false;
			if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 4) {
				std::cout << "four RAX exist, move to stage 4" << std::endl;
				raxstarted = false;

				++stage;
			}
			break;
		case 4:
			if (Observation()->GetMinerals() >= 100) {
				if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) == 1) {
					std::cout << "second Supply Depot" << std::endl;
					BuildSupplyDepotTwo(NULL);
				}
			}
			if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) == 2) {
				// Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
				std::cout << "second depot was built, move to stage 5" << std::endl;
				buildDepot = true;


				++stage;
			}
			break;
		case 5:
			if (Observation()->GetMinerals() >= 300 || raxstarted) {
				raxstarted = true;
				if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 4) { // less than 4 is kinda sketch, should always be 1
					rax++;
					
					BuildBarracksAfter(NULL, 5);
					BuildBarracksAfter(NULL, 6);
				}
			}

			if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 6) {
				std::cout << "6 RAX exist, move to stage 6" << std::endl;
				++stage;
			}
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
			if (makeMarine) {
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
			}
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
	void assignBase() {
		base = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_COMMANDCENTER)).front();
	}
	void trainMarine() {
		Units units = Observation()->GetUnits(Unit::Alliance::Self);
		Units rax = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_BARRACKS));
		/*
		for (const auto& unit : units) {
			if (unit->unit_type == UNIT_TYPEID::TERRAN_BARRACKS && unit->orders.size() == 0) {
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
				break;
			}
		}
		*/
		while (rax[0]->orders.front().ability_id == ABILITY_ID::TRAIN_MARINE) {
			Actions()->UnitCommand(rax[0], ABILITY_ID::TRAIN_MARINE);
		}
		std::cout << "marine" << std::endl;
		
		
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

			if (unit->unit_type == unit_type && unit!=scouter) {
				unit_to_build = unit;
			}
		}

		float rx = GetRandomScalar();
		float ry = GetRandomScalar();

		if (ability_type_for_structure == ABILITY_ID::BUILD_SUPPLYDEPOT && CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) == 0) {
			BuildSupplyDepotOne(unit_to_build);
		} else if (ability_type_for_structure == ABILITY_ID::BUILD_BARRACKS && CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 0) {
			BuildBarracksOne(unit_to_build);
		} else if (ability_type_for_structure == ABILITY_ID::BUILD_SUPPLYDEPOT && CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) == 1) {
			BuildSupplyDepotTwo(unit_to_build);
        } else if (ability_type_for_structure == ABILITY_ID::BUILD_BARRACKS && CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 1) {
            BuildBarracksAfter(unit_to_build, 2);
        }else if (ability_type_for_structure == ABILITY_ID::BUILD_BARRACKS && CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 2) {
           BuildBarracksAfter(unit_to_build, 3);
        }else if (ability_type_for_structure == ABILITY_ID::BUILD_BARRACKS && CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 3) {
            BuildBarracksAfter(unit_to_build, 4);
        }else if (ability_type_for_structure == ABILITY_ID::BUILD_BARRACKS && CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 4) {
            BuildBarracksAfter(unit_to_build, 5);
        }else if (ability_type_for_structure == ABILITY_ID::BUILD_BARRACKS && CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) == 5) {
            BuildBarracksAfter(unit_to_build, 6);
        }else {
			Actions()->UnitCommand(unit_to_build,
				ability_type_for_structure,
				Point2D(base->pos.x + rx * 15.0f, base->pos.y + ry * 15.0f));
		}

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
				float d = DistanceSquared2D(u->pos, base->pos);
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

	void BuildSupplyDepotOne(const Unit* unit_to_build) {
		const ObservationInterface* observation = Observation();
		const Point2D startLocation = observation->GetStartLocation();
		Units units = observation->GetUnits(Unit::Alliance::Self);
		float rx = GetRandomScalar();
		float ry = GetRandomScalar();

		// First supply depot being built
		// we want to build it at choke point

		// if base is at top left
		if (startLocation.x < 100 && startLocation.y > 100) {
			std::cout << "position top left" << std::endl;

			buildPoint = Point2D(51,160);

		// if base top right
		} else if (startLocation.x > 100 && startLocation.y > 100) {
			std::cout << "position top right" << std::endl;

			buildPoint = Point2D(160,141);

		// if base bottom right
		} else if (startLocation.x > 100 && startLocation.y < 100) {
			std::cout << "position bottom right" << std::endl;

			buildPoint = Point2D(138,29);			

		// if base bottom left
		} else if (startLocation.x < 100 && startLocation.y < 100) {
			std::cout << "position bottom left" << std::endl;

			buildPoint = Point2D(29,54);

		// error control (SHOULD NEVER HAPPEN)		
		} else {
			std::cout << "LOCATION ERROR" << std::endl;
			std::cout << startLocation.x << ", " << startLocation.y << std::endl;
			buildPoint = Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f);
		}

		// buildPoint = Point2D(32,51);

		// build supply depot at choke point
		Actions()->UnitCommand(unit_to_build,
			ABILITY_ID::BUILD_SUPPLYDEPOT,
			buildPoint);


	}

	void BuildBarracksOne(const Unit* unit_to_build) {
		const ObservationInterface* observation = Observation();
		const Point2D startLocation = observation->GetStartLocation();
		Units units = observation->GetUnits(Unit::Alliance::Self,IsUnit(UNIT_TYPEID::TERRAN_SUPPLYDEPOT));
		float rx = GetRandomScalar();
		float ry = GetRandomScalar();

		// std::cout << units.size() <<std::endl;

		// we know only depot exists when first barracks is getting built
		// so we store the supply depot into variable for later reference
		supplyDepotOne = units[0]; 


		// First barracks being built
		// we want to build it at choke point

		// if base is at top left
		if (startLocation.x < 100 && startLocation.y > 100) {
			buildPoint = Point2D(51,162);

		// if base top right
		} else if (startLocation.x > 100 && startLocation.y > 100) {
			buildPoint = Point2D(162,140);

		// if base bottom right
		} else if (startLocation.x > 100 && startLocation.y < 100) {
			buildPoint = Point2D(140,29);			

		// if base bottom left
		} else if (startLocation.x < 100 && startLocation.y < 100) {
			buildPoint = Point2D(29,51);

		// error control (SHOULD NEVER HAPPEN)		
		} else {
			std::cout << "LOCATION ERROR" << std::endl;
			std::cout << startLocation.x << ", " << startLocation.y << std::endl;
			buildPoint = Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f);
		}

		// buildPoint = Point2D(29,51);

		// Build first barracks at choke point
		Actions()->UnitCommand(unit_to_build,
			ABILITY_ID::BUILD_BARRACKS,
			buildPoint);
	}

	void BuildSupplyDepotTwo(const Unit* unit_to_build) {
		const ObservationInterface* observation = Observation();
		const Point2D startLocation = observation->GetStartLocation();
		float rx = GetRandomScalar();
		float ry = GetRandomScalar();


		Units units = Observation()->GetUnits(Unit::Alliance::Self);
		if (unit_to_build == NULL) {
			for (const auto& unit : units) {
				if (unit->unit_type == UNIT_TYPEID::TERRAN_SCV && unit != scouter && (unit->orders.size() == 0 || unit->orders.front().ability_id == ABILITY_ID::HARVEST_GATHER)) {
					unit_to_build = unit;
					// std::cout << "UNIT FOUND" << std::endl;
				}
			}
		}

		// second supply depot being built
		// we want to build it at choke point

		// if base is at top left
		if (startLocation.x < 100 && startLocation.y > 100) {
			buildPoint = Point2D(54,163);

		// if base top right
		} else if (startLocation.x > 100 && startLocation.y > 100) {
			buildPoint = Point2D(163,138);

		// if base bottom right
		} else if (startLocation.x > 100 && startLocation.y < 100) {
			buildPoint = Point2D(141,32);				

		// if base bottom left
		} else if (startLocation.x < 100 && startLocation.y < 100) {
			buildPoint = Point2D(32,51);

		// error control (SHOULD NEVER HAPPEN)		
		} else {
			std::cout << "LOCATION ERROR" << std::endl;
			std::cout << startLocation.x << ", " << startLocation.y << std::endl;
			buildPoint = Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f);
		}

		// buildPoint = Point2D(29,54);

		Actions()->UnitCommand(unit_to_build,
				ABILITY_ID::BUILD_SUPPLYDEPOT,
				buildPoint);

		Actions()->UnitCommand(supplyDepotOne, ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
		std::cout << "lower depot 1" << std::endl;
	}

    void BuildBarracksAfter(const Unit* unit_to_build,size_t Barrack ) {
        const ObservationInterface* observation = Observation();
        const Point2D startLocation = observation->GetStartLocation();
        float rx = GetRandomScalar();
        float ry = GetRandomScalar();
		Units units = Observation()->GetUnits(Unit::Alliance::Self);
		if (unit_to_build == NULL) {
			for (const auto& unit : units) {
				if (unit->unit_type == UNIT_TYPEID::TERRAN_SCV && unit != scouter && (unit->orders.size()==0 ||unit->orders.front().ability_id != ABILITY_ID::BUILD_BARRACKS)) {
					unit_to_build = unit;
				}
			}
		}
            
        // if base is at top left
        if (startLocation.x < 100 && startLocation.y > 100) {
            if (Barrack == 2 ){
                buildPoint = Point2D(34.5,151.5);
            }
            else if (Barrack == 3){
                buildPoint = Point2D(37.5,152.5);
            }
            else if (Barrack == 4){
                buildPoint = Point2D(40.5,153.5);
            }
            else if (Barrack == 5){
                buildPoint = Point2D(43.5,154.5);
            }
            else if (Barrack == 6){
                buildPoint = Point2D(47.5,155.5);
            }
            Actions()->UnitCommand(unit_to_build,
                                    ABILITY_ID::BUILD_BARRACKS,
                                    buildPoint);
                
        // if base top right
        } else if (startLocation.x > 100 && startLocation.y > 100) {
            if (Barrack == 2 ){
                buildPoint = Point2D(150.5,156.5);
            }
            else if (Barrack == 3){
                buildPoint = Point2D(152.5,153.5);
            }
            else if (Barrack == 4){
                buildPoint = Point2D(154.5,150.5);
            }
            else if (Barrack == 5){
                buildPoint = Point2D(156.5,147.5);
            }
            else if (Barrack == 6){
                buildPoint = Point2D(158.5,144.5);
            }
            Actions()->UnitCommand(unit_to_build,
                                    ABILITY_ID::BUILD_BARRACKS,
                                    buildPoint);
                
        // if base bottom right
        } else if (startLocation.x > 100 && startLocation.y < 100) {
            if (Barrack == 2 ){
                buildPoint = Point2D(157.5,41.5);
            }
            else if (Barrack == 3){
                buildPoint = Point2D(154.5,39.5);
            }
            else if (Barrack == 4){
                buildPoint = Point2D(151.5,37.5);
            }
            else if (Barrack == 5){
                buildPoint = Point2D(148.5,35.5);
            }
            else if (Barrack == 6){
                buildPoint = Point2D(145.5,33.5);
            }
            Actions()->UnitCommand(unit_to_build,
                                    ABILITY_ID::BUILD_BARRACKS,
                                    buildPoint);
                
        // if base bottom left
        } else if (startLocation.x < 100 && startLocation.y < 100) {
            if (Barrack == 2 ){
                buildPoint = Point2D(42.5,34.5);
            }
            else if (Barrack == 3){
                buildPoint = Point2D(40.5,37.5);
            }
            else if (Barrack == 4){
                buildPoint = Point2D(38.5,40.5);
            }
            else if (Barrack == 5){
                buildPoint = Point2D(36.5,43.5);
            }
            else if (Barrack == 6){
                buildPoint = Point2D(34.5,46.5);
            }
            Actions()->UnitCommand(unit_to_build,
                                    ABILITY_ID::BUILD_BARRACKS,
                                    buildPoint);
                
                // error control (SHOULD NEVER HAPPEN)
        } else {
            std::cout << "LOCATION ERROR" << std::endl;
            std::cout << startLocation.x << ", " << startLocation.y << std::endl;
            Actions()->UnitCommand(unit_to_build,
                                    ABILITY_ID::BUILD_BARRACKS,
                                    Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
        }
    }

	// virtual void OnBuildingConstructionComplete(const Unit* unit) {
	// 	if (unit->unit_type.ToType() == UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
	// 		Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
	// 		// Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SUPPLYDEPOT_RAISE);
	// 	}
	// }

	// ! Called when an enemy unit enters vision from out of fog of war.
    // !< \param unit The unit entering vision.
    virtual void OnUnitEnterVision(const Unit*) {
		
		if (enemypos > 0) {
			std::cout << "ENEMY" << std::endl;
			Actions()->UnitCommand(supplyDepotOne, ABILITY_ID::MORPH_SUPPLYDEPOT_RAISE);
		}
	}

	const Unit* scouter = NULL;
	const Unit* base = NULL;
	int enemypos = -1;
	int scouting = 0;
	bool makeMarine=false;
	bool raxstarted;
	int supplies;
	int rax = 0;
	bool makeSCV = true;
	int stage = 0;
  	Point2D buildPoint;
	bool buildDepot = false;
	const Unit* supplyDepotOne;
	// bool scouting_done = false;
	// const Unit* supplyDepotTwo;

};

int main(int argc, char* argv[]) {
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);
	coordinator.SetStepSize(10);

	Bot bot;
	coordinator.SetParticipants({
		CreateParticipant(Race::Terran, &bot),
		CreateComputer(Race::Protoss)
	});

	coordinator.SetWindowSize(2000,1500);

	coordinator.LaunchStarcraft();
	coordinator.StartGame("CactusValleyLE.SC2Map");

	while (coordinator.Update()) {
	}

	return 0;
}
