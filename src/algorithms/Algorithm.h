//
// Created by jakub on 10/11/26.
//

#ifndef MAPF_ALGORITHM_H
#define MAPF_ALGORITHM_H

#include <vector>

#include "../Agent.h"
#include "../Map.h"
#include "../Scenario.h"

class Algorithm {
private:
    virtual std::vector<AgentPath> resolve_(
        const Map& map,
        const std::vector<Scenario>& scenarios
    ) = 0;

public:
    std::vector<AgentPath> resolve(
        const Map& map,
        const std::vector<Scenario>& scenarios
    );

    virtual ~Algorithm() = default;
};


#endif //MAPF_ALGORITHM_H
