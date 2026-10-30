//
// Created by jakub on 10/30/26.
//

#ifndef MAPF_AGENT_H
#define MAPF_AGENT_H

#include "AgentCord.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <generator>
#include <iosfwd>
#include <utility>

using AgentId = std::size_t;

struct Agent {
private:
    inline static std::atomic<AgentId> nextId{0};

    static const std::array<std::pair<int, int>, 4> canMoves;
    static const std::array<std::pair<int, int>, 4> diagMoves;

public:
    AgentCord position;
    AgentCord goal;
    const AgentId id;

    Agent(std::size_t x, std::size_t y);
    Agent(
        std::size_t x,
        std::size_t y,
        std::size_t gx,
        std::size_t gy
    );
    Agent(AgentCord pos, AgentCord g);

    std::generator<AgentCord> getNeighbors() const;
    std::generator<AgentCord> getMoveTo() const;

    static bool isValidMove(AgentCord from, AgentCord to);

    bool isAtGoal() const;

    friend std::ostream& operator<<(std::ostream& os, const Agent& agent);
};

#endif //MAPF_AGENT_H
