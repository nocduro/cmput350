#include <sc2api/sc2_api.h>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>
#include "sc2api/sc2_map_info.h"
#include <iostream>
#include <math.h>
#include "bot.h"
#include "util.h"

using namespace sc2;
namespace sc = boost::statechart;
namespace mpl = boost::mpl;

// MainState is the meta state, the parent state of all others
// this way it can handle things common to all sub-states 
// (like scouting, and unit idles) if the sub-state is unable
// to process it.
struct MainState;
struct GameStart;
struct GameStart_Refinery;
struct GameStart_SupplyDepot;
struct GameStart_BuildArmy;
struct EarlyRushState;

struct event1 : sc::event<event1> {};
// event fired when on_step called?
struct StepEvent : sc::event<StepEvent> {};
struct CommandCenterIdle : sc::event<CommandCenterIdle> {
	CommandCenterIdle(const Unit* u) : unit(u) {};
	const Unit* unit;
};
struct SCVIdle : sc::event<SCVIdle> {
	SCVIdle(const Unit* u) : unit(u) {};
	const Unit* unit;
};
struct BarracksIdle : sc::event<BarracksIdle> { const Unit* unit; };
struct MarineIdle : sc::event<MarineIdle> { const Unit* unit; };

// sources: 
// https://github.com/CodesBay/State-Machine-Using-Boost-Statechart
// https://stackoverflow.com/questions/12662891/how-can-i-pass-a-member-function-where-a-free-function-is-expected
// https://github.com/Juniper/contrail-controller/blob/master/src/bgp/state_machine.h


// StateMachine is started in `MainState`
// must be defined like an onion. order matters for some reason?
// most general states, then their sub-states, then the next general state
class StateMachine : public sc::state_machine<StateMachine, MainState> {
public:
	// have to store a pointer to the function to get the observation interface...
	// this is ugly but it works. yay?
	observation_t Observation;
	action_t Actions;

	StateMachine() {};
	~StateMachine() {
	}
	StateMachine(observation_t o, action_t a) : Observation(o), Actions(a) {}

	StateMachine& operator=(const StateMachine &rhs) {
		Observation = rhs.Observation;
		Actions = rhs.Actions;
		return *this;
	}
};


// Main State starts in sub-state `GameStart`
struct MainState : sc::simple_state<MainState, StateMachine, GameStart> {
	MainState() {
		std::cout << "In State: MainState" << std::endl;
	}

	typedef mpl::list<
		sc::custom_reaction<event1>,
		sc::custom_reaction<CommandCenterIdle>,
		sc::custom_reaction<BarracksIdle>,
		sc::custom_reaction<SCVIdle>,
		sc::custom_reaction<MarineIdle>
	> reactions;

	sc::result react(const event1& event) {
		std::cout << "event1! player id: " << context<StateMachine>().Observation()->GetPlayerID() << std::endl;
		return discard_event();
	}

	sc::result react(const CommandCenterIdle& e) {
		context<StateMachine>().Actions()->UnitCommand(e.unit, ABILITY_ID::TRAIN_SCV);
		return discard_event();
	}

	sc::result react(const BarracksIdle& event) {
		return discard_event();
	}

	sc::result react(const SCVIdle& event) {
		return discard_event();
	}

	sc::result react(const MarineIdle& event) {
		return discard_event();
	}
};

struct EarlyRushState : sc::simple_state<EarlyRushState, MainState> {
	EarlyRushState() {
		std::cout << "In State: EarlyRushState" << std::endl;
	}
};

// this state is responsible for gathering resources early in the game
// and building the starting buildings. It has several substates and starts 
// in the state of trying to build a refinery (?)
struct GameStart : sc::simple_state<GameStart, MainState, GameStart_Refinery> {
	GameStart() {
		std::cout << "In state: GameStart" << std::endl;
	}
};

struct GameStart_Refinery : sc::simple_state<GameStart_Refinery, GameStart> {
	GameStart_Refinery() {
		std::cout << "Build refinery state" << std::endl;
	}
	typedef sc::custom_reaction<StepEvent> reactions;
	sc::result react(const StepEvent& event) {
		action_t actions = context<StateMachine>().Actions;
		observation_t observation = context<StateMachine>().Observation;
		if (TryBuildRefinery(actions, observation)) {
			std::cout << "trying to transit" << std::endl;
			return transit<GameStart_SupplyDepot>();
		}
		return discard_event();
	}
};

struct GameStart_SupplyDepot : sc::simple_state<GameStart_SupplyDepot, GameStart> {
	GameStart_SupplyDepot() {
		std::cout << "Build Supply depot state" << std::endl;
	}
	typedef sc::custom_reaction<StepEvent> reactions;
	sc::result react(const StepEvent& event) {
		auto actions = context<StateMachine>().Actions;
		auto observation = context<StateMachine>().Observation;
		if (TryBuildSupplyDepot(actions, observation)) {
			std::cout << "trying to transit" << std::endl;
			return transit<GameStart_BuildArmy>();
		}
		return discard_event();
	}
};

struct GameStart_BuildArmy : sc::simple_state<GameStart_SupplyDepot, GameStart> {
	GameStart_BuildArmy() {
		std::cout << "BuildArmy state" << std::endl;
	}

	typedef sc::custom_reaction<StepEvent> reactions;
	sc::result react(const StepEvent& event) {
		auto observation = context<StateMachine>().Observation();
		int num_marines = 0;
		Units my_units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto unit : my_units) {
			if (unit->unit_type == UNIT_TYPEID::TERRAN_MARINE) {
				++num_marines;
			}
		}
		if (num_marines > 10) {
			std::cout << "trying to transit" << std::endl;
			return transit<EarlyRushState>();
		}
		else {
			return discard_event();
		}
	}
};



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
			sm.process_event(BarracksIdle());
			break;
		}
		case UNIT_TYPEID::TERRAN_MARINE: {
			sm.process_event(MarineIdle());
			break;
		}
		default: {
			break;
		}

		}
	}
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