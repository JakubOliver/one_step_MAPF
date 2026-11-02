//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include <functional>
#include <cstdint>

#include "../Agent.h"
#include "Algorithm.h"

class CSPTest;

class CSP : public Algorithm {
private:
    friend class CSPTest;

    using FitnessFunction = std::function<int64_t(
        const Map&,
        const std::vector<Agent>&,
        const Agent&
    )>;

    std::function<bool()> interruption_requested = []() {
        return false;
    }; //TODO: add condition that this is present only in debug mode
    bool group_solve_interrupted = false;

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

    static int64_t defaultFitness(
        const Map& map,
        const std::vector<Agent>& agents,
        const Agent& agent
    );

public:
    CSP() = default;
    explicit CSP(std::function<bool()> interruption_requested);
};

#endif //MAPF_CSP_H
