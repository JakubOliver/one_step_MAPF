//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include <functional>

#include "Algorithm.h"
#include "Grouping.h"

class CSPTest;

class CSP : public Algorithm {
private:
    friend class CSPTest;

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

public:
    CSP() = default;
    explicit CSP(std::function<bool()> interruption_requested);
};

#endif //MAPF_CSP_H
