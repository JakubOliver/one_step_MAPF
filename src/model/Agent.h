//
// Created by jakub on 10/30/26.
//

#ifndef MAPF_AGENT_H
#define MAPF_AGENT_H

#include "model/AgentCord.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <generator>
#include <iosfwd>
#include <optional>
#include <utility>

using AgentId = std::size_t;

// Direction priority: up, right, down, left.
enum class CorridorDirection { Up, Right, Down, Left };

struct Agent {
private:
    inline static std::atomic<AgentId> nextId{0};

    static const std::array<std::pair<int, int>, 4> canMoves;
    static const std::array<std::pair<int, int>, 4> diagMoves;

public:
    AgentCord position;
    AgentCord goal;
    std::optional<CorridorDirection> corridor_direction;
    const AgentId id;

    void updateStagnation(std::uint64_t distance, bool left_goal = false);
    std::size_t stagnation() const { return stagnation_count; }

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

private:
    std::optional<std::uint64_t> best_distance;
    std::size_t stagnation_count = 0;
};

#endif //MAPF_AGENT_H
