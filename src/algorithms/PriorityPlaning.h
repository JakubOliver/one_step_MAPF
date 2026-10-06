//
// Created by jakub on 10/6/26.
//

#ifndef MAPF_PRIORITYPLANING_H
#define MAPF_PRIORITYPLANING_H

#include "../Map.h"
#include "../Scenario.h"

#include <vector>
#include <set>

struct PriorityPlaningState {
public:
    std::size_t x;
    std::size_t y;
    std::size_t time;
};

class PriorityPlaning {
private:
    std::set<PriorityPlaningState> states;
    std::size_t agentCounter = 0;

    void AStar(const Map& map, const Scenario& scenario) const;
public:
    PriorityPlaning() = default;

    void resolve(const Map& map, const std::vector<Scenario>& scenarios) const;
};


#endif //MAPF_PRIORITYPLANING_H
