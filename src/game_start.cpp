#include "state.h"

sc::result GameStart_Refinery::react(const StepEvent& event) {
	return discard_event();
}

sc::result GameStart_BuildArmy::react(const StepEvent& event) {
	return discard_event();
}