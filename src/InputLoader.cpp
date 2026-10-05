//
// Created by jakub on 10/5/26.
//

#include "InputLoader.h"

#include "csv.hpp"

std::pair<Map, std::vector<Scenario>> InputLoader::loadInput(
    const std::string& mapFileName,
    const std::string& scenarioFileName
) {
    Map map(63, 161, std::map<std::pair<int, int>, bool>{}); //TODO: from file

    csv::CSVFormat format;
    format.delimiter('\t');
    format.header_row(-1);

    csv::CSVReader reader(scenarioFileName, format);
    std::vector<Scenario> scenarios;

    bool headerProcessed = false;

    for (auto& row : reader) {
        if (!headerProcessed) {
            headerProcessed = true;
            continue;
        }

        scenarios.push_back({
            .bucket = row[0].get<int>(),
            .map = row[1].get<std::string>(),
            .width = row[2].get<int>(),
            .height = row[3].get<int>(),
            .startX = row[4].get<int>(),
            .startY = row[5].get<int>(),
            .goalX = row[6].get<int>(),
            .goalY = row[7].get<int>(),
            .optimalDistance = row[8].get<double>()
        });
    }

    return std::make_pair(map, scenarios);
}
