#include "Arguments.h"

#include "../Exporter.h"

#include <argparse/argparse.hpp>
#include <iostream>
#include <stdexcept>

namespace util {

std::optional<Arguments> parseArguments(
    int argc,
    char** argv,
    bool visualizer_available
) {
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

        if (!visualizer_available && program.get<bool>("--visualize")) {
            throw std::runtime_error(
                "Visualization support is disabled. Reconfigure with "
                "-DMAPF_BUILD_VISUALIZER=ON to use --visualize."
            );
        }
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return std::nullopt;
    }

    return Arguments{
        .map_file = program.get<std::string>("map"),
        .scenario_file = program.get<std::string>("scenario"),
        .algorithm = program.get<std::string>("--algorithm"),
        .fitness = program.get<std::string>("--fitness"),
        .memory_size = program.get<int>("--memory-size"),
        .memory_penalty = program.get<int>("--memory-penalty"),
        .prevent_immediate_return = program.get<bool>("--prevent-immediate-return"),
        .visualize = program.get<bool>("--visualize"),
        .output_file = program.get<std::string>("--output"),
        .validate = program.get<bool>("--validate")
    };
}

} // namespace util
