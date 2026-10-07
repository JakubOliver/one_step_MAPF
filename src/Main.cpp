//
// Created by jakub on 10/5/26.
//

#include <iostream>

#include "InputLoader.h"
#include <argparse/argparse.hpp>

#include "Exporter.h"
#include "algorithms/PriorityPlaning.h"

#include "../external/mapf-visualizer/src/Visualizer.h"

int main(int argc, char** argv) {
    argparse::ArgumentParser program("MAPF");

    program.add_argument("map")
        .help("path to the map file");
    program.add_argument("scenario")
        .help("path to the scenario file");
    program.add_argument("--algorithm")
        .default_value("PriorityPlaning");
    program.add_argument("--visualize")
        .default_value(false)
        .implicit_value(true);

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

    auto [map, scenarios] = InputLoader::loadInput(map_file, scenario_file);

    auto algorithm_name = program.get<std::string>("--algorithm");

    std::vector<Agent> agents;
    if (algorithm_name == "PriorityPlaning") {
        agents = PriorityPlaning().resolve(map, scenarios);
    } else {
        std::cerr << "Unknown algorithm: " << algorithm_name << std::endl;
        return 1;
    }

    Exporter::export_agents(agents);

    if (program.get<bool>("--visualize")) {
        std::string AGENT_PATH_FILE_NAME_FOR_VISUALIZER = AGENT_PATH_FILE_NAME;

        char* visualizer_argv[] = {
            "mapf-visualizer",
            map_file.data(),
            AGENT_PATH_FILE_NAME_FOR_VISUALIZER.data()
        };

        //TODO: remove constant
        Visualizer::run(3, visualizer_argv);
    }

    return 0;
}