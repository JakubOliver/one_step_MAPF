//
// Created by jakub on 10/7/26.
//

#include "Exporter.h"

#include <fstream>

void Exporter::export_agents(
    const std::vector<AgentPath>& agents,
    const std::vector<Scenario>& scenarios,
    const std::string& outputFile
) {
    std::size_t maxPathSize = 0;
    for (const AgentPath& agent : agents) {
        maxPathSize = std::max(maxPathSize, agent.getLength());
    }

    std::ofstream file(outputFile);

    file << "goals=";

    for (const auto& scenario : scenarios) {
        file << '(' << scenario.goalX << ',' << scenario.goalY << ')' << ',';
    }

    file << '\n';

    for (std::size_t i = 0; i < maxPathSize; i++) {
        file << i << ":";

        for (const AgentPath& agent : agents) {
            auto [x, y] = agent.getPathAtTime(i);

            file << '(' << x << ',' << y << ')' << ',';
        }

        file << '\n';
    }
}
