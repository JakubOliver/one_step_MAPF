//
// Created by jakub on 10/5/26.
//

#ifndef MAPF_INPUTLOADER_H
#define MAPF_INPUTLOADER_H

#include "Scenario.h"
#include "Map.h"

#include <iostream>
#include <vector>

class InputLoader {
public:
    static std::pair<Map, std::vector<Scenario>> loadInput(
        std::string mapFileName,
        std::string scenarioFileName
    );
};


#endif //MAPF_INPUTLOADER_H
