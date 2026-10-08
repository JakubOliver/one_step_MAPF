//
// Created by jakub on 10/6/26.
//

#ifndef MAPF_PRIORITYPLANING_H
#define MAPF_PRIORITYPLANING_H

#include "../Agent.h"
#include "../Map.h"
#include "../Scenario.h"

#include <cstddef>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

struct PriorityPlaningState {
    std::size_t x;
    std::size_t y;
    std::size_t time;
    std::size_t previousX;
    std::size_t previousY;

    bool operator<(const PriorityPlaningState& other) const {
        return std::tie(time, x, y) < std::tie(other.time, other.x, other.y);
    }
};

class PriorityPlanning {
private:
    std::set<PriorityPlaningState> states;
    std::map<std::pair<std::size_t, std::size_t>, std::size_t> finalPositions;
    std::map<std::pair<std::size_t, std::size_t>, std::size_t> latestVisitTimes;
    std::size_t latestReservationTime = 0;

    Agent AStar(const Map& map, const Scenario& scenario) const;
    void reservePath(const Agent& agent);

    bool hasVertexConflict(
        std::size_t x,
        std::size_t y,
        std::size_t time
    ) const;
    bool hasTransitionConflict(
        std::pair<std::size_t, std::size_t> from,
        std::pair<std::size_t, std::size_t> to,
        std::size_t arrivalTime
    ) const;
    bool canOccupyGoalFrom(
        std::size_t x,
        std::size_t y,
        std::size_t time
    ) const;

public:
    PriorityPlanning() = default;

    std::vector<Agent> resolve(
        const Map& map,
        const std::vector<Scenario>& scenarios
    );
};


#endif //MAPF_PRIORITYPLANING_H
