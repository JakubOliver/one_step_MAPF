//
// Created by jakub on 10/11/26.
//

#ifndef MAPF_GROUPING_H
#define MAPF_GROUPING_H

#include "../AgentCord.h"

#include <cstddef>
#include <vector>
#include <generator>
#include <array>
#include <map>

struct Agent {
private:
    static constexpr std::array<std::pair<int, int>, 4> canMoves = {{
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    }};

    static constexpr std::array<std::pair<int, int>, 4> diagMoves = {{
        {1, 1},
        {-1, -1},
        {-1, 1},
        {1, -1}
    }};
public:
    AgentCord position;
    AgentCord goal;

    Agent(std::size_t x, std::size_t y) : Agent(x, y, 0, 0) {}
    Agent(
        std::size_t x,
        std::size_t y,
        std::size_t gx,
        std::size_t gy
    ) : Agent(AgentCord{x, y}, AgentCord{gx, gy}) {}
    Agent(AgentCord pos, AgentCord g) : position(pos), goal(g) {}

    std::generator<AgentCord> getNeighbors() const;
    std::generator<AgentCord> getMoveTo() const;
};

class Grouping {
public:
    //TODO: prepsat na iterator
    //TODO: maybe static
    std::vector<std::vector<Agent>> getGroups(
        const std::map<AgentCord, Agent>& agents
    );
};


#endif //MAPF_GROUPING_H
