//
// Created by jakub on 10/5/26.
//

#ifndef MAPF_SCENARIO_H
#define MAPF_SCENARIO_H

#include <iostream>

class Scenario {
public:
    const std::size_t bucket;
    const std::string map;
    const std::size_t width;
    const std::size_t height;
    const std::size_t startX;
    const std::size_t startY;
    const std::size_t goalX;
    const std::size_t goalY;
    const double optimalDistance;

    void print(std::ostream& os) const {
        os << "Scenario: " << map << " " << bucket << " " << width << " " << height << " " << startX << " " << startY << " " << goalX << " " << goalY << " " << optimalDistance;
    }

    inline friend std::ostream& operator<<(std::ostream& os, const Scenario& scenario) {
        scenario.print(os); return os;
    }
};


#endif //MAPF_SCENARIO_H
