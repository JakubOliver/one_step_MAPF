#ifndef MAPF_RUN_ALGORITHM_H
#define MAPF_RUN_ALGORITHM_H

#include "io/Arguments.h"
#include "model/AgentPath.h"
#include "model/Map.h"
#include "model/Scenario.h"

#include <vector>

namespace util {

std::vector<AgentPath> runAlgorithm(
    const Arguments& arguments,
    const Map& map,
    const std::vector<Scenario>& scenarios
);

} // namespace util

#endif // MAPF_RUN_ALGORITHM_H
