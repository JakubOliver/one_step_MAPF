#include "Fitness.h"
#include "DistanceCache.h"

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

RandomFitness::RandomFitness()
    : RandomFitness(std::random_device{}()) {
}

RandomFitness::RandomFitness(std::uint32_t seed)
    : generator(seed),
      distribution(1.0, 1.0) {
}

/*
Randomize fitness works better (in completion) rather than determistic version, 
but there is problem how to have all agent stopping at the same time. Probably
some condition apply change only if agent is in surrouding would make sense. 
*/
int64_t RandomFitness::operator()(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    const int64_t dx = std::llabs(
        static_cast<int64_t>(agent.position.first) -
        static_cast<int64_t>(agent.goal.first)
    );
    const int64_t dy = std::llabs(
        static_cast<int64_t>(agent.position.second) -
        static_cast<int64_t>(agent.goal.second)
    );

    if (is_staying){
        return 0;
    }

    return dx + dy +
           std::llround(distribution(generator));

    // return dx + dy +
    //        std::llround(distribution(generator)) +
    //        (is_staying ? 1 : 0);
}

namespace {

int64_t manhattanDistanceFitness(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    const int64_t dx = std::llabs(
        static_cast<int64_t>(agent.position.first) -
        static_cast<int64_t>(agent.goal.first)
    );
    const int64_t dy = std::llabs(
        static_cast<int64_t>(agent.position.second) -
        static_cast<int64_t>(agent.goal.second)
    );

    return 2 * (dx + dy) + (is_staying ? 1 : 0);
}

int64_t sqrtDistanceFitness(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    constexpr double offset = 4.0;
    constexpr int64_t scale = 1000;

    const int64_t dx = std::llabs(
        static_cast<int64_t>(agent.position.first) -
        static_cast<int64_t>(agent.goal.first)
    );
    const int64_t dy = std::llabs(
        static_cast<int64_t>(agent.position.second) -
        static_cast<int64_t>(agent.goal.second)
    );

    const auto cost = std::llround(
        scale * (
            std::sqrt(static_cast<double>(dx + dy) + offset) -
            std::sqrt(offset)
        )
    );

    return cost + (is_staying ? 200 : 0);
}

uint64_t checkedManhattanDistance(const Agent& agent) {
    const auto difference = [](std::size_t first, std::size_t second) {
        return first > second ? first - second : second - first;
    };
    const auto dx = difference(agent.position.first, agent.goal.first);
    const auto dy = difference(agent.position.second, agent.goal.second);
    if (dx > std::numeric_limits<uint64_t>::max() - dy) {
        throw std::overflow_error("Manhattan distance is too large");
    }
    return static_cast<uint64_t>(dx) + static_cast<uint64_t>(dy);
}

int64_t squaredDistanceFitness(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    const auto distance = checkedManhattanDistance(agent);
    constexpr auto max_squared =
        (std::numeric_limits<int64_t>::max() - 1) / 2;
    if (distance != 0 && distance > static_cast<uint64_t>(max_squared) / distance) {
        throw std::overflow_error("Squared distance fitness is too large");
    }
    return 2 * static_cast<int64_t>(distance * distance) + (is_staying ? 1 : 0);
}

int64_t piecewiseDistanceFitness(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    constexpr uint64_t threshold = 4;
    constexpr uint64_t slope = 4 * threshold;
    constexpr uint64_t threshold_cost = 2 * threshold * threshold;
    constexpr uint64_t max_cost = std::numeric_limits<int64_t>::max() - 1;
    const auto distance = checkedManhattanDistance(agent);

    uint64_t cost;
    if (distance <= threshold) {
        cost = 2 * distance * distance;
    } else {
        const auto excess = distance - threshold;
        if (excess > (max_cost - threshold_cost) / slope) {
            throw std::overflow_error("Piecewise distance fitness is too large");
        }
        cost = threshold_cost + slope * excess;
    }
    return static_cast<int64_t>(cost) + (is_staying ? 1 : 0);
}

int64_t logarithmicDistanceFitness(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    constexpr double scale = 1000.0;
    const auto distance = checkedManhattanDistance(agent);
    return std::llround(scale * std::log1p(static_cast<double>(distance))) +
           (is_staying ? 200 : 0);
}

} // namespace

Fitness::FitnessFunction Fitness::shortestPathFitness(
    const Map& map,
    const std::vector<Scenario>& scenarios
) {
    auto distances = std::make_shared<DistanceCache>(map, scenarios);
    return [distances](const Map&, const std::vector<Agent>&, const Agent& agent, bool is_staying) {
        return 2 * distances->distance(agent.position, agent.goal) + (is_staying ? 1 : 0);
    };
}

Fitness::FitnessFunction Fitness::getFitness(const std::string& name) {
    using FitnessFactory = std::function<FitnessFunction()>;

    static const std::vector<std::pair<std::string, FitnessFactory>> fitnesses = {
        {"manhattan", [] {
            return manhattanDistanceFitness;
        }},
        {"sqrt", [] {
            return sqrtDistanceFitness;
        }},
        {"squared", [] {
            return squaredDistanceFitness;
        }},
        {"piecewise", [] {
            return piecewiseDistanceFitness;
        }},
        {"log", [] {
            return logarithmicDistanceFitness;
        }},
        {"random", [] {
            return FitnessFunction(RandomFitness());
        }}
    };

    for (const auto& [fitness_name, fitness_factory] : fitnesses) {
        if (fitness_name == name) {
            return fitness_factory();
        }
    }

    throw std::invalid_argument(
        "Unknown CSP fitness: " + name +
        ". Available fitnesses: manhattan, shortest-path, sqrt, squared, piecewise, log, random"
    );
}

