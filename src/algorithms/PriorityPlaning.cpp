//
// Created by jakub on 10/6/26.
//

#include "PriorityPlaning.h"

#include <queue>

void PriorityPlaning::resolve(const Map &map, const std::vector<Scenario> &scenarios) const {
    for (const auto &scenario : scenarios) {
        AStar(map, scenario);
    }
}

void PriorityPlaning::AStar(const Map &map, const Scenario &scenario) const {
    std::queue<PriorityPlaningState> queue;
    std::set<PriorityPlaningState> visited; //visited in this run

    queue.push({scenario.startX, scenario.startY, 0});

    while (!queue.empty()) {
        
    }
}
