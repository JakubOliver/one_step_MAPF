//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include <cstdint>
#include <functional>
#include <string>

#include "../Agent.h"
#include "Algorithm.h"

class CSPTest;

class CSP : public Algorithm {
private:
    friend class CSPTest;

    static int64_t manhattanDistanceFitness(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent
    );

public:
    using FitnessFunction = std::function<int64_t(
        const Map&,
        const std::vector<Agent>&,
        const Agent&
    )>;

    static FitnessFunction getFitness(const std::string& name);

    CSP();
    explicit CSP(std::function<bool()> interruption_requested);
    explicit CSP(
        FitnessFunction fitness,
        std::function<bool()> interruption_requested
    );

private:

    std::function<bool()> interruption_requested;
    bool group_solve_interrupted = false;
    FitnessFunction fitness;

    std::vector<AgentPath> resolve_(
        const Map& map,
        const std::vector<Scenario>& scenarios
    ) override;

    std::vector<Agent> processGroup(
        const std::vector<Agent>& agents,
        const Map& map
    );

    std::vector<Agent> processGroup(
        const std::vector<Agent>& agents,
        const Map& map,
        const FitnessFunction& fitness
    );

};

#endif //MAPF_CSP_H
