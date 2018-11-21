#pragma once

#include <sc2api/sc2_api.h>
#include "sc2api/sc2_map_info.h"
#include <iostream>
#include <math.h>

using namespace sc2;


class Bot : public Agent {
public:
    virtual void OnGameStart() final;

    virtual void OnStep() final;

    virtual void OnUnitIdle(const Unit* unit) final;

private:
    void earlyrush(size_t marinecount, int enemypos);

    int FindNearestEnemy(const GameInfo& game_info, sc2::Units barracks);

    float Distance(int x1, int y1, int x2, int y2);

    size_t CountUnitType(UNIT_TYPEID unit_type);

    bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV);

    bool TryBuildSupplyDepot();

    bool TryBuildRefinery();

    const Unit* FindNearestMineralPatch(const Point2D& start);

    bool TryBuildBarracks();

    // finds the nearest gas patch in relation to unit
    const Unit* FindNearestGasPatch(const Point2D& start);

    int scouting = 0; //used for scouting all enemy bases
    bool earlyAttacked = 0; //used as a flag to see if we have early rushed or not
    const Unit *scouter; //marine unit used for scouting earlygame
    int enemypos = -1; //index for enemy position
};
