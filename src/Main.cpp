//
// Created by jakub on 10/5/26.
//

#include <iostream>
#include <stdexcept>

#include "InputLoader.h"
#include <argparse/argparse.hpp>

#include "Exporter.h"
#include "algorithms/CSP.h"
#include "algorithms/PriorityPlanning.h"
#include "util/InterruptHandler.h"

#include "validation/Validator.h"

#if MAPF_WITH_VISUALIZER
#include "../external/mapf-visualizer/src/Visualizer.h"
#endif

int main(int argc, char** argv) {
    argparse::ArgumentParser program("MAPF");

    program.add_argument("map")
        .help("path to the map file");
    program.add_argument("scenario")
        .help("path to the scenario file");
    program.add_argument("--algorithm")
        .default_value("PriorityPlanning");
    program.add_argument("--fitness")
        .default_value("manhattan")
        .help("CSP fitness function");
    program.add_argument("--memory-size")
        .default_value(0)
        .scan<'i', int>()
        .help("CSP recent-position memory size (0 disables memory)");
    program.add_argument("--memory-penalty")
        .default_value(1)
        .scan<'i', int>()
        .help("CSP penalty for moving to a remembered position");
    program.add_argument("--prevent-immediate-return")
        .default_value(false)
        .implicit_value(true)
        .help("CSP forbids returning to the previous position immediately");
    program.add_argument("--visualize")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--output")
        .default_value(AGENT_PATH_FILE_NAME);
    program.add_argument("--validate")
        .default_value(false)
        .implicit_value(true);

    try {
        program.parse_args(argc, argv);

#if !MAPF_WITH_VISUALIZER
        if (program.get<bool>("--visualize")) {
            throw std::runtime_error(
                "Visualization support is disabled. Reconfigure with "
                "-DMAPF_BUILD_VISUALIZER=ON to use --visualize."
            );
        }
#endif
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

    std::vector<AgentPath> agents;
    if (algorithm_name == "PriorityPlanning") {
        agents = PriorityPlanning().resolve(map, scenarios);
    } else if (algorithm_name == "CSP") {
        util::InterruptHandler interrupt_handler;
        try {
            const auto memory_size = program.get<int>("--memory-size");
            const auto memory_penalty = program.get<int>("--memory-penalty");
            const auto prevent_immediate_return =
                program.get<bool>("--prevent-immediate-return");
            if (memory_size < 0 || memory_penalty <= 0) {
                throw std::invalid_argument(
                    "Memory size must be non-negative and memory penalty must be positive"
                );
            }

            auto fitness = CSP::getFitness(program.get<std::string>("--fitness"));
            auto interrupted = [&interrupt_handler]() {
                return interrupt_handler.wasInterrupted();
            };
            if (memory_size == 0 && !prevent_immediate_return) {
                agents = CSP(std::move(fitness), interrupted).resolve(map, scenarios);
            } else {
                agents = CSP(
                    std::move(fitness),
                    interrupted,
                    {
                        static_cast<std::size_t>(memory_size),
                        memory_size == 0 ? 0 : memory_penalty,
                        prevent_immediate_return
                    }
                ).resolve(map, scenarios);
            }
        } catch (const std::exception& err) {
            std::cerr << err.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Unknown algorithm: " << algorithm_name << std::endl;
        return 1;
    }

    auto output_file = program.get<std::string>("--output");
    Exporter::export_agents(agents, scenarios, output_file);

    if (program.get<bool>("--validate")) {
        const auto [is_valid, validation_error] = Validator::validate(
            map,
            scenarios,
            agents
        );

        if (is_valid) {
            std::cout << "Validation successful." << std::endl;
        } else {
            std::cerr << "Validation failed";
            if (validation_error.has_value()) {
                std::cerr << ": " << *validation_error;
            }
            std::cerr << std::endl;
            return 1;
        }
    }

#if MAPF_WITH_VISUALIZER
    if (program.get<bool>("--visualize")) {
        char visualizer_name[] = "mapf-visualizer";
        char* visualizer_argv[] = {
            visualizer_name,
            map_file.data(),
            output_file.data()
        };

        //TODO: remove constant
        Visualizer::run(3, visualizer_argv);
    }
#endif

    /*
    std::map<AgentCord, Agent> groupAgents = {
        {{3,3}, Agent(3,3)},
        {{4,3}, Agent(4,3)},
        {{6,6}, Agent(6,6)}
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
