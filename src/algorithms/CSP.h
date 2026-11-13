//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include <cstdint>
#include <functional>
#include <random>
#include <string>

#include "../Agent.h"
#include "../MemoryAgent.h"
#include "Algorithm.h"

class CSPTest;

class RandomFitness {
private:
    std::mt19937 generator;
    std::normal_distribution<double> distribution;

public:
    RandomFitness();
    explicit RandomFitness(std::uint32_t seed);

    int64_t operator()(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent,
        bool is_staying
    );
};

class CSP : public Algorithm {
private:
    friend class CSPTest;

    static int64_t manhattanDistanceFitness(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent,
        bool is_staying
    );

    static int64_t sqrtDistanceFitness(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent,
        bool is_staying
    );

    static int64_t squaredDistanceFitness(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent,
        bool is_staying
    );

    static int64_t piecewiseDistanceFitness(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent,
        bool is_staying
    );

    static int64_t logarithmicDistanceFitness(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent,
        bool is_staying
    );

public:
    using FitnessFunction = std::function<int64_t(
        const Map&,
        const std::vector<Agent>&,
        const Agent&,
        bool
    )>;

    static FitnessFunction getFitness(const std::string& name);

    struct MemoryOptions {
        std::size_t capacity;
        int64_t penalty;
    };

    CSP();
    explicit CSP(std::function<bool()> interruption_requested);
    explicit CSP(
        FitnessFunction fitness,
        std::function<bool()> interruption_requested
    );
    CSP(
        FitnessFunction fitness,
        std::function<bool()> interruption_requested,
        MemoryOptions memory
    );

private:

    std::function<bool()> interruption_requested;
    bool group_solve_interrupted = false;
    FitnessFunction fitness;
    MemoryOptions memory{0, 0};

    std::vector<AgentPath> resolve_(
        const Map& map,
        const std::vector<Scenario>& scenarios
    ) override;

    std::vector<Agent> processGroup(
        const std::vector<Agent>& agents,
        const Map& map
    );

    template<typename AgentType>
    std::vector<AgentType> processGroupTyped(
        const std::vector<AgentType>& agents,
        const Map& map
    );

    template<typename AgentType>
    std::vector<AgentPath> resolveTyped(
        const Map& map,
        const std::vector<Scenario>& scenarios
    );

};

#endif //MAPF_CSP_H
