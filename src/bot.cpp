#include "bot.hpp"
#include <algorithm>
#include <ctime>

using namespace sc2;

// returns boolean value which indicates if it is geyser
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

void Bot::OnGameStart() {
	std::cout << "Can I get a pogchamp in the chat?" << std::endl;
}

void Bot::OnStep() {
        TryBuildSupplyDepot();
        TryBuildBarracks();
        const GameInfo& game_info = Observation()->GetGameInfo();
        if (enemypos > -1) {//know where the enemy is
            playerpos = getPlayerPos(game_info.enemy_start_locations);
            getPatrolPoints(playerpos, game_info.enemy_start_locations[enemypos]);
        }
        double duration=0;
        if (earlyAttacked) {
            duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
        }
        if (duration > 40) {
            allowPatrol = true;
        }
        
}

// calls when unit is idle
void Bot::OnUnitIdle(const Unit* unit) {
const ObservationInterface* observation = Observation();
        switch (unit->unit_type.ToType()) {
            // if command center is idle, makes scvs (should be running constantly)
            case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
                break;
            }
            
            // if scv is idle, make it do something productive
            case UNIT_TYPEID::TERRAN_SCV: {
                const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
                const Unit* gas_target = FindNearestGasPatch(unit->pos);
                if (!mineral_target) {
                    // TryBuildRefinery();
                    // Actions()->UnitCommand(unit, ABILITY_ID::SMART, gas_target); // send idle worker to gas
                    break;
                }
                if (observation->GetFoodUsed() == 15) {
                    Actions()->UnitCommand(unit, ABILITY_ID::SMART, gas_target); // send idle worker to gas
                }
                if (gas_target){
                    TryBuildRefinery(); // build refinery if a gas patch is found
                    // Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target); // send idle worker to mineral patch
                    // Actions()->UnitCommand(unit, ABILITY_ID::BUILD_REFINERY, gas_target);
                }
                Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target); // send idle worker to mineral patch
                break;
                
            }
            case UNIT_TYPEID::TERRAN_BARRACKS: {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
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
                    if (unit == scouter && scouter->health > 0) {
                        std::cout << "scouter still alive" << std::endl;
                        Actions()->UnitCommand(scouter, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[scouting]); //move marine to enemy base location
                        ++scouting; //move to next target next time
                    }
                    else if(!scouter->is_alive){
                        enemypos = scouting-1;
                        std::cout << "enemy at pos " << enemypos << std::endl;
                        if (enemypos > -1) {
                            
                        }
                    }
                }
                if (earlyAttacked && allowPatrol) {
                    Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, patrolpoints[patrolpos]); //go to patrol point
                    patrolpos++;
                    patrolpos = patrolpos % 3;
                }
                //small early game rush attempt
                if (marinecount > 12 && !earlyAttacked && enemypos >= 0) { //check if we have enough marines
                    earlyAttacked = 1; //dont do it again
                    earlyrush(marinecount); //ATTACC
                }
                break;
            }
            default: {
                break;
            }

        }
}

//attacks early game with marinecount marines to nearest enemy
void Bot::earlyrush(size_t marinecount) {
    start = std::clock();
        const GameInfo& game_info = Observation()->GetGameInfo();
        sc2::Units marines = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE)); //get our marine units
            
        for (size_t i = 0; i < marinecount; i++) //iterate through our marines
        {
            Actions()->UnitCommand(marines[i], ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations[enemypos]); //attack selected target
        }
}

//find nearest enemy will return index of the nearest enemy base using x,y
int Bot::FindNearestEnemy(const GameInfo& game_info, sc2::Units barracks) {
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
float Bot::Distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

size_t Bot::CountUnitType(UNIT_TYPEID unit_type) {
	return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

bool Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
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
            Actions()->UnitCommand(unit_to_build, ability_type_for_structure, FindNearestGasPatch(unit_to_build->pos));
        } else {
          Actions()->UnitCommand(unit_to_build,
            ability_type_for_structure,
            Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
        }

        return true;
}

// attempts to build supply depot
bool Bot::TryBuildSupplyDepot() {
	const ObservationInterface* observation = Observation();

        if (observation->GetFoodUsed() == 14) {
            // std::cout << "BUILD SUPPLY DEPOT" << std::endl;
            return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
        }

        // If we are not supply capped, don't build a supply depot.
        if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
            return false;

        // std::cout << observation->GetFoodUsed() << " " << observation->GetFoodCap() << std::endl;

        // Try and build a depot. Find a random SCV and give it the order.
        return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
    }

// attempts to build a refinery on gas patch
bool Bot::TryBuildRefinery() {
     const ObservationInterface* observation = Observation();

        if (observation->GetFoodUsed() == 14 || observation->GetFoodUsed() == 16) {
            // std::cout << "BUILD REFINERY" << std::endl;
            return TryBuildStructure(ABILITY_ID::BUILD_REFINERY);
        }

        return TryBuildStructure(ABILITY_ID::BUILD_REFINERY);
}

// finds the nearest mineral patch to our current base
const Unit* Bot::FindNearestMineralPatch(const Point2D& start) {
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
bool Bot::TryBuildBarracks() {
	   const ObservationInterface* observation = Observation();

        // can't build barracks without at least one supply depot
        if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
            return false;
        }

        // build first barracks only when supply is at 16
        if (observation->GetFoodUsed() == 16) {
            // std::cout << "BUILD BARRACKS" << std::endl;
        }

        // if we have more than 2 barracks, don't build anymore
        if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) > 2) {
            return false;
        }

        return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS); // conditions were passed and we delegate to TryBuildStructure()
}

sc2::Point2D Bot::getPlayerPos(std::vector<sc2::Point2D> enemylocations) {
   //Starting positions within the map
        std::vector<sc2::Point2D> startlocations; 
        startlocations.push_back(Point2D(33.5, 33.5));
        startlocations.push_back(Point2D(33.5, 158.5));
        startlocations.push_back(Point2D(158.5, 33.5));
        startlocations.push_back(Point2D(158.5, 158.5));
        //(33.5,33.5)   (33.5, 158.5)   (158.5, 33.5)   (158.5, 158.5)
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

// finds the nearest gas patch in relation to unit
const Unit* Bot::FindNearestGasPatch(const Point2D& start) {
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

//function called once we know our enemies position, so we can patrol the correct part of our base
// appends Point2D objects (3) to the vector patrolpoints.
void Bot::getPatrolPoints(Point2D mypos, Point2D enemypos) {
    bool xadj = false; // will use these to determine which patrol points to send units to
        bool yadj = false; //used to indicate if enemy adjacent in any axis
        bool selfcorner[] = { false,false,false,false }; // need to find which corner we are in to filter out garbage patrol points
        // ordered in terms of corners BL, TL, TR, BR
        if (mypos.x < 100) {
            //left side of map
            if (mypos.y < 100) {
                //bottom side of map
                selfcorner[0] = true;
            }
            else {
                //topside of map
                selfcorner[1] = true;
            }
        }
        else {
            //right side of map
            if (mypos.y > 100) {
                //bottom side of map
                selfcorner[2] = true;
            }
            else {
                //topside of map
                selfcorner[3] = true;
            }
        }
        
        
        if (mypos.x == enemypos.x) {
            xadj = true;
        }
        if (mypos.y == enemypos.y) {
            yadj = true;
        }
        // now know enemies position relative to us in terms of adjacency

        std::vector<Point2D> allpatrols; // all the possible patrol points

        int dx, dy;
        const GameInfo& game_info = Observation()->GetGameInfo();

        if (mypos.x > 100) {
            //right side of map so need to take right edge
            dx = game_info.width - mypos.x;
        }
        else {
            dx = mypos.x;
        }

        if (mypos.y > 100) {
            //top side of map so need to take top edge
            dy= game_info.height - mypos.y;
        }
        else {
            dy = mypos.y;
        }
        for (int x = -1; x < 2; ++x) {
            for (int y = -1; y < 2; ++y) {
                Point2D temp((mypos.x + (x*dx)), (mypos.y + (y*dy)));
                allpatrols.push_back(temp);
            }
        }


        /*
        2 5 8
        1 4 7
        0 3 6
        */
        if (selfcorner[0]) {
            //we are at bottom left so need patrol points 2,5,6,7,8
            patrolpoints.push_back(allpatrols[8]); //order matters a bit, but not that much, put in order of importance/impact
            if (xadj) {
                patrolpoints.push_back(allpatrols[5]);
                patrolpoints.push_back(allpatrols[2]);
            }
            else if (yadj) {
                patrolpoints.push_back(allpatrols[7]);
                patrolpoints.push_back(allpatrols[6]);
            }
            else {
                patrolpoints.push_back(allpatrols[5]);
                patrolpoints.push_back(allpatrols[7]);
            }
        }else if (selfcorner[1]) {
            //we are at top left so need patrol points 0,3,6,7,8
            patrolpoints.push_back(allpatrols[6]); 
            if (xadj) {
                patrolpoints.push_back(allpatrols[3]);
                patrolpoints.push_back(allpatrols[0]);
            }
            else if (yadj) {
                patrolpoints.push_back(allpatrols[7]);
                patrolpoints.push_back(allpatrols[8]);
            }
            else {
                patrolpoints.push_back(allpatrols[3]);
                patrolpoints.push_back(allpatrols[7]);
            }
        }else if(selfcorner[2]) {
            //we are at top right so need patrol points 0,3,6,1,2
            patrolpoints.push_back(allpatrols[0]);
            if (xadj) {
                patrolpoints.push_back(allpatrols[3]);
                patrolpoints.push_back(allpatrols[6]);
            }
            else if (yadj) {
                patrolpoints.push_back(allpatrols[2]);
                patrolpoints.push_back(allpatrols[1]);
            }
            else {
                patrolpoints.push_back(allpatrols[3]);
                patrolpoints.push_back(allpatrols[1]);
            }
    
        }else {
            //we are at bottom right so need patrol points 0,1,2,5,8
            patrolpoints.push_back(allpatrols[2]);
            if (xadj) {
                patrolpoints.push_back(allpatrols[5]);
                patrolpoints.push_back(allpatrols[8]);
            }
            else if (yadj) {
                patrolpoints.push_back(allpatrols[1]);
                patrolpoints.push_back(allpatrols[0]);
            }
            else {
                patrolpoints.push_back(allpatrols[5]);
                patrolpoints.push_back(allpatrols[8]);
            }
        }
        return;
}