//
// Created by jakub on 10/5/26.
//

#ifndef MAPF_SCENARIO_H
#define MAPF_SCENARIO_H

#include <iostream>

class Scenario {
public:
    const int bucket;
    const std::string map;
    const int width;
    const int height;
    const int startX;
    const int startY;
    const int goalX;
    const int goalY;
    const double optimalDistance;

    void print(std::ostream& os) const {
        os << "Scenario: " << map << " " << bucket << " " << width << " " << height << " " << startX << " " << startY << " " << goalX << " " << goalY << " " << optimalDistance;
    }

    inline friend std::ostream& operator<<(std::ostream& os, const Scenario& scenario) {
        scenario.print(os); return os;
    }
};


#endif //MAPF_SCENARIO_H
