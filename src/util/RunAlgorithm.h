#ifndef MAPF_RUN_ALGORITHM_H
#define MAPF_RUN_ALGORITHM_H

#include "Arguments.h"
#include "../AgentPath.h"
#include "../Map.h"
#include "../Scenario.h"

#include <vector>

namespace util {

std::vector<AgentPath> runAlgorithm(
    const Arguments& arguments,
    const Map& map,
    const std::vector<Scenario>& scenarios
);

} // namespace util

#endif // MAPF_RUN_ALGORITHM_H
