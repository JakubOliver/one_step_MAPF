//
// Created by jakub on 10/7/26.
//

#ifndef MAPF_EXPORTER_H
#define MAPF_EXPORTER_H

#include <string>
#include <vector>

#include "Agent.h"

constexpr char AGENT_PATH_FILE_NAME[] = "agents.csv";

class Exporter {
public:
    static void export_agents(const std::vector<AgentPath>& agents, const std::string& outputFile = AGENT_PATH_FILE_NAME);
};


#endif //MAPF_EXPORTER_H
