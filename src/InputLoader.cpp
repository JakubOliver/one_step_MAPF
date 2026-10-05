//
// Created by jakub on 10/5/26.
//

#include "InputLoader.h"

#include "csv.hpp"
#include <fstream>

std::pair<Map, std::vector<Scenario>> InputLoader::loadInput(
    const std::string& mapFileName,
    const std::string& scenarioFileName
) {
    return std::make_pair(
        loadMap(mapFileName),
        loadScenarios(scenarioFileName)
    );
}

Map InputLoader::loadMap(const std::string& fileName) {
    std::fstream mapFile(fileName);

    std::string typeKey, type;
    std::string heightKey;
    std::size_t height;
    std::string widthKey;
    std::size_t width;
    std::string mapKey;

    mapFile >> typeKey >> type;
    mapFile >> heightKey >> height;
    mapFile >> widthKey >> width;
    mapFile >> mapKey;

    //TOOD: add validation

    std::map<std::pair<std::size_t, std::size_t>, bool> mapData;
    std::string line;
    std::size_t row = 0;
    while (getline(mapFile, line)) {
        for (std::size_t column = 0; column < line.length(); column++) {
            char c = line[column];

            if (c == 'T') {
                mapData[std::make_pair(row, column)] = true;
            } else {
                mapData[std::make_pair(row, column)] = false;
            }
        }
    }

    return {
        .type = type,
        .height = height,
        .width = width,
        .map = std::move(mapData)
    };
}

std::vector<Scenario> InputLoader::loadScenarios(const std::string& fileName) {
    csv::CSVFormat format;
    format.delimiter('\t');
    format.header_row(-1);

    csv::CSVReader reader(fileName, format);
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

    return scenarios;
}
