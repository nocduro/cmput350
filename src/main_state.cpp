#include "state.h"


sc::result MainState::react(const event1& event) {
	return discard_event();
}

sc::result MainState::react(const CommandCenterIdle& e) {
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