//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include "../Agent.h"
#include "Algorithm.h"

class CSPTest;

class CSP : public Algorithm {
private:
    friend class CSPTest;

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
};

#endif //MAPF_CSP_H
