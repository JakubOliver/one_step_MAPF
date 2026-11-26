//
// Created by jakub on 10/11/26.
//

#ifndef MAPF_GROUPING_H
#define MAPF_GROUPING_H

#include "../Agent.h"
#include "../Map.h"

#include <map>
#include <vector>

class Grouping {
public:
    //TODO: prepsat na iterator
    //TODO: maybe static
    template<typename AgentType>
    std::vector<std::vector<AgentType>> getGroups(
        const std::map<AgentCord, AgentType>& agents
    );

    template<typename AgentType>
    std::vector<std::vector<AgentType>> getGroups(
        const std::map<AgentCord, AgentType>& agents,
        const Map& map
    );
};


#endif //MAPF_GROUPING_H
