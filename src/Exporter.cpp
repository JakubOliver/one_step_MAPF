//
// Created by jakub on 10/7/26.
//

#include "Exporter.h"

#include <fstream>

void Exporter::export_agents(const std::vector<Agent> &agents) {
    std::size_t maxPathSize = 0;
    for (const Agent& agent : agents) {
        maxPathSize = std::max(maxPathSize, agent.getLength());
    }

    std::ofstream file(AGENT_PATH_FILE_NAME);

    for (std::size_t i = 0; i < maxPathSize; i++) {
        file << i << ":";

        for (const Agent& agent : agents) {
            auto [x, y] = agent.getPathAtTime(i);

            file << '(' << x << ',' << y << ')' << ',';
        }

        file << std::endl;
    }
}
