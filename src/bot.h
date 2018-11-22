#pragma once
#include <functional>
#include <sc2api/sc2_api.h>

typedef std::function<sc2::ActionInterface*()> action_t;
typedef std::function<const sc2::ObservationInterface*()> observation_t;

