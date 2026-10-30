//
// Created by jakub on 10/30/26.
//

#include "Agent.h"

#include <limits>
#include <optional>
#include <ostream>

const std::array<std::pair<int, int>, 4> Agent::canMoves = {{
    {1, 0},
    {-1, 0},
    {0, 1},
    {0, -1}
}};

const std::array<std::pair<int, int>, 4> Agent::diagMoves = {{
    {1, 1},
    {-1, -1},
    {-1, 1},
    {1, -1}
}};

Agent::Agent(std::size_t x, std::size_t y) : Agent(x, y, 0, 0) {}

Agent::Agent(
    std::size_t x,
    std::size_t y,
    std::size_t gx,
    std::size_t gy
) : Agent(AgentCord{x, y}, AgentCord{gx, gy}) {}

Agent::Agent(AgentCord pos, AgentCord g)
    : position(pos), goal(g), id(nextId++) {}

std::generator<AgentCord> Agent::getNeighbors() const {
    auto [x, y] = position;

    auto validMove = [](std::size_t value, int delta)
        -> std::optional<std::size_t>
    {
        if (delta < 0) {
            auto d = static_cast<std::size_t>(-delta);
            if (value < d)
                return std::nullopt;

            return value - d;
        }

        auto d = static_cast<std::size_t>(delta);

        if (value > std::numeric_limits<std::size_t>::max() - d)
            return std::nullopt;

        return value + d;
    };

    for (auto [dx, dy] : canMoves) {
        for (int distance : {1, 2}) {
            auto nx = validMove(x, dx * distance);
            auto ny = validMove(y, dy * distance);

            if (nx && ny)
                co_yield {*nx, *ny};
        }
    }

    for (auto [dx, dy] : diagMoves) {
        auto nx = validMove(x, dx);
        auto ny = validMove(y, dy);

        if (nx && ny)
            co_yield AgentCord{*nx, *ny};
    }
}

std::generator<AgentCord> Agent::getMoveTo() const {
    auto [x, y] = position;

    auto validMove = [](std::size_t value, int delta)
        -> std::optional<std::size_t>
    {
        if (delta < 0) {
            auto d = static_cast<std::size_t>(-delta);
            if (value < d)
                return std::nullopt;

            return value - d;
        }

        auto d = static_cast<std::size_t>(delta);

        if (value > std::numeric_limits<std::size_t>::max() - d)
            return std::nullopt;

        return value + d;
    };

    for (auto [dx, dy] : canMoves) {
        auto nx = validMove(x, dx);
        auto ny = validMove(y, dy);

        if (nx && ny)
            co_yield AgentCord{*nx, *ny};
    }
}

bool Agent::isValidMove(AgentCord from, AgentCord to) {
    const auto xDistance = from.first > to.first
        ? from.first - to.first
        : to.first - from.first;
    const auto yDistance = from.second > to.second
        ? from.second - to.second
        : to.second - from.second;

    return (
        (xDistance == 0 && yDistance <= 1)
        || (yDistance == 0 && xDistance <= 1)
    );
}

bool Agent::isAtGoal() const {
    return position == goal;
}

std::ostream& operator<<(std::ostream& os, const Agent& agent) {
    os << "Agent: ("
       << agent.position.first << ", " << agent.position.second
       << ") -> ("
       << agent.goal.first << ", " << agent.goal.second
       << ')';

    return os;
}
