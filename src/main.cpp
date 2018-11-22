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
        

        for (int i = 0; i < 20; ++i){
            ReplayPlayerInfo replayPlayerInfo;
            Coordinator coordinator;
            coordinator.LoadSettings(argc, argv);
            coordinator.SetMultithreaded(true);

            coordinator.SetStepSize(10);
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
            std::cout << "we won: " << replayPlayerInfo.game_result << std::endl;

            std::cout << "GAME COMPLETE" << std::endl;

            coordinator.LeaveGame();
        }

        for (int i = 0; i < 20; ++i){
            ReplayPlayerInfo replayPlayerInfo;
        Coordinator coordinator;
        coordinator.LoadSettings(argc, argv);
        coordinator.SetMultithreaded(true);

        coordinator.SetStepSize(10);
            Bot bot;
            coordinator.SetParticipants({
                CreateParticipant(Race::Terran, &bot),
                CreateComputer(Race::Terran)
                });
            coordinator.LaunchStarcraft();
            //have to include a hard coded path to the map
            coordinator.StartGame("CactusValleyLE.SC2Map");

            while (coordinator.Update()) {
            }


            std::cout << "GAME COMPLETE" << std::endl;

            coordinator.LeaveGame();
        }

        for (int i = 0; i < 20; ++i){
            ReplayPlayerInfo replayPlayerInfo;
        Coordinator coordinator;
        coordinator.LoadSettings(argc, argv);
        coordinator.SetMultithreaded(true);

        coordinator.SetStepSize(10);
            Bot bot;
            coordinator.SetParticipants({
                CreateParticipant(Race::Terran, &bot),
                CreateComputer(Race::Zerg)
                });
            coordinator.LaunchStarcraft();
            //have to include a hard coded path to the map
            coordinator.StartGame("CactusValleyLE.SC2Map");

            while (coordinator.Update()) {
            }

            std::cout << "GAME COMPLETE" << std::endl;

            coordinator.LeaveGame();
        }
    }



    

    return 0;
}