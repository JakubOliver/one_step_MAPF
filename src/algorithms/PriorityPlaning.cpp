//
// Created by jakub on 10/6/26.
//

#include "PriorityPlaning.h"

#include <algorithm>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

using Position = std::pair<std::size_t, std::size_t>;

struct SearchState {
    std::size_t x;
    std::size_t y;
    std::size_t time;

    bool operator==(const SearchState& other) const = default;

    bool operator<(const SearchState& other) const {
        return std::tie(time, x, y) < std::tie(other.time, other.x, other.y);
    }
};

struct OpenNode {
    SearchState state;
    std::size_t estimatedCost;
    std::size_t sequence;
};

struct OpenNodeCompare {
    bool operator()(const OpenNode& first, const OpenNode& second) const {
        if (first.estimatedCost != second.estimatedCost) {
            return first.estimatedCost > second.estimatedCost;
        }

        return first.sequence > second.sequence;
    }
};

struct Neighbor {
    std::size_t x;
    std::size_t y;
};

std::size_t distanceBetween(std::size_t first, std::size_t second) {
    return first > second ? first - second : second - first;
}

std::size_t heuristic(Position from, Position to) {
    const auto xDistance = distanceBetween(from.first, to.first);
    const auto yDistance = distanceBetween(from.second, to.second);
    return xDistance + yDistance;
}

std::vector<Neighbor> getNeighbors(const Map& map, const SearchState& state) {
    std::vector<Neighbor> neighbors;
    neighbors.reserve(5);

    const bool hasLeft = state.x > 0;
    const bool hasRight = state.x < map.width - 1;
    const bool hasUp = state.y > 0;
    const bool hasDown = state.y < map.height - 1;

    if (hasLeft && !map.isBlocked(state.x - 1, state.y)) {
        neighbors.push_back({state.x - 1, state.y});
    }

    if (hasRight && !map.isBlocked(state.x + 1, state.y)) {
        neighbors.push_back({state.x + 1, state.y});
    }

    if (hasUp && !map.isBlocked(state.x, state.y - 1)) {
        neighbors.push_back({state.x, state.y - 1});
    }

    if (hasDown && !map.isBlocked(state.x, state.y + 1)) {
        neighbors.push_back({state.x, state.y + 1});
    }

    neighbors.push_back({state.x, state.y});
    return neighbors;
}

std::runtime_error invalidPositionError(
    const std::string& name,
    std::size_t x,
    std::size_t y
) {
    std::ostringstream message;
    message << "The " << name << " position (" << x << ", " << y
            << ") is outside the map or blocked";

    return std::runtime_error(message.str());
}

std::vector<Agent> PriorityPlaning::resolve(
    const Map& map,
    const std::vector<Scenario>& scenarios
) {
    states.clear();
    finalPositions.clear();
    latestVisitTimes.clear();
    latestReservationTime = 0;

    std::vector<Agent> agents;
    agents.reserve(scenarios.size());

    for (const auto& scenario : scenarios) {
        std::cout << "Processing: " << scenario << std::endl;

        auto agent = AStar(map, scenario);
        reservePath(agent);
        agents.push_back(std::move(agent));
    }

    return agents;
}

Agent PriorityPlaning::AStar(const Map& map, const Scenario& scenario) const {
    if (map.width == 0 || map.height == 0) {
        throw std::runtime_error("A* cannot search a map with a zero dimension");
    }

    if (
        !map.isInBounds(scenario.startX, scenario.startY)
        || map.isBlocked(scenario.startX, scenario.startY)
    ) {
        throw invalidPositionError("start", scenario.startX, scenario.startY);
    }

    if (
        !map.isInBounds(scenario.goalX, scenario.goalY)
        || map.isBlocked(scenario.goalX, scenario.goalY)
    ) {
        throw invalidPositionError("goal", scenario.goalX, scenario.goalY);
    }

    if (hasVertexConflict(scenario.startX, scenario.startY, 0)) {
        throw std::runtime_error("The start position conflicts at time 0");
    }

    const auto cellCount = map.width * map.height;
    const auto maximumTime = latestReservationTime + cellCount;
    const SearchState start{
        .x = scenario.startX,
        .y = scenario.startY,
        .time = 0
    };
    const Position goal{scenario.goalX, scenario.goalY};

    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeCompare> open;
    std::set<SearchState> discovered;
    std::map<SearchState, SearchState> parents;
    std::size_t sequence = 0;

    discovered.insert(start);
    open.push({
        .state = start,
        .estimatedCost = heuristic(
            {start.x, start.y},
            goal
        ),
        .sequence = sequence++
    });

    while (!open.empty()) {
        const auto currentNode = open.top();
        open.pop();

        const auto current = currentNode.state;
        if (
            current.x == scenario.goalX
            && current.y == scenario.goalY
            && canOccupyGoalFrom(current.x, current.y, current.time)
        ) {
            std::vector<Position> path;
            auto pathState = current;

            while (true) {
                path.emplace_back(pathState.x, pathState.y);
                if (pathState == start) {
                    break;
                }

                pathState = parents.at(pathState);
            }

            std::reverse(path.begin(), path.end());
            return Agent(std::move(path));
        }

        if (current.time >= maximumTime) {
            continue;
        }

        for (const auto& neighbor : getNeighbors(map, current)) {
            const auto nextTime = current.time + 1;
            if (hasVertexConflict(neighbor.x, neighbor.y, nextTime)) {
                continue;
            }

            const Position from{current.x, current.y};
            const Position to{neighbor.x, neighbor.y};
            if (from != to && hasTransitionConflict(from, to, nextTime)) {
                continue;
            }

            const SearchState next{
                .x = neighbor.x,
                .y = neighbor.y,
                .time = nextTime
            };
            if (!discovered.insert(next).second) {
                continue;
            }

            parents.emplace(next, current);
            open.push({
                .state = next,
                .estimatedCost = nextTime + heuristic(to, goal),
                .sequence = sequence++
            });
        }
    }

    std::ostringstream message;
    message << "No conflict-free path exists from (" << scenario.startX << ", "
            << scenario.startY << ") to (" << scenario.goalX << ", "
            << scenario.goalY << ")";
    throw std::runtime_error(message.str());
}

void PriorityPlaning::reservePath(const Agent& agent) {
    const auto& path = agent.getPath();

    for (std::size_t time = 0; time < path.size(); time++) {
        const auto [x, y] = path[time];
        const auto [previousX, previousY] = time == 0 ? path[0] : path[time - 1];

        states.insert({
            .x = x,
            .y = y,
            .time = time,
            .previousX = previousX,
            .previousY = previousY
        });

        const Position position{x, y};
        const auto latestVisit = latestVisitTimes.find(position);
        if (
            latestVisit == latestVisitTimes.end()
            || latestVisit->second < time
        ) {
            latestVisitTimes.insert_or_assign(position, time);
        }

        latestReservationTime = std::max(latestReservationTime, time);
    }

    finalPositions.insert_or_assign(path.back(), path.size() - 1);
}

bool PriorityPlaning::hasVertexConflict(
    std::size_t x,
    std::size_t y,
    std::size_t time
) const {
    const PriorityPlaningState state{
        .x = x,
        .y = y,
        .time = time,
        .previousX = x,
        .previousY = y
    };
    if (states.contains(state)) {
        return true;
    }

    const auto finalPosition = finalPositions.find({x, y});
    return finalPosition != finalPositions.end()
        && finalPosition->second <= time;
}

bool PriorityPlaning::hasTransitionConflict(
    Position from,
    Position to,
    std::size_t arrivalTime
) const {
    const PriorityPlaningState reverseDestination{
        .x = from.first,
        .y = from.second,
        .time = arrivalTime,
        .previousX = 0,
        .previousY = 0
    };
    const auto reservedState = states.find(reverseDestination);

    return reservedState != states.end()
        && reservedState->previousX == to.first
        && reservedState->previousY == to.second;
}

bool PriorityPlaning::canOccupyGoalFrom(
    std::size_t x,
    std::size_t y,
    std::size_t time
) const {
    const Position position{x, y};
    if (finalPositions.contains(position)) {
        return false;
    }

    const auto latestVisit = latestVisitTimes.find(position);
    return latestVisit == latestVisitTimes.end() || latestVisit->second < time;
}
