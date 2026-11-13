#ifndef MAPF_ARGUMENTS_H
#define MAPF_ARGUMENTS_H

#include <optional>
#include <string>

namespace util {

struct Arguments {
    std::string map_file;
    std::string scenario_file;
    std::string algorithm;
    std::string fitness;
    int memory_size;
    int memory_penalty;
    bool prevent_immediate_return;
    bool visualize;
    std::string output_file;
    bool validate;
};

std::optional<Arguments> parseArguments(
    int argc,
    char** argv,
    bool visualizer_available
);

} // namespace util

#endif // MAPF_ARGUMENTS_H
