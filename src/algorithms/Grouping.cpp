//
// Created by jakub on 10/11/26.
//

#include "Grouping.h"

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
