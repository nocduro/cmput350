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
		assignScout();
	}

	virtual void OnStep() final {
		if (enemypos < 0) {
			scout();
		}
		TryBuildSupplyDepot();
		TryBuildBarracks();

        TryFarmGas();
        //TryUpdgradeBarracks();
        TryBuildFactory();
        TryBuildStarport();
        
        

	}

	virtual void OnUnitIdle(const Unit* unit) final {
        const ObservationInterface* observation = Observation();
		switch (unit->unit_type.ToType()) {
            // if command center is idle, makes scvs (should be running constantly)
    		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
                size_t scvCount = CountUnitType(UNIT_TYPEID::TERRAN_SCV);
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
                    if (countSCV <14){
                        Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);// send idle worker to mineral patch
                    }
                    else{
                        Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER, refinery_target);
                    }
    				break;
    			}
    		}
    		case UNIT_TYPEID::TERRAN_BARRACKS: {
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
				
				if (marinecount==1 && !earlyAttacked) {
					scouter = unit; 
				}
    			
    			if (enemypos<0) { //check if we already found enemy base
    				// start scouting
                    /*
					if (unit == scouter && scouter->health > 0) {
						std::cout << "scouter still alive" << std::endl;
						Actions()->UnitCommand(scouter, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[scouting]); //move marine to enemy base location
						++scouting; //move to next target next time
					}
					else if(!scouter->is_alive){
						enemypos = scouting-1;
						std::cout << "enemy at pos " << enemypos << std::endl;
					}*/

    			}
				
    			//small early game rush attempt
				if (marinecount > 12 && !earlyAttacked && enemypos >= 0) { //check if we have enough marines
					earlyAttacked = 1; //dont do it again
					earlyrush(marinecount); //ATTACC
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
                    std::cout << "here" << std::endl;
                    Actions()->UnitCommand(unit, ABILITY_ID::BUILD_TECHLAB);
                    std::cout << "here2" << std::endl;
                    if (counttech_lab >=1){
                        haveTechLab = true;
                        std::cout << "here3" << std::endl;
                        
                    }
                }else{
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SIEGETANK);
                    std::cout << "here4" << std::endl;
                }
            }
    		default: {
    			break;
    		}

		}
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
			}
			else if (!scouter->is_alive) {
				enemypos = scouting;
				std::cout << "enemy at pos " << enemypos << std::endl;
			}
		}
	}
	//attacks early game with marinecount marines to nearest enemy
	void earlyrush(size_t marinecount) {
		const GameInfo& game_info = Observation()->GetGameInfo();
		sc2::Units marines = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE)); //get our marine units
			
		for (size_t i = 0; i < marinecount; i++) //iterate through our marines
		{
			Actions()->UnitCommand(marines[i], ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[enemypos]); //attack selected target
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

        if (observation->GetFoodUsed() == 14) {
            // std::cout << "BUILD SUPPLY DEPOT" << std::endl;
            return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
        }

		// If we are not supply capped, don't build a supply depot.
		if (observation->GetFoodUsed() <= observation->GetFoodCap() - 15)
			return false;

        // std::cout << observation->GetFoodUsed() << " " << observation->GetFoodCap() << std::endl;

		// Try and build a depot. Find a random SCV and give it the order.
		return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
	}

    // attempts to build a refinery on gas patch
    bool TryBuildRefinery() {
        const ObservationInterface* observation = Observation();

        if (observation->GetFoodUsed() == 14 || observation->GetFoodUsed() == 16) {
            // std::cout << "BUILD REFINERY" << std::endl;
            return TryBuildStructure(ABILITY_ID::BUILD_REFINERY);
        }

        return TryBuildStructure(ABILITY_ID::BUILD_REFINERY);
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

        // build first barracks only when supply is at 16
        if (observation->GetFoodUsed() == 16) {
            // std::cout << "BUILD BARRACKS" << std::endl;
        }

        // if we have more than 1 barracks, don't build anymore
		if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) >= 1) {
			return false;
		}

		return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS); // conditions were passed and we delegate to TryBuildStructure()
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
                std::cout << "here" << std::endl;
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
