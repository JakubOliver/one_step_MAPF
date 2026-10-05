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
private:
    static std::vector<Scenario> loadScenarios(const std::string& fileName);
    static Map loadMap(const std::string& fileName);
public:
    static std::pair<Map, std::vector<Scenario>> loadInput(
        const std::string& mapFileName,
        const std::string& scenarioFileName
    );
};


#endif //MAPF_INPUTLOADER_H
