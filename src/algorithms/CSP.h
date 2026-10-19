//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include "Algorithm.h"
#include "Grouping.h"

class CSP : public Algorithm {
private:
    std::vector<AgentPath> resolve_(
        const Map& map,
        const std::vector<Scenario>& scenarios
    ) override;

    std::vector<Agent> processGroup(
        const std::vector<Agent>& agents,
        std::size_t map_width
    );
public:
    void processGroup_mocked();

    CSP() = default;
};

#endif //MAPF_CSP_H
