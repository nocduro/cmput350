#pragma once

#include <sc2api/sc2_api.h>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>
#include <iostream>
#include <map>
#include <set>
#include "bot.h"
#include "util.h"

using namespace sc2;
namespace sc = boost::statechart;
namespace mpl = boost::mpl;


/*
Forward Declarations
*/
struct MainState;
struct GameStart;
struct GameStart_Refinery;
struct GameStart_SupplyDepot;
struct GameStart_BuildArmy;
struct GameStart_BuildBarracks;
struct EarlyRushState;


// StateMachine is started in `MainState`
// must be defined like depth first search
// most general state, then its sub-states, then the next sibling to the general state
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



/*
EVENTS
*/


// StepEvent is sent on every step of the simulation.
// It is the event usually called to progress the state machine
// and every state should implemenet a handler for it
struct StepEvent : sc::event<StepEvent> {};
struct CommandCenterIdle : sc::event<CommandCenterIdle> {
	CommandCenterIdle(const Unit* u) : unit(u) {};
	const Unit* unit;
};
struct SCVIdle : sc::event<SCVIdle> {
	SCVIdle(const Unit* u) : unit(u) {};
	const Unit* unit;
};
struct BarracksIdle : sc::event<BarracksIdle> {
	BarracksIdle(const Unit* u) : unit(u) {}
	const Unit* unit; 
};
struct MarineIdle : sc::event<MarineIdle> { 
	MarineIdle(const Unit* u) : unit(u) {}
	const Unit* unit; 
};
struct UnitCreated : sc::event<UnitCreated> {
	const Unit* unit;
	UnitCreated(const Unit* u) : unit(u) {}
};


/*
STATES
*/

struct MainState : sc::simple_state<MainState, StateMachine, GameStart> {
	MainState();
	// list of the type of events we can process in MainState
	typedef mpl::list<
		sc::custom_reaction<CommandCenterIdle>,
		sc::custom_reaction<BarracksIdle>,
		sc::custom_reaction<SCVIdle>,
		sc::custom_reaction<MarineIdle>
	> reactions;

	// corresponding reaction functions for each kind of event
	sc::result react(const CommandCenterIdle& e);
	sc::result react(const BarracksIdle& event);
	sc::result react(const SCVIdle& event);
	sc::result react(const MarineIdle& event);
};


struct EarlyRushState : sc::simple_state<EarlyRushState, MainState> {
	EarlyRushState() {
		std::cout << "EarlyRushState state" << std::endl;
	}
};

// this state is responsible for gathering resources early in the game
// and building the starting buildings. It has several substates and starts 
// in the state of trying to build a refinery (?)
struct GameStart : sc::simple_state<GameStart, MainState, GameStart_BuildBarracks> {
	GameStart() {
		std::cout << "GameStart state" << std::endl;
	}
};


struct GameStart_Refinery : sc::simple_state<GameStart_Refinery, GameStart> {
	GameStart_Refinery() {
		std::cout << "GameStart_Refinery state" << std::endl;
		workers_offset = 0;
	}
	typedef mpl::list<
		sc::custom_reaction<StepEvent>,
		sc::custom_reaction<UnitCreated>
	> reactions;
	sc::result react(const StepEvent& event);
	sc::result react(const UnitCreated& event);
	size_t workers_offset;
};

struct GameStart_BuildArmy : sc::simple_state<GameStart_BuildArmy, GameStart> {
	GameStart_BuildArmy() {
		std::cout << "GameStart_BuildArmy state" << std::endl;
	}
	typedef sc::custom_reaction<StepEvent> reactions;
	sc::result react(const StepEvent& event);
};

struct GameStart_BuildBarracks : sc::simple_state<GameStart_BuildBarracks, GameStart> {
	GameStart_BuildBarracks() {
		std::cout << "GameStart_BuildBarracks state" << std::endl;
	}
	typedef sc::custom_reaction<StepEvent> reactions;
	sc::result react(const StepEvent& event);
};




