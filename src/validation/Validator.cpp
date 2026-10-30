#include "Validator.h"

#include "../Agent.h"

#include <algorithm>
#include <map>
#include <set>
#include <utility>

bool Validator::validate(
    const Map& map,
    const std::vector<Scenario>& scenarios,
    const std::vector<AgentPath>& paths
) {
    return (
        pathsMatchScenarios(scenarios, paths)
        && pathsUseValidPositions(map, paths)
        && pathsUseValidMoves(paths)
        && pathsHaveNoVertexCollisions(paths)
        && pathsHaveNoEdgeCollisions(paths)
    );
}

bool Validator::pathsMatchScenarios(
    const std::vector<Scenario>& scenarios,
    const std::vector<AgentPath>& paths
) {
    if (scenarios.size() != paths.size()) {
        return false;
    }

    using PathEndpoints = std::pair<AgentCord, AgentCord>;
    std::map<PathEndpoints, std::size_t> unmatchedScenarios;

    for (const auto& scenario : scenarios) {
        unmatchedScenarios[{
            {scenario.startX, scenario.startY},
            {scenario.goalX, scenario.goalY}
        }]++;
    }

    for (const auto& agentPath : paths) {
        const auto& path = agentPath.getPath();
        if (path.empty()) {
            return false;
        }

        const PathEndpoints endpoints{path.front(), path.back()};
        const auto scenario = unmatchedScenarios.find(endpoints);
        if (scenario == unmatchedScenarios.end() || scenario->second == 0) {
            return false;
        }
        scenario->second--;
    }

    return true;
}

bool Validator::pathsUseValidPositions(
    const Map& map,
    const std::vector<AgentPath>& paths
) {
    for (const auto& agentPath : paths) {
        const auto& path = agentPath.getPath();

        for (const auto& position : path) {
            if (!isValidPosition(map, position)) {
                return false;
            }
        }
    }

    return true;
}

bool Validator::pathsUseValidMoves(const std::vector<AgentPath>& paths) {
    for (const auto& agentPath : paths) {
        const auto& path = agentPath.getPath();

        for (std::size_t time = 1; time < path.size(); ++time) {
            if (!Agent::isValidMove(path[time - 1], path[time])) {
                return false;
            }
        }
    }

    return true;
}

bool Validator::pathsHaveNoVertexCollisions(
    const std::vector<AgentPath>& paths
) {
    const auto maximumPathLength = getMaximumPathLength(paths);

    for (std::size_t time = 0; time < maximumPathLength; ++time) {
        std::set<AgentCord> occupiedPositions;

        for (const auto& path : paths) {
            const auto position = path.getPathAtTime(time);
            if (!occupiedPositions.insert(position).second) {
                return false;
            }
        }
    }

    return true;
}

bool Validator::pathsHaveNoEdgeCollisions(
    const std::vector<AgentPath>& paths
) {
    const auto maximumPathLength = getMaximumPathLength(paths);

    for (std::size_t time = 1; time < maximumPathLength; ++time) {
        for (std::size_t first = 0; first < paths.size(); ++first) {
            const auto firstPrevious = paths[first].getPathAtTime(time - 1);
            const auto firstCurrent = paths[first].getPathAtTime(time);

            for (
                std::size_t second = first + 1;
                second < paths.size();
                ++second
            ) {
                const auto secondPrevious = paths[second].getPathAtTime(
                    time - 1
                );
                const auto secondCurrent = paths[second].getPathAtTime(time);

                if (
                    firstPrevious == secondCurrent
                    && secondPrevious == firstCurrent
                ) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool Validator::isValidPosition(const Map& map, AgentCord position) {
    if (!map.isInBounds(position)) {
        return false;
    }

    const auto cell = map.map.find(position);
    return cell != map.map.end() && !cell->second;
}

std::size_t Validator::getMaximumPathLength(
    const std::vector<AgentPath>& paths
) {
    std::size_t maximumPathLength = 0;

    for (const auto& path : paths) {
        maximumPathLength = std::max(
            maximumPathLength,
            path.getLength()
        );
    }

    return maximumPathLength;
}
