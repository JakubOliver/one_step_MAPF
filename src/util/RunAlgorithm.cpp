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

    auto fitness = arguments.fitness == "shortest-path"
        ? Fitness::shortestPathFitness(map, scenarios)
        : CSP::getFitness(arguments.fitness);
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
