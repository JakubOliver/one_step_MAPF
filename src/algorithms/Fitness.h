#ifndef MAPF_FITNESS_H
#define MAPF_FITNESS_H

#include "../Agent.h"
#include "../Map.h"
#include "../Scenario.h"

#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <vector>

class RandomFitness {
private:
    std::mt19937 generator;
    std::normal_distribution<double> distribution;

public:
    RandomFitness();
    explicit RandomFitness(std::uint32_t seed);

    int64_t evaluate(uint64_t distance, bool is_staying);

    int64_t operator()(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent,
        bool is_staying
    );
};

namespace Fitness {

using FitnessFunction = std::function<int64_t(
    const Map&,
    const std::vector<Agent>&,
    const Agent&,
    bool
)>;
using DistanceFunction = std::function<uint64_t(const Agent&)>;

FitnessFunction getFitness(const std::string& name);
FitnessFunction getFitness(const std::string& name, DistanceFunction distance);
DistanceFunction shortestPathDistance(const Map& map, const std::vector<Scenario>& scenarios);

} // namespace Fitness

#endif // MAPF_FITNESS_H
