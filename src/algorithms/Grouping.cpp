//
// Created by jakub on 10/11/26.
//

#include "Grouping.h"

#include <limits>

std::vector<std::vector<Agent> > Grouping::getGroups(
    const std::map<AgentCord, Agent>& agents
) {
    std::vector<std::vector<Agent>> groups;
    std::map<AgentCord, std::size_t> groupForAgent;
    std::size_t groupIndex = 0;

    for (const auto& [pos, agent] : agents) {
        if (!groupForAgent.contains(pos)) {
            groupForAgent[pos] = groupIndex;
            groups.emplace_back();
            groupIndex++;
        }

        for (const auto& neighbor : agent.getNeighbors()) {
            if (agents.contains(neighbor)) {
                groupForAgent[neighbor] = groupForAgent.at(pos);
            }
        }

        groups.at(groupForAgent.at(pos)).push_back(agent);
    }

    return groups;
}

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
