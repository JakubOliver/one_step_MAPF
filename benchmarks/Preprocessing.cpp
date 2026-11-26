#include "Benchmark.h"

#include "algorithms/Fitness.h"

namespace benchmark {

Result preprocessing(
    const Map& map,
    const std::vector<Scenario>& scenarios,
    const std::string& method
) {
    const auto start = Clock::now();
    auto distance = Fitness::shortestPathDistance(map, scenarios, method);
    return {Clock::now() - start, "completed"};
}

} // namespace benchmark
