//
// Created by jakub on 10/6/26.
//

#ifndef MAPF_PRIORITYPLANING_H
#define MAPF_PRIORITYPLANING_H

#include "Algorithm.h"

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

class PriorityPlanning : public Algorithm{
private:
    std::set<PriorityPlaningState> states;
    std::map<AgentCord, std::size_t> finalPositions;
    std::map<AgentCord, std::size_t> latestVisitTimes;
    std::size_t latestReservationTime = 0;

    std::vector<AgentPath> resolve_(
        const Map& map,
        const std::vector<Scenario>& scenarios
    ) override;

    AgentPath AStar(const Map& map, const Scenario& scenario) const;
    void reservePath(const AgentPath& agent);

    bool hasVertexConflict(
        std::size_t x,
        std::size_t y,
        std::size_t time
    ) const;
    bool hasTransitionConflict(
        AgentCord from,
        AgentCord to,
        std::size_t arrivalTime
    ) const;
    bool canOccupyGoalFrom(
        std::size_t x,
        std::size_t y,
        std::size_t time
    ) const;

public:
    PriorityPlanning() = default;
};


#endif //MAPF_PRIORITYPLANING_H
