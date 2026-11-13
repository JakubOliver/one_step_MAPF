#include "DistanceCache.h"

#include <array>
#include <queue>
#include <stdexcept>

DistanceCache::DistanceCache(const Map& map, const std::vector<Scenario>& scenarios)
    : width(map.width), height(map.height) {
    // TODO: Compare incremental lazy BFS and optimal grid searches such as
    // Jump Point Search or Rectangular Symmetry Reduction for this workload.
    for (const auto& scenario : scenarios) {
        const AgentCord goal{scenario.goalX, scenario.goalY};
        if (distances.contains(goal)) {
            continue;
        }
        if (!map.isInBounds(goal) || map.isBlocked(goal)) {
            throw std::invalid_argument("Shortest-path fitness requires a valid goal");
        }

        std::vector<int64_t> field(width * height, -1);
        std::queue<AgentCord> pending;
        field[goal.second * width + goal.first] = 0;
        pending.push(goal);

        while (!pending.empty()) {
            const auto position = pending.front();
            pending.pop();
            const auto current_distance = field[position.second * width + position.first];

            const auto [x, y] = position;
            const std::array<AgentCord, 4> neighbors = {{
                {x + 1, y}, {x == 0 ? width : x - 1, y},
                {x, y + 1}, {x, y == 0 ? height : y - 1}
            }};
            for (const auto& next : neighbors) {
                if (!map.isInBounds(next) || map.isBlocked(next)) {
                    continue;
                }
                auto& next_distance = field[next.second * width + next.first];
                if (next_distance == -1) {
                    next_distance = current_distance + 1;
                    pending.push(next);
                }
            }
        }
        distances.emplace(goal, std::move(field));
    }

    for (const auto& scenario : scenarios) {
        distance({scenario.startX, scenario.startY}, {scenario.goalX, scenario.goalY});
    }
}

int64_t DistanceCache::distance(AgentCord position, AgentCord goal) const {
    if (position.first >= width || position.second >= height) {
        throw std::invalid_argument("Shortest-path fitness position is outside the map");
    }
    const auto result = distances.at(goal).at(position.second * width + position.first);
    if (result < 0) {
        throw std::invalid_argument("Goal is unreachable from the agent's position");
    }
    return result;
}
