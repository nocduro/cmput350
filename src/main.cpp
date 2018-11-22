#include "bot.hpp"
#include <sc2api/sc2_api.h>
#include "sc2api/sc2_map_info.h"
#include <iostream>
#include <math.h>

using namespace sc2;

int main(int argc, char* argv[]) {

    enum myRace {Terran, Protoss, Zerg};

    std::cout << "Run once ('r') or simulate many ('s')?" << std::endl;

    char decision;
    std::cin >> decision;

    if (decision == 'r') {
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
    } else if (decision == 's') {
        Coordinator coordinator;
        coordinator.LoadSettings(argc, argv);

        coordinator.SetStepSize(100);

        for (int i = 0; i < 20; ++i){
            Bot bot;
            // myRace race1 = static_cast<myRace>(race);
            coordinator.SetParticipants({
                CreateParticipant(Race::Terran, &bot),
                CreateComputer(Race::Protoss)
                });

            coordinator.LaunchStarcraft();
            //have to include a hard coded path to the map
            coordinator.StartGame("CactusValleyLE.SC2Map");

            while (coordinator.Update()) {
            }

            std::cout << "GAME COMPLETE" << std::endl;
        }
    }



    

    return 0;
}