#include "bot.hpp"
#include <sc2api/sc2_api.h>
#include "sc2api/sc2_map_info.h"
#include <iostream>
#include <math.h>

using namespace sc2;

int main(int argc, char* argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

    Bot bot;
    coordinator.SetParticipants({
        CreateParticipant(Race::Terran, &bot),
        CreateComputer(Race::Protoss)
        });

    coordinator.LaunchStarcraft();
    //have to include a hard coded path to the map
    coordinator.StartGame("CactusValleyLE.SC2Map");

    while (coordinator.Update()) {
    }

    return 0;
}