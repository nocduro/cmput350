#include "state.h"


sc::result MainState::react(const event1& event) {
	std::cout << "event1! player id: " << context<StateMachine>().Observation()->GetPlayerID() << std::endl;
	return discard_event();
}

sc::result MainState::react(const CommandCenterIdle& e) {
	context<StateMachine>().Actions()->UnitCommand(e.unit, ABILITY_ID::TRAIN_SCV);
	return discard_event();
}

sc::result MainState::react(const BarracksIdle& event) {
	return discard_event();
}

sc::result MainState::react(const SCVIdle& event) {
	return discard_event();
}

sc::result MainState::react(const MarineIdle& event) {
	return discard_event();
}