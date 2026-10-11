//
// Created by jakub on 10/5/26.
//

#include <iostream>

#include "InputLoader.h"
#include <argparse/argparse.hpp>

#include "Exporter.h"
#include "algorithms/PriorityPlanning.h"
#include "algorithms/Grouping.h"

#include "../external/mapf-visualizer/src/Visualizer.h"

int main(int argc, char** argv) {
    argparse::ArgumentParser program("MAPF");

    program.add_argument("map")
        .help("path to the map file");
    program.add_argument("scenario")
        .help("path to the scenario file");
    program.add_argument("--algorithm")
        .default_value("PriorityPlanning");
    program.add_argument("--visualize")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--output")
        .default_value(AGENT_PATH_FILE_NAME);

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
    if (algorithm_name == "PriorityPlanning") {
        agents = PriorityPlanning().resolve(map, scenarios);
    } else {
        std::cerr << "Unknown algorithm: " << algorithm_name << std::endl;
        return 1;
    }

    auto output_file = program.get<std::string>("--output");
    Exporter::export_agents(agents, output_file);

    if (program.get<bool>("--visualize")) {
        char* visualizer_argv[] = {
            "mapf-visualizer",
            map_file.data(),
            output_file.data()
        };

        //TODO: remove constant
        Visualizer::run(3, visualizer_argv);
    }

    /*
    std::map<std::pair<std::size_t, std::size_t>, GroupAgent> groupAgents = {
        {{3,3}, GroupAgent(3,3)},
        {{4,3}, GroupAgent(4,3)},
        {{6,6}, GroupAgent(6,6)}
    };

    auto groups = Grouping().getGroups(groupAgents);
    for (const auto& group : groups) {
        std::cout << "Group: " << std::endl;
        for (const auto& agent : group) {
            std::cout << agent.position.first << ", " << agent.position.second << std::endl;
        }
    }
    */

    return 0;
}