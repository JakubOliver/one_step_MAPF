#ifndef MAPF_BENCHMARK_H
#define MAPF_BENCHMARK_H

#include "Map.h"
#include "Scenario.h"

#include <chrono>
#include <string>
#include <vector>

namespace benchmark {

using Clock = std::chrono::steady_clock;

struct Result {
    Clock::duration elapsed;
    std::string status;
};

Result preprocessing(
    const Map& map,
    const std::vector<Scenario>& scenarios,
    const std::string& method
);
Result algorithm(
    const Map& map,
    const std::vector<Scenario>& scenarios,
    const std::string& method,
    std::chrono::seconds timeout
);

} // namespace benchmark

#endif // MAPF_BENCHMARK_H
