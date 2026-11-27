#include "algorithms/csp/OnDemandDistance.h"

#include <array>
#include <limits>
#include <queue>
#include <stdexcept>
#include <tuple>

namespace {

using Direction = std::pair<int, int>;
constexpr std::array<Direction, 4> directions{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

AgentCord step(AgentCord position, Direction direction) {
    return {position.first + direction.first, position.second + direction.second};
}

bool passable(const Map& map, AgentCord position) {
    return map.isInBounds(position) && !map.isBlocked(position);
}

std::size_t index(const Map& map, AgentCord position) {
    return position.second * map.width + position.first;
}

int64_t manhattan(AgentCord a, AgentCord b) {
    return static_cast<int64_t>(a.first > b.first ? a.first - b.first : b.first - a.first) +
           static_cast<int64_t>(a.second > b.second ? a.second - b.second : b.second - a.second);
}

void validateStarts(const Map& map, const std::vector<Scenario>& scenarios) {
    // Connectivity checks preserve early validation without calculating distances.
    std::vector<std::size_t> components(map.width * map.height, 0);
    std::size_t next_component = 0;
    for (const auto& scenario : scenarios) {
        const AgentCord start{scenario.startX, scenario.startY};
        const AgentCord goal{scenario.goalX, scenario.goalY};
        if (!map.isInBounds(start)) {
            throw std::invalid_argument("Shortest-path fitness position is outside the map");
        }
        if (!passable(map, start)) {
            throw std::invalid_argument("Goal is unreachable from the agent's position");
        }

        if (components[index(map, start)] == 0) {
            ++next_component;
            std::queue<AgentCord> pending;
            pending.push(start);
            components[index(map, start)] = next_component;
            while (!pending.empty()) {
                const auto current = pending.front();
                pending.pop();
                for (const auto direction : directions) {
                    const auto next = step(current, direction);
                    if (passable(map, next) && components[index(map, next)] == 0) {
                        components[index(map, next)] = next_component;
                        pending.push(next);
                    }
                }
            }
        }
        if (components[index(map, start)] != components[index(map, goal)]) {
            throw std::invalid_argument("Goal is unreachable from the agent's position");
        }
    }
}

} // namespace

OnDemandDistance::OnDemandDistance(
    const Map& map,
    const std::vector<Scenario>& scenarios,
    const std::string& method
) : map(map), method(method) {
    if (method != "lazy-bfs" && method != "lazy-astar" && method != "astar" &&
        method != "lazy-jps") {
        throw std::invalid_argument("Unknown shortest-path method: " + method);
    }
    for (const auto& scenario : scenarios) {
        const AgentCord goal{scenario.goalX, scenario.goalY};
        if (!passable(map, goal)) {
            throw std::invalid_argument("Shortest-path fitness requires a valid goal");
        }
        if (!fields.contains(goal)) {
            Field field{std::vector<int64_t>(map.width * map.height, -1), {}};
            field.distances[index(map, goal)] = 0;
            if (method == "lazy-bfs") {
                field.pending.push(goal);
            }
            fields.emplace(goal, std::move(field));
        }
    }

    if (method == "astar") {
        // All cells are targets, so h = 0; this is a Dijkstra traversal.
        using Entry = std::pair<int64_t, AgentCord>;
        for (auto& [goal, field] : fields) {
            std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
            open.emplace(0, goal);
            while (!open.empty()) {
                const auto [cost, position] = open.top();
                open.pop();
                if (cost != field.distances[index(map, position)]) {
                    continue;
                }
                for (const auto direction : directions) {
                    const auto next = step(position, direction);
                    if (!passable(map, next)) {
                        continue;
                    }
                    auto& value = field.distances[index(map, next)];
                    if (value < 0 || cost + 1 < value) {
                        value = cost + 1;
                        open.emplace(value, next);
                    }
                }
            }
        }
        for (const auto& scenario : scenarios) {
            distance({scenario.startX, scenario.startY}, {scenario.goalX, scenario.goalY});
        }
    } else {
        validateStarts(map, scenarios);
    }
}

int64_t OnDemandDistance::distance(AgentCord position, AgentCord goal) {
    if (!map.isInBounds(position)) {
        throw std::invalid_argument("Shortest-path fitness position is outside the map");
    }
    auto& field = fields.at(goal);
    auto& result = field.distances[index(map, position)];
    if (result < 0 && method != "astar" && passable(map, position)) {
        if (method == "lazy-bfs") {
            lazyDistance(position, field);
        } else {
            result = search(position, goal, method == "lazy-jps");
        }
    }
    if (result < 0) {
        throw std::invalid_argument("Goal is unreachable from the agent's position");
    }
    return result;
}

void OnDemandDistance::lazyDistance(AgentCord position, Field& field) {
    while (field.distances[index(map, position)] < 0 && !field.pending.empty()) {
        const auto current = field.pending.front();
        field.pending.pop();
        const auto distance = field.distances[index(map, current)];
        for (const auto direction : directions) {
            const auto next = step(current, direction);
            if (passable(map, next) && field.distances[index(map, next)] < 0) {
                field.distances[index(map, next)] = distance + 1;
                field.pending.push(next);
            }
        }
    }
}

// Search structure inspired by https://github.com/hvillanua/jps (JPS8).
// Independently written JPS4 uses horizontal-first pruning and vertical jumps,
// following https://arxiv.org/abs/2501.14816 rather than diagonal movement.
int64_t OnDemandDistance::search(AgentCord start, AgentCord goal, bool jump_points) const {
    struct Node {
        int64_t priority;
        int64_t cost;
        AgentCord position;
        Direction incoming;
        bool operator>(const Node& other) const {
            return std::tie(priority, cost, position) > std::tie(other.priority, other.cost,
                other.position);
        }
    };
    const auto infinity = std::numeric_limits<int64_t>::max();
    std::vector<int64_t> best(map.width * map.height, infinity);
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    best[index(map, start)] = 0;
    open.push({manhattan(start, goal), 0, start, {0, 0}});

    while (!open.empty()) {
        const auto current = open.top();
        open.pop();
        if (current.cost != best[index(map, current.position)]) {
            continue;
        }
        if (current.position == goal) {
            return current.cost;
        }
        for (const auto direction : directions) {
            if (jump_points && current.incoming.second != 0) {
                if (direction.second != 0 && direction != current.incoming) {
                    continue;
                }
                if (direction.first != 0) {
                    const auto previous = step(current.position, {0, -current.incoming.second});
                    if (passable(map, step(previous, direction))) {
                        continue;
                    }
                }
            } else if (jump_points && current.incoming.first != 0 &&
                       direction.first == -current.incoming.first) {
                continue;
            }

            auto next = current.position;
            int64_t length = 0;
            do {
                const auto previous = next;
                next = step(next, direction);
                if (!passable(map, next)) {
                    break;
                }
                ++length;
                bool forced = false;
                if (jump_points && direction.second != 0) {
                    for (const int dx : {-1, 1}) {
                        const Direction side{dx, 0};
                        if (passable(map, step(next, side)) && !passable(map,
                            step(previous, side))) {
                            forced = true;
                        }
                    }
                }
                if (!jump_points || direction.first != 0 || forced || next == goal) {
                    const auto cost = current.cost + length;
                    if (cost < best[index(map, next)]) {
                        best[index(map, next)] = cost;
                        open.push({cost + manhattan(next, goal), cost, next, direction});
                    }
                    break;
                }
            } while (true);
        }
    }
    return -1;
}
