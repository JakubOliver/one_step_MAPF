//
// Created by jakub on 10/11/26.
//

#include "Grouping.h"
#include "../MemoryAgent.h"

#include <algorithm>
#include <queue>
#include <set>

namespace {

// Checking every reachable position is unnecessary: only shared intermediate
// positions can connect non-adjacent agents.
[[maybe_unused]] bool canShareDestination_slower(
    const Agent& first,
    const Agent& second,
    const Map& map
) {
    std::set<AgentCord> destinations;
    destinations.insert(first.position);
    for (const auto& position : first.getMoveTo()) {
        if (map.isInBounds(position) && !map.isBlocked(position)) {
            destinations.insert(position);
        }
    }

    if (destinations.contains(second.position)) {
        return true;
    }
    for (const auto& position : second.getMoveTo()) {
        if (map.isInBounds(position) && !map.isBlocked(position) &&
            destinations.contains(position)) {
            return true;
        }
    }
    return false;
}

bool canShareDestination(const Agent& first, const Agent& second, const Map& map) {
    const auto [x1, y1] = first.position;
    const auto [x2, y2] = second.position;
    const auto dx = x1 > x2 ? x1 - x2 : x2 - x1;
    const auto dy = y1 > y2 ? y1 - y2 : y2 - y1;

    // Adjacent agents can move into each other's current positions.
    if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
        return true;
    }

    if (dx == 1 && dy == 1) {
        return !map.isBlocked({x1, y2}) || !map.isBlocked({x2, y1});
    }
    if (dx == 2 && dy == 0) {
        return !map.isBlocked({std::min(x1, x2) + 1, y1});
    }
    if (dx == 0 && dy == 2) {
        return !map.isBlocked({x1, std::min(y1, y2) + 1});
    }
    return false;
}

}

template<typename AgentType>
std::vector<std::vector<AgentType>> getGroupsImpl(
    const std::map<AgentCord, AgentType>& agents,
    const Map* map
) {
    std::vector<std::vector<AgentType>> groups;
    std::set<AgentCord> visited;

    for (const auto& agent_entry : agents) {
        const auto& pos = agent_entry.first;

        if (visited.contains(pos)) {
            continue;
        }

        auto& group = groups.emplace_back();
        std::queue<AgentCord> pending;
        pending.push(pos);
        visited.insert(pos);

        while (!pending.empty()) {
            const auto current_position = pending.front();
            pending.pop();

            const auto& current_agent = agents.at(current_position);
            group.push_back(current_agent);

            for (const auto& neighbor : current_agent.getNeighbors()) {
                if (
                    agents.contains(neighbor) &&
                    !visited.contains(neighbor) &&
                    (!map || canShareDestination(current_agent, agents.at(neighbor), *map))
                ) {
                    visited.insert(neighbor);
                    pending.push(neighbor);
                }
            }
        }
    }

    return groups;
}

template<typename AgentType>
std::vector<std::vector<AgentType>> Grouping::getGroups(
    const std::map<AgentCord, AgentType>& agents
) {
    return getGroupsImpl(agents, nullptr);
}

template<typename AgentType>
std::vector<std::vector<AgentType>> Grouping::getGroups(
    const std::map<AgentCord, AgentType>& agents,
    const Map& map
) {
    return getGroupsImpl(agents, &map);
}

template std::vector<std::vector<Agent>> Grouping::getGroups(
    const std::map<AgentCord, Agent>& agents
);
template std::vector<std::vector<MemoryAgent>> Grouping::getGroups(
    const std::map<AgentCord, MemoryAgent>& agents
);
template std::vector<std::vector<Agent>> Grouping::getGroups(
    const std::map<AgentCord, Agent>& agents, const Map& map
);
template std::vector<std::vector<MemoryAgent>> Grouping::getGroups(
    const std::map<AgentCord, MemoryAgent>& agents, const Map& map
);
