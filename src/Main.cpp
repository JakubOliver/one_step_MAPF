//
// Created by jakub on 10/5/26.
//

#include <iostream>

#include "InputLoader.h"
#include <argparse/argparse.hpp>

#include "algorithms/PriorityPlaning.h"

int main(int argc, char** argv) {
    argparse::ArgumentParser program("MAPF");

    program.add_argument("map")
        .help("path to the map file");
    program.add_argument("scenario")
        .help("path to the scenario file");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    auto map_file = program.get<std::string>("map");
    auto scenario_file = program.get<std::string>("scenario");
    std::cout << map_file << " " << scenario_file << std::endl;

    auto [map, scenarios] = InputLoader::loadInput(map_file, scenario_file);

    PriorityPlaning().resolve(map, scenarios);

    return 0;
}
