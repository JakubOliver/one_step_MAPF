#ifndef MAPF_VALIDATOR_H
#define MAPF_VALIDATOR_H

#include "../AgentPath.h"
#include "../Map.h"
#include "../Scenario.h"

#include <cstddef>
#include <vector>

class Validator {
private:
    static bool pathsMatchScenarios(
        const std::vector<Scenario>& scenarios,
        const std::vector<AgentPath>& paths
    );
    static bool pathsUseValidPositions(
        const Map& map,
        const std::vector<AgentPath>& paths
    );
    static bool pathsUseValidMoves(const std::vector<AgentPath>& paths);
    static bool pathsHaveNoVertexCollisions(
        const std::vector<AgentPath>& paths
    );
    static bool pathsHaveNoEdgeCollisions(
        const std::vector<AgentPath>& paths
    );

    static bool isValidPosition(const Map& map, AgentCord position);
    static std::size_t getMaximumPathLength(
        const std::vector<AgentPath>& paths
    );

public:
    static bool validate(
        const Map& map,
        const std::vector<Scenario>& scenarios,
        const std::vector<AgentPath>& paths
    );
};

#endif //MAPF_VALIDATOR_H
