#include "Validator.h"

#include "../Agent.h"

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <utility>

namespace {

std::string formatPosition(AgentCord position) {
    return "(" + std::to_string(position.first) + ", "
        + std::to_string(position.second) + ")";
}

}

std::pair<bool, std::optional<std::string>> Validator::validate(
    const Map& map,
    const std::vector<Scenario>& scenarios,
    const std::vector<AgentPath>& paths
) {
    if (auto error = pathsMatchScenarios(scenarios, paths)) {
        return {false, std::move(error)};
    }
    if (auto error = pathsUseValidPositions(map, paths)) {
        return {false, std::move(error)};
    }
    if (auto error = pathsUseValidMoves(paths)) {
        return {false, std::move(error)};
    }
    if (auto error = pathsHaveNoVertexCollisions(paths)) {
        return {false, std::move(error)};
    }
    if (auto error = pathsHaveNoEdgeCollisions(paths)) {
        return {false, std::move(error)};
    }

    return {true, std::nullopt};
}

std::optional<std::string> Validator::pathsMatchScenarios(
    const std::vector<Scenario>& scenarios,
    const std::vector<AgentPath>& paths
) {
    if (scenarios.size() != paths.size()) {
        return "Expected " + std::to_string(scenarios.size())
            + " paths, but received " + std::to_string(paths.size());
    }

    using PathEndpoints = std::pair<AgentCord, AgentCord>;
    std::map<PathEndpoints, std::size_t> unmatchedScenarios;

    for (const auto& scenario : scenarios) {
        unmatchedScenarios[{
            {scenario.startX, scenario.startY},
            {scenario.goalX, scenario.goalY}
        }]++;
    }

    for (std::size_t pathIndex = 0; pathIndex < paths.size(); ++pathIndex) {
        const auto& path = paths[pathIndex].getPath();
        if (path.empty()) {
            return "Path " + std::to_string(pathIndex) + " is empty";
        }

        const PathEndpoints endpoints{path.front(), path.back()};
        const auto scenario = unmatchedScenarios.find(endpoints);
        if (scenario == unmatchedScenarios.end() || scenario->second == 0) {
            return "Path " + std::to_string(pathIndex) + " from "
                + formatPosition(path.front()) + " to "
                + formatPosition(path.back())
                + " does not match an unmatched scenario";
        }
        scenario->second--;
    }

    return std::nullopt;
}

std::optional<std::string> Validator::pathsUseValidPositions(
    const Map& map,
    const std::vector<AgentPath>& paths
) {
    for (std::size_t pathIndex = 0; pathIndex < paths.size(); ++pathIndex) {
        const auto& path = paths[pathIndex].getPath();

        for (std::size_t time = 0; time < path.size(); ++time) {
            const auto position = path[time];
            std::string reason;

            if (!map.isInBounds(position)) {
                reason = "is outside the map";
            } else {
                const auto cell = map.map.find(position);
                if (cell == map.map.end()) {
                    reason = "is missing from the map";
                } else if (cell->second) {
                    reason = "is blocked";
                }
            }

            if (!reason.empty()) {
                return "Path " + std::to_string(pathIndex) + " position "
                    + formatPosition(position) + " at time "
                    + std::to_string(time) + " " + reason;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::string> Validator::pathsUseValidMoves(
    const std::vector<AgentPath>& paths
) {
    for (std::size_t pathIndex = 0; pathIndex < paths.size(); ++pathIndex) {
        const auto& path = paths[pathIndex].getPath();

        for (std::size_t time = 1; time < path.size(); ++time) {
            if (!Agent::isValidMove(path[time - 1], path[time])) {
                return "Path " + std::to_string(pathIndex)
                    + " has an invalid move from "
                    + formatPosition(path[time - 1]) + " to "
                    + formatPosition(path[time]) + " at time "
                    + std::to_string(time);
            }
        }
    }

    return std::nullopt;
}

std::optional<std::string> Validator::pathsHaveNoVertexCollisions(
    const std::vector<AgentPath>& paths
) {
    const auto maximumPathLength = getMaximumPathLength(paths);

    for (std::size_t time = 0; time < maximumPathLength; ++time) {
        std::map<AgentCord, std::size_t> occupiedPositions;

        for (std::size_t pathIndex = 0; pathIndex < paths.size(); ++pathIndex) {
            const auto position = paths[pathIndex].getPathAtTime(time);
            const auto [occupant, inserted] = occupiedPositions.emplace(
                position,
                pathIndex
            );

            if (!inserted) {
                return "Paths " + std::to_string(occupant->second) + " and "
                    + std::to_string(pathIndex) + " collide at "
                    + formatPosition(position) + " at time "
                    + std::to_string(time);
            }
        }
    }

    return std::nullopt;
}

std::optional<std::string> Validator::pathsHaveNoEdgeCollisions(
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
                    return "Paths " + std::to_string(first) + " and "
                        + std::to_string(second) + " swap positions "
                        + formatPosition(firstPrevious) + " and "
                        + formatPosition(firstCurrent) + " at time "
                        + std::to_string(time);
                }
            }
        }
    }

    return std::nullopt;
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
