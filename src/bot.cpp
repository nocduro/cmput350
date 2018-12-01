#include <sc2api/sc2_api.h>
#include <sc2api/sc2_map_info.h>
#include <iostream>
#include <math.h>
#include "bot.h"
#include "util.h"
#include "state.h"

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
		auto actions = std::bind(&Bot::Actions, this);
		auto observation = std::bind(&Bot::Observation, this);
		sm = StateMachine(observation, actions);
		sm.initiate();
		sm.process_event(event1());
	}

	virtual void OnStep() final {
		sm.process_event(StepEvent());
	}

	virtual void OnUnitIdle(const Unit* unit) final {
		switch (unit->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			sm.process_event(CommandCenterIdle(unit));
			break;
		}
		case UNIT_TYPEID::TERRAN_SCV: {
			sm.process_event(SCVIdle(unit));
			break;
		}
		case UNIT_TYPEID::TERRAN_BARRACKS: {
			sm.process_event(BarracksIdle(unit));
			break;
		}
		case UNIT_TYPEID::TERRAN_MARINE: {
			sm.process_event(MarineIdle(unit));
			break;
		}
		default: {
			break;
		}

		}
	}

	virtual void OnUnitCreated(const Unit* unit) final {}
	virtual void OnUpgradeComplete(UpgradeID) {}
	virtual void OnBuildingConstructionComplete(const Unit* unit) {}
	virtual void OnNydusDetected() {}
	virtual void OnNuclearLaunchDetected() {}
	virtual void OnUnitEnterVision(const Unit* unit) {}
	virtual void OnGameEnd() {}
	virtual void OnError(const std::vector<ClientError>& client_errs, const std::vector<std::string>& protocol_errs = {}) {}
private:
	StateMachine sm;
};

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