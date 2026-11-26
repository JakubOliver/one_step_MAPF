#include "io/InputLoader.h"
#include "Benchmark.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using benchmark::Clock;

struct Input {
    std::string map;
    std::string scenario;
};

std::vector<Input> readInputs(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open input list: " + filename);
    }

    std::vector<Input> inputs;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream row(line);
        Input input;
        std::string extra;
        if (!(row >> input.map) || input.map.starts_with('#')) {
            continue;
        }
        if (!(row >> input.scenario) || row >> extra) {
            throw std::runtime_error("Expected a map and scenario path on each line");
        }
        if (!std::filesystem::is_regular_file(input.map) ||
            !std::filesystem::is_regular_file(input.scenario)) {
            throw std::runtime_error("Input file does not exist: " + line);
        }
        inputs.push_back(input);
    }
    if (inputs.empty()) {
        throw std::runtime_error("Input list must contain at least one map/scenario pair");
    }
    return inputs;
}

std::size_t positiveNumber(const char* value, const std::string& name) {
    const std::string text(value);
    std::size_t parsed = 0;
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument(name + " must be a positive integer");
    }
    const auto result = std::stoull(text, &parsed);
    if (result == 0 || result > static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
        throw std::invalid_argument(name + " must be a positive integer");
    }
    return static_cast<std::size_t>(result);
}

std::string csv(const std::string& value) {
    std::string result = "\"";
    for (char c : value) {
        if (c == '"') {
            result += '"';
        }
        result += c;
    }
    return result + '"';
}

double milliseconds(Clock::duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
}

void printStatistics(const char* label, const std::vector<double>& times) {
    std::cerr << label << " (ms): ";
    if (times.empty()) {
        std::cerr << "n/a\n";
        return;
    }

    const auto [minimum, maximum] = std::minmax_element(times.begin(), times.end());
    const auto average = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    std::cerr << "avg=" << average << ", max=" << *maximum << ", min=" << *minimum << '\n';
}

void printHelp(std::ostream& output, const std::string& command) {
    if (command.empty()) {
        output << "Usage: benchmark <command> [arguments]\n\n"
               << "Commands:\n"
               << "  preprocessing  Measure preprocessing only\n"
               << "  algorithm      Measure preprocessing and CSP together\n\n"
               << "Use benchmark <command> --help for command details.\n";
    } else if (command == "preprocessing") {
        output << "Usage: benchmark preprocessing <bfs|lazy-bfs|lazy-astar|astar|lazy-jps> <input-list> [agents-per-map]\n\n"
               << "Measures distance initialization and start validation, excluding file loading.\n";
    } else {
        output << "Usage: benchmark algorithm <bfs|lazy-bfs|lazy-astar|astar|lazy-jps|none> <input-list> [agents-per-map] [timeout-seconds]\n\n"
               << "Measures preprocessing and CSP, excluding file loading and validation.\n"
               << "CSP uses memory size 5 and memory penalty 1.\n"
               << "bfs, lazy-bfs, lazy-astar, astar and lazy-jps use exact distances; none uses Manhattan.\n"
               << "The cooperative timeout includes initialization but cannot interrupt distance searches.\n";
    }
    if (!command.empty()) {
        output << "astar computes each complete distance field with h=0 (Dijkstra) at initialization.\n"
               << "lazy-astar uses A* with Manhattan distance for individual queries.\n";
        output << "The input list contains map and scenario path pairs,\n"
               << "separated by whitespace and relative to the working directory.\n"
               << "The first agents-per-map scenarios are used (default: 10).\n";
        if (command == "algorithm") {
            output << "The timeout per map defaults to 30 seconds.\n";
        }
        output << "Results are written as CSV to standard output; summary goes to standard error.\n"
               << "Statistics exclude cases that fail before a time is recorded.\n";
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        printHelp(std::cout, "");
        return 0;
    }
    if (argc < 2) {
        printHelp(std::cerr, "");
        return 1;
    }
    const std::string command = argv[1];
    if (command != "preprocessing" && command != "algorithm") {
        std::cerr << "Unknown benchmark: " << command << '\n';
        printHelp(std::cerr, "");
        return 1;
    }
    if (argc == 3 && std::string(argv[2]) == "--help") {
        printHelp(std::cout, command);
        return 0;
    }
    const bool run_algorithm = command == "algorithm";
    if (argc < 4 || argc > (run_algorithm ? 6 : 5)) {
        printHelp(std::cerr, command);
        return 1;
    }

    try {
        const std::string method = argv[2];
        if (method != "bfs" && method != "lazy-bfs" && method != "lazy-astar" &&
            method != "astar" && method != "lazy-jps" && !(run_algorithm && method == "none")) {
            throw std::invalid_argument("Unknown preprocessing method: " + method);
        }
        const auto inputs = readInputs(argv[3]);
        const auto agent_count = argc >= 5 ? positiveNumber(argv[4], "Agent count") : 10;
        const auto timeout = argc >= 6 ? positiveNumber(argv[5], "Timeout") : 30;
        if (timeout > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::invalid_argument("Timeout is too large");
        }

        std::cout << "map,scenario,method,agents,unique_goals,elapsed_ms,status\n";
        bool failed = false;
        std::vector<double> all_times;
        std::vector<double> completed_times;
        for (const auto& input : inputs) {
            try {
                auto [map, scenarios] = InputLoader::loadInput(input.map, input.scenario);
                if (scenarios.size() < agent_count) {
                    throw std::runtime_error("Scenario has fewer agents than requested");
                }
                scenarios = std::vector<Scenario>(scenarios.begin(), scenarios.begin() + agent_count);
                std::set<AgentCord> goals;
                for (const auto& scenario : scenarios) {
                    goals.emplace(scenario.goalX, scenario.goalY);
                }

                const auto result = run_algorithm
                    ? benchmark::algorithm(map, scenarios, method, std::chrono::seconds(timeout))
                    : benchmark::preprocessing(map, scenarios, method);
                const auto elapsed = milliseconds(result.elapsed);
                all_times.push_back(elapsed);
                if (result.status == "completed") {
                    completed_times.push_back(elapsed);
                }
                if (result.status == "invalid") {
                    failed = true;
                }
                std::cout << csv(input.map) << ',' << csv(input.scenario) << ',' << method << ','
                          << agent_count << ',' << goals.size() << ',' << elapsed
                          << ',' << result.status << '\n';
            } catch (const std::exception& error) {
                std::cerr << input.map << ": " << error.what() << '\n';
                failed = true;
            }
        }
        std::cerr << "\nCompleted: " << completed_times.size() << '/' << inputs.size() << '\n';
        printStatistics("All measured", all_times);
        printStatistics("Completed", completed_times);
        return failed ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
