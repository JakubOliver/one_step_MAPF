#include "RunAlgorithm.h"

#include "InterruptHandler.h"
#include "../algorithms/CSP.h"
#include "../algorithms/PriorityPlanning.h"

#include <stdexcept>
#include <utility>

namespace util {

std::vector<AgentPath> runAlgorithm(
    const Arguments& arguments,
    const Map& map,
    const std::vector<Scenario>& scenarios
) {
    if (arguments.algorithm == "PriorityPlanning") {
        return PriorityPlanning().resolve(map, scenarios);
    }
    if (arguments.algorithm != "CSP") {
        throw std::invalid_argument("Unknown algorithm: " + arguments.algorithm);
    }

    InterruptHandler interrupt_handler;
    if (arguments.memory_size < 0 || arguments.memory_penalty <= 0) {
        throw std::invalid_argument(
            "Memory size must be non-negative and memory penalty must be positive"
        );
    }

    // Preserve the original shortest-path fitness as an alias for linear cost.
    const bool shortest_path_alias = arguments.fitness == "shortest-path";
    const auto& fitness_name = shortest_path_alias ? std::string("manhattan") : arguments.fitness;
    const auto& distance_name = shortest_path_alias ? std::string("shortest-path") : arguments.distance;
    Fitness::FitnessFunction fitness;
    if (distance_name == "manhattan") {
        fitness = Fitness::getFitness(fitness_name);
    } else if (distance_name == "shortest-path") {
        fitness = Fitness::getFitness(fitness_name, Fitness::shortestPathDistance(map, scenarios));
    } else {
        throw std::invalid_argument("Unknown CSP distance: " + distance_name);
    }
    auto interrupted = [&interrupt_handler]() {
        return interrupt_handler.wasInterrupted();
    };
    if (arguments.memory_size == 0 && !arguments.prevent_immediate_return) {
        return CSP(std::move(fitness), interrupted).resolve(map, scenarios);
    }

    return CSP(
        std::move(fitness),
        interrupted,
        {
            static_cast<std::size_t>(arguments.memory_size),
            arguments.memory_size == 0 ? 0 : arguments.memory_penalty,
            arguments.prevent_immediate_return
        }
    ).resolve(map, scenarios);
}

} // namespace util
