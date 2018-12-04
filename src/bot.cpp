#include <sc2api/sc2_api.h>
#include "sc2api/sc2_map_info.h"
#include <iostream>
#include <math.h>
#include <algorithm>
#include <ctime>


using namespace sc2;

struct IsVespeneGeyser {
	bool operator()(const Unit& unit) {
		switch (unit.unit_type.ToType()) {
			case UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
			case UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER: return true;
			case UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER: return true;
			default: return false;
		}
	}
};

class Bot : public Agent {
public:
	virtual void OnGameStart() final {
		std::cout << "Can I get a pogchamp in the chat?" << std::endl;
		const GameInfo& game_info = Observation()->GetGameInfo();
		playerpos = getPlayerPos(game_info.enemy_start_locations);
		std::cout << "We are starting at: (" << playerpos.x << ", " << playerpos.y << ")"<< std::endl;
		assignScout();
	}

	virtual void OnStep() final {
		if (enemypos < 0) {
			scout();
		}
		TryBuildSupplyDepot();
		TryBuildBarracks();
        TryFarmGas();
        TryBuildFactory();
        TryBuildStarport();
        earlyrush();
        
        

	}

	virtual void OnUnitIdle(const Unit* unit) final {
        const ObservationInterface* observation = Observation();
		switch (unit->unit_type.ToType()) {

            // if command center is idle, makes scvs
    		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
                size_t scvCount = CountUnitType(UNIT_TYPEID::TERRAN_SCV);

				// when supply hits 19, we want to stop scv production and call orbital
				if (observation->GetFoodUsed() >= 19) {
					Actions()->UnitCommand(unit, ABILITY_ID::MORPH_ORBITALCOMMAND);
					std::cout << "Orbital command started" << std::endl;
				}

                if (scvCount <= 30){
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
                }

    			break;
    		}
			
            // if scv is idle, make it do something productive
    		case UNIT_TYPEID::TERRAN_SCV: {
                size_t countSCV = CountUnitType(UNIT_TYPEID::TERRAN_SCV);
    			const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
    			const Unit* gas_target = FindNearestObject(unit->pos,UNIT_TYPEID::NEUTRAL_VESPENEGEYSER);
                const Unit* refinery_target = FindNearestObject(unit->pos,UNIT_TYPEID::TERRAN_REFINERY);
                Units Refinerys = Observation()->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::TERRAN_REFINERY));

				// if scv is idle and supply is equal to 19, build a second command center
				// if (observation->GetFoodUsed() >= 19) {
				// 	Actions()->UnitCommand(unit, ABILITY_ID::BUILD_COMMANDCENTER, mineral_target);
				// }

    			if (!mineral_target) {
                    // TryBuildRefinery();
    				// Actions()->UnitCommand(unit, ABILITY_ID::SMART, gas_target); // send idle worker to gas
                    break;
    			}
                if (observation->GetFoodUsed() == 15) {
                    Actions()->UnitCommand(unit, ABILITY_ID::SMART, gas_target);
                    
                    break;// send idle worker to gas
                }
    			if (gas_target){
                    TryBuildRefinery();
				}
                
                /*if (Refinerys.size() > 0){
                    for ( const auto& refinery: Refinerys){
                        if (refinery->assigned_harvesters < refinery->ideal_harvesters){
                            Actions()->UnitCommand(unit,ABILITY_ID::HARVEST_GATHER,refinery);
                            std::cout << "here" << std::endl;
                        }
                    }
                }*/
                
    			else {
                    if (countSCV < 14){
                        Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);// send idle worker to mineral patch
                    }
                    else{
                        Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER, refinery_target);
                    }
    				break;
    			}
    		}
    		case UNIT_TYPEID::TERRAN_BARRACKS: {
                // size_t counttech_lab = CountUnitType(UNIT_TYPEID::TERRAN_TECHLAB);
                // if (counttech_lab < 1 && !haveTechLab){
                //     std::cout << "Build TechLab" << std::endl;
                //     Actions()->UnitCommand(unit, ABILITY_ID::BUILD_TECHLAB);
                //     // std::cout << "here2" << std::endl;
                //     if (counttech_lab >=1){
                //         haveTechLab = true;
                //     }
                // } else {
                //     //BuildOrder(unit);
				// }
                size_t countReactor = CountUnitType(UNIT_TYPEID::TERRAN_BARRACKSREACTOR);
                if (countReactor < 1 && !haveReactor){
                  
                    Actions()->UnitCommand(unit, ABILITY_ID::BUILD_REACTOR);
                    
                    if (countReactor >=1){
                        haveReactor = true;
                    }
                }else{
                    //Train 2 marines at once
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
                }
    			
    			break;
    		}
    		case UNIT_TYPEID::TERRAN_MARINE: {
				Actions()->UnitCommand(unit, ABILITY_ID::HOLDPOSITION, unit->pos);
				const GameInfo& game_info = Observation()->GetGameInfo();
				size_t marinecount = CountUnitType(UNIT_TYPEID::TERRAN_MARINE); // get number of marines
			  			
				
    			//small early game rush attempt
				if (marinecount > 12 && !earlyAttacked && enemypos >= 0) { //check if we have enough marines
					earlyAttacked = 1; //dont do it again
					//earlyrush(marinecount); //ATTACC
				}
    			break;
    		}
            case UNIT_TYPEID::TERRAN_STARPORT: {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MEDIVAC);
                break;
            }
            case UNIT_TYPEID::TERRAN_FACTORY:{
                size_t counttech_lab = CountUnitType(UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
                //Units tech_lab = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_TECHLAB));
                if (counttech_lab < 1 && !haveTechLab){
                    //std::cout << "here" << std::endl;
                    Actions()->UnitCommand(unit, ABILITY_ID::BUILD_TECHLAB);
                    //std::cout << "here2" << std::endl;
                    if (counttech_lab >=1){
                        haveTechLab = true;
                        //std::cout << "here3" << std::endl;
                        
                    }
                }else{
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SIEGETANK);
                    //std::cout << "here4" << std::endl;
                }
            }
    		default: {
    			break;
    		}

		}
	}

	virtual void OnUnitCreated(const Unit* unit) final {

	}

	virtual void OnUpgradeCompleted(UpgradeID) {

	}

	virtual void OnBuildingConstructionComplete(const Unit*) {

	}

	// Called when an enemy unit enters view
    // param unit The unit entering vision.
    virtual void OnUnitEnterVision(const Unit*) {

	}






private:
	void assignScout() {
		Units SCVs = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SCV));
		if (scouter == NULL) {
			scouter = SCVs.front();
			std::cout << "scout assigned" << std::endl;
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
				if (mineralpatches.size() <2) {
					const Unit* target = FindNearestMineralPatch(scouter->pos);
					if (target != nullptr) {
						if (std::find(mineralpatches.begin(), mineralpatches.end(), target) != mineralpatches.end()) {
							/* v contains x */
						}
						else {
							mineralpatches.push_back(target);
							std::cout << target->pos.x << " " << target->pos.y << std::endl;
						}
					}
				}
			}
			else if (!scouter->is_alive) {
				enemypos = scouting;
				std::cout << "enemy at pos " << enemypos << std::endl;
			}
		}
	}
	//attacks early game with marinecount marines to nearest enemy
	void earlyrush() {
		const GameInfo& game_info = Observation()->GetGameInfo();
		sc2::Units marines = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE)); //get our marine units
        sc2::Units tanks = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SIEGETANK));
        sc2::Units medivac = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MEDIVAC));
        if (tanks.size() >= 3 && enemypos >= 0){
            std::cout << "early rush" << std::endl;
            for (size_t i = 0; i < marines.size(); i++){ //iterate through our marines
                Actions()->UnitCommand(marines[i], ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[enemypos]); //attack selected target
            }
            for (size_t j  = 0; j < tanks.size(); j++){
                Actions()->UnitCommand(tanks[j], ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[enemypos]);
            }
            for (size_t k  = 0; k < medivac.size(); k++){
                Actions()->UnitCommand(medivac[k], ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[enemypos]);
            }
        }
	}
	//find nearest enemy will return index of the nearest enemy base using x,y
	int FindNearestEnemy(const GameInfo& game_info, sc2::Units barracks) {
		int min = 0;
		int startx = barracks[0]->pos.x; //get the position of our barracks
		int starty = barracks[0]->pos.y;
		float smallest = Distance(startx, starty, game_info.enemy_start_locations[0].x, game_info.enemy_start_locations[0].y);
		
		for (size_t i = 1; i < game_info.enemy_start_locations.size(); i++) //iterate through enemy bases
		{
			if (Distance(startx,starty, game_info.enemy_start_locations[i].x, game_info.enemy_start_locations[i].y) < smallest) //if next enemy is closer
			{
				min = i; //set min to new min
			}
		}
		return min; //return index
	}
	//helper function to get distance between two points
	float Distance(int x1, int y1, int x2, int y2) {
		return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
	}

	// returns the number of a given unit type
	size_t CountUnitType(UNIT_TYPEID unit_type) {
		return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
	}

	// attempts to build a structure of given type
	// default unit to build is scv
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV) {
		const ObservationInterface* observation = Observation();

		// If any unit already is building a supply structure of this type, do nothing.
		// Also get an scv to build the structure.
		const Unit* unit_to_build = nullptr;
		Units units = observation->GetUnits(Unit::Alliance::Self); // all units
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

		// if the structure type we want to build is a refinery, find nearest geyser and build
        if (ability_type_for_structure == ABILITY_ID::BUILD_REFINERY) {
            Actions()->UnitCommand(unit_to_build, ability_type_for_structure, FindNearestObject(unit_to_build->pos,UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
        } else {
		  	Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,
			Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
        }

		return true;
	}

    // attempts to build supply depot
	bool TryBuildSupplyDepot() {
		const ObservationInterface* observation = Observation();

		// first supply depot built at 14 supply
        if (observation->GetFoodUsed() == 14) {
            std::cout << "BUILT SUPPLY DEPOT" << std::endl;
			std::cout << observation->GetFoodUsed() << std::endl;
            return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
        }

		// If we are not supply capped, don't build a supply depot.
		if (observation->GetFoodUsed() <= observation->GetFoodCap() - 15)
			return false;

        // std::cout << observation->GetFoodUsed() << " " << observation->GetFoodCap() << std::endl;

		// Try and build a depot. Find a random SCV and give it the order.
		return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
		// return false;
	}

    // attempts to build a refinery on gas patch
    bool TryBuildRefinery() {
        const ObservationInterface* observation = Observation();

        if (observation->GetFoodUsed() == 14 || observation->GetFoodUsed() == 16) {
            std::cout << "BUILT REFINERY" << std::endl;
			std::cout << observation->GetFoodUsed() << std::endl;
            return TryBuildStructure(ABILITY_ID::BUILD_REFINERY);
        }

        return TryBuildStructure(ABILITY_ID::BUILD_REFINERY);
		// return false;
    }
	
    // finds the nearest mineral patch to our current base
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

    // attempts to build a barracks depending on a number of conditions
	bool TryBuildBarracks() {
		const ObservationInterface* observation = Observation();

        // can't build barracks without at least one supply depot
		if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
			return false;
		}

        // build first barracks when supply is at 16
        if (observation->GetFoodUsed() == 16) {
            // std::cout << "BUILD BARRACKS" << std::endl;
			// std::cout << observation->GetFoodUsed() << std::endl;
			// TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);
        }

         //if we have more than 1 barracks, don't build anymore
		 if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) >= 1) {
		 	return false;
		 }

		return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS); // conditions were passed and we delegate to TryBuildStructure()
		// return false;
	}
        
    bool TryBuildStarport() {
        const ObservationInterface* observation = Observation();
            
            // can't build barracks without at least one supply depot
        if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
            return false;
        }
            
            // build first barracks only when supply is at 16
        if (observation->GetFoodUsed() == 16) {
            // std::cout << "BUILD BARRACKS" << std::endl;
        }
            
            // if we have more than 1 barracks, don't build anymore
        if (CountUnitType(UNIT_TYPEID::TERRAN_STARPORT) >= 1) {
            return false;
        }
        
        return TryBuildStructure(ABILITY_ID::BUILD_STARPORT); // conditions were passed and we delegate to TryBuildStructure()
    }
        
    bool TryBuildFactory() {
        const ObservationInterface* observation = Observation();
            
            // can't build barracks without at least one supply depot
        if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
            return false;
        }
            
            // build first barracks only when supply is at 16
        if (observation->GetFoodUsed() == 16) {
                // std::cout << "BUILD BARRACKS" << std::endl;
        }
            
            // if we have more than 1 barracks, don't build anymore
        if (CountUnitType(UNIT_TYPEID::TERRAN_FACTORY) >= 1) {
            return false;
        }
            
        return TryBuildStructure(ABILITY_ID::BUILD_FACTORY); // conditions were passed and we delegate to TryBuildStructure()
    }

	bool TryBuildEngBay() {
		const ObservationInterface* observation = Observation();
            
        if (observation->GetFoodUsed() == 16) {
        }
            
            // if we already have 1  or more engineering bay, don't build anymore
        if (CountUnitType(UNIT_TYPEID::TERRAN_ENGINEERINGBAY) >= 1) {
            return false;
        }
            
        return TryBuildStructure(ABILITY_ID::BUILD_ENGINEERINGBAY); // conditions were passed and we delegate to TryBuildStructure()

	}
        
        

	sc2::Point2D getPlayerPos(std::vector<sc2::Point2D> enemylocations) {
		//Starting positions within the map
		std::vector<sc2::Point2D> startlocations; 
		startlocations.push_back(Point2D(33.5, 33.5));
		startlocations.push_back(Point2D(33.5, 158.5));
		startlocations.push_back(Point2D(158.5, 33.5));
		startlocations.push_back(Point2D(158.5, 158.5));
		//(33.5,33.5)	(33.5, 158.5)	(158.5, 33.5)	(158.5, 158.5)
		for (Point2D pos : startlocations) {
			if (std::find(enemylocations.begin(), enemylocations.end(), pos) != enemylocations.end()) {
				// not our starting position
			}
			else {
				//our starting position
				return pos;
			}
		}
		return Point2D(); //somehow we dont exist and have returned a null point2d
	}



        
    const Unit* FindNearestObject(const Point2D& start,UNIT_TYPEID type) {
        Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
        float distance = std::numeric_limits<float>::max();
        const Unit* target = nullptr;
        for (const auto& u : units) {
            if (u->unit_type == type) {
                float d = DistanceSquared2D(u->pos, start);
                if (d < distance) {
                    distance = d;
                    target = u;
                }
            }
        }
        return target;
    }
        
    void BuildOrder(const Unit* unit){
        const ObservationInterface* observation = Observation();
        size_t mineral=observation->GetMinerals();
        size_t vespene=observation->GetVespene();
        size_t countMarine = CountUnitType(UNIT_TYPEID::TERRAN_MARINE);
        size_t countreaper = CountUnitType(UNIT_TYPEID::TERRAN_REAPER);
        size_t counttech_lab = CountUnitType(UNIT_TYPEID::TERRAN_TECHLAB);
        
        /*if (countMarine > 10 && countreaper < 5){
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_REAPER);
        }
        else if (counttech_lab >= 1){
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARAUDER);
        }
        if (mineral > 50 && vespene >25){
            Actions()->UnitCommand(unit, ABILITY_ID::BUILD_TECHLAB);
        }*/
        
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
        
            
    }
        
    void TryFarmGas(){
        Units Refinerys = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_REFINERY));
        
        for ( const auto& refinery: Refinerys){
            Units Workers = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_SCV));
            
            if (refinery->assigned_harvesters < refinery->ideal_harvesters){
                Actions()->UnitCommand(Workers.front(),ABILITY_ID::HARVEST_GATHER,refinery);
                //std::cout << "here" << std::endl;
            }
            
        }
        
    }
        
 

	int scouting = 0; //used for scouting all enemy bases
	bool earlyAttacked = 0; //used as a flag to see if we have early rushed or not
	const Unit *scouter = NULL; //marine unit used for scouting earlygame
	int enemypos = -1; //index for enemy position
	sc2::Point2D playerpos;
	bool haveTechLab = false;
	bool haveReactor = false;
	std::vector<const Unit*> mineralpatches;
};

int main(int argc, char* argv[]) {
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);

	Bot bot;
	coordinator.SetParticipants({
		CreateParticipant(Race::Terran, &bot),
        CreateComputer(Race::Protoss, sc2::Medium)
		});

	coordinator.LaunchStarcraft();
	//have to include a hard coded path to the map
	coordinator.StartGame("CactusValleyLE.SC2Map");

	while (coordinator.Update()) {
	}
	return 0;
}
