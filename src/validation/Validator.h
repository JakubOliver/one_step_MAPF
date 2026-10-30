#ifndef MAPF_VALIDATOR_H
#define MAPF_VALIDATOR_H

#include "../AgentPath.h"
#include "../Map.h"
#include "../Scenario.h"

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Validator {
private:
    static std::optional<std::string> pathsMatchScenarios(
        const std::vector<Scenario>& scenarios,
        const std::vector<AgentPath>& paths
    );
    static std::optional<std::string> pathsUseValidPositions(
        const Map& map,
        const std::vector<AgentPath>& paths
    );
    static std::optional<std::string> pathsUseValidMoves(
        const std::vector<AgentPath>& paths
    );
    static std::optional<std::string> pathsHaveNoVertexCollisions(
        const std::vector<AgentPath>& paths
    );
    static std::optional<std::string> pathsHaveNoEdgeCollisions(
        const std::vector<AgentPath>& paths
    );

    static std::size_t getMaximumPathLength(
        const std::vector<AgentPath>& paths
    );

public:
    static std::pair<bool, std::optional<std::string>> validate(
        const Map& map,
        const std::vector<Scenario>& scenarios,
        const std::vector<AgentPath>& paths
    );
};

#endif //MAPF_VALIDATOR_H
