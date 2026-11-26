//
// Created by jakub on 10/11/26.
//

#include "Grouping.h"
#include "../MemoryAgent.h"

#include <queue>
#include <set>

namespace {

bool canShareDestination(const Agent& first, const Agent& second, const Map& map) {
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
