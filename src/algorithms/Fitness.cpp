#include "Fitness.h"
#include "DistanceCache.h"

#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

uint64_t manhattanDistance(const Agent& agent) {
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

int64_t linearCost(uint64_t distance, bool staying) {
    constexpr auto max_distance = (std::numeric_limits<int64_t>::max() - 1) / 2;
    if (distance > static_cast<uint64_t>(max_distance)) {
        throw std::overflow_error("Distance fitness is too large");
    }
    return 2 * static_cast<int64_t>(distance) + (staying ? 1 : 0);
}

int64_t squaredCost(uint64_t distance, bool staying) {
    constexpr auto max_squared = (std::numeric_limits<int64_t>::max() - 1) / 2;
    if (distance != 0 && distance > static_cast<uint64_t>(max_squared) / distance) {
        throw std::overflow_error("Squared distance fitness is too large");
    }
    return 2 * static_cast<int64_t>(distance * distance) + (staying ? 1 : 0);
}

int64_t piecewiseCost(uint64_t distance, bool staying) {
    constexpr uint64_t threshold = 4;
    constexpr uint64_t slope = 4 * threshold;
    constexpr uint64_t threshold_cost = 2 * threshold * threshold;
    constexpr uint64_t max_cost = std::numeric_limits<int64_t>::max() - 1;

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
    return static_cast<int64_t>(cost) + (staying ? 1 : 0);
}

} // namespace

RandomFitness::RandomFitness() : RandomFitness(std::random_device{}()) {}

RandomFitness::RandomFitness(std::uint32_t seed)
    : generator(seed), distribution(1.0, 1.0) {}

int64_t RandomFitness::evaluate(uint64_t distance, bool is_staying) {
    if (is_staying) {
        return 0;
    }
    if (distance > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
        throw std::overflow_error("Random distance fitness is too large");
    }
    const auto noise = std::llround(distribution(generator));
    const auto base = static_cast<int64_t>(distance);
    if (noise > 0 && base > std::numeric_limits<int64_t>::max() - noise) {
        throw std::overflow_error("Random distance fitness is too large");
    }
    return base + noise;
}

int64_t RandomFitness::operator()(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    return evaluate(manhattanDistance(agent), is_staying);
}

Fitness::DistanceFunction Fitness::shortestPathDistance(
    const Map& map,
    const std::vector<Scenario>& scenarios
) {
    auto distances = std::make_shared<DistanceCache>(map, scenarios);
    return [distances](const Agent& agent) {
        return static_cast<uint64_t>(distances->distance(agent.position, agent.goal));
    };
}

Fitness::FitnessFunction Fitness::getFitness(const std::string& name) {
    return getFitness(name, manhattanDistance);
}

Fitness::FitnessFunction Fitness::getFitness(const std::string& name, DistanceFunction distance) {
    if (!distance) {
        throw std::invalid_argument("CSP distance function must be provided");
    }

    using CostFunction = std::function<int64_t(uint64_t, bool)>;
    CostFunction cost;
    if (name == "manhattan") {
        cost = linearCost;
    } else if (name == "sqrt") {
        cost = [](uint64_t d, bool staying) {
            return std::llround(1000.0 * (std::sqrt(static_cast<double>(d) + 4.0) - 2.0)) +
                   (staying ? 200 : 0);
        };
    } else if (name == "squared") {
        cost = squaredCost;
    } else if (name == "piecewise") {
        cost = piecewiseCost;
    } else if (name == "log") {
        cost = [](uint64_t d, bool staying) {
            return std::llround(1000.0 * std::log1p(static_cast<double>(d))) +
                   (staying ? 200 : 0);
        };
    } else if (name == "random") {
        cost = [random = RandomFitness()](uint64_t d, bool staying) mutable {
            return random.evaluate(d, staying);
        };
    } else {
        throw std::invalid_argument(
            "Unknown CSP fitness: " + name +
            ". Available fitnesses: manhattan, sqrt, squared, piecewise, log, random"
        );
    }

    return [distance = std::move(distance), cost = std::move(cost)](
        const Map&, const std::vector<Agent>&, const Agent& agent, bool staying
    ) {
        return cost(distance(agent), staying);
    };
}
