#include <iostream>
#include <stdexcept>

#include "Exporter.h"
#include "InputLoader.h"
#include "util/Arguments.h"
#include "util/RunAlgorithm.h"
#include "validation/Validator.h"

#if MAPF_WITH_VISUALIZER
#include "../external/mapf-visualizer/src/Visualizer.h"
#endif

int main(int argc, char** argv) {
    auto arguments = util::parseArguments(argc, argv, MAPF_WITH_VISUALIZER);
    if (!arguments) {
        return 1;
    }

    auto [map, scenarios] = InputLoader::loadInput(
        arguments->map_file,
        arguments->scenario_file
    );

    std::vector<AgentPath> agents;
    try {
        agents = util::runAlgorithm(*arguments, map, scenarios);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    Exporter::export_agents(agents, scenarios, arguments->output_file);

    if (arguments->validate) {
        const auto [is_valid, validation_error] = Validator::validate(
            map,
            scenarios,
            agents
        );
        if (!is_valid) {
            std::cerr << "Validation failed";
            if (validation_error.has_value()) {
                std::cerr << ": " << *validation_error;
            }
            std::cerr << std::endl;
            return 1;
        }
        std::cout << "Validation successful." << std::endl;
    }

#if MAPF_WITH_VISUALIZER
    if (arguments->visualize) {
        char visualizer_name[] = "mapf-visualizer";
        char* visualizer_argv[] = {
            visualizer_name,
            arguments->map_file.data(),
            arguments->output_file.data()
        };
        Visualizer::run(3, visualizer_argv);
    }
#endif

    return 0;
}
