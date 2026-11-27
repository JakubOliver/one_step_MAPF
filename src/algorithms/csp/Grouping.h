//
// Created by jakub on 10/11/26.
//

#ifndef MAPF_GROUPING_H
#define MAPF_GROUPING_H

#include "model/Agent.h"
#include "model/Map.h"

#include <map>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <utility>

template<typename AgentType>
struct Group {
    std::vector<AgentType> agents;
    // Indices into agents; each undirected edge appears once.
    std::vector<std::pair<std::size_t, std::size_t>> edges;

    std::size_t maxStagnation() const {
        std::size_t result = 0;
        for (const auto& agent : agents) {
            if (!agent.isAtGoal()) {
                result = std::max(result, agent.stagnation());
            }
        }
        return result;
    }

    bool xStagnating(double percentage) const {
        if (!(percentage >= 0 && percentage <= 100)) {
            throw std::invalid_argument("Stagnation percentage must be between 0 and 100");
        }

        std::size_t active = 0;
        std::size_t stagnant = 0;

        for (const auto& agent : agents) {
            if (!agent.isAtGoal()) {
                ++active;
                stagnant += agent.stagnation() > 10;
            }
        }

        return active > 0 && 100.0 * stagnant >= percentage * active;
    }

    bool isStagnating() const { return xStagnating(100); }
    bool halfStagnating() const { return xStagnating(50); }
};

class Grouping {
public:
    //TODO: prepsat na iterator
    //TODO: maybe static
    template<typename AgentType>
    std::vector<Group<AgentType>> getGroups(
        const std::map<AgentCord, AgentType>& agents
    );

    template<typename AgentType>
    std::vector<Group<AgentType>> getGroups(
        const std::map<AgentCord, AgentType>& agents,
        const Map& map
    );
};

#endif //MAPF_GROUPING_H
