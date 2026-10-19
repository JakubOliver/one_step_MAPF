//
// Created by jakub on 10/6/26.
//

#ifndef MAPF_AGENT_H
#define MAPF_AGENT_H

#include "AgentCord.h"

#include <cstddef>
#include <vector>

class AgentPath {
private:
    std::vector<AgentCord> path;

public:
    AgentPath() = default;
    explicit AgentPath(std::vector<AgentCord> path);

    std::size_t getLength() const;

    const std::vector<AgentCord>& getPath() const;
    AgentCord getPathAtTime(std::size_t time) const;
};


#endif //MAPF_AGENT_H
