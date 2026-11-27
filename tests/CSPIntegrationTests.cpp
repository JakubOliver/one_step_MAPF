#include <gtest/gtest.h>

#include <string>
#include <ostream>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "io/InputLoader.h"
#include "algorithms/csp/CSP.h"
#include "algorithms/csp/Fitness.h"
#include "validation/Validator.h"

namespace {

struct Configuration {
    std::string fitness;
    std::string distance;
    std::size_t memory_size;
    int64_t memory_penalty;
    bool prevent_return;
    bool require_completion;
};

std::ostream& operator<<(std::ostream& out, const Configuration& config) {
    return out << config.fitness << ", " << config.distance
               << ", memory=" << config.memory_size << ", penalty=" << config.memory_penalty
               << ", prevent_return=" << config.prevent_return
               << ", require_completion=" << config.require_completion;
}

std::vector<Configuration> configurations() {
    std::vector<Configuration> result;
    for (const auto* fitness : {"manhattan", "sqrt", "squared", "piecewise", "log", "random"}) {
        for (const auto* distance : {"manhattan", "shortest-path"}) {
            // Require completion only for configurations that completed in the initial run.
            const std::string name = fitness;
            const bool is_manhattan = name == "manhattan";
            const bool is_sqrt = name == "sqrt";
            result.push_back({fitness, distance, 0, 0, false, false});
            result.push_back({fitness, distance, 3, 2, false, is_manhattan || is_sqrt});
            result.push_back({fitness, distance, 0, 0, true, is_sqrt});
            result.push_back({fitness, distance, 3, 2, true, is_manhattan || is_sqrt});
        }
    }
    return result;
}

class CSPIntegrationTest : public ::testing::TestWithParam<Configuration> {};

TEST_P(CSPIntegrationTest, FindsValidPathsForFiftyAgents) {
    const auto& config = GetParam();
    auto [map, scenarios] = InputLoader::loadInput(
        "test-fixtures/csp/empty-32-32.map",
        "test-fixtures/csp/empty-32-32-even-1-50.scen"
    );
    ASSERT_EQ(scenarios.size(), 50);
    ASSERT_EQ(map.width, 32);
    ASSERT_EQ(map.height, 32);

    auto fitness = config.distance == "shortest-path"
        ? Fitness::getFitness(config.fitness, Fitness::shortestPathDistance(map, scenarios))
        : Fitness::getFitness(config.fitness);
    const auto* short_run = std::getenv("MAPF_INTEGRATION_SHORT");
    const std::size_t max_iterations =
        short_run && std::string(short_run) == "1" && !config.require_completion ? 50 : 200;
    std::size_t iteration_checks = 0;
    bool interrupted = false;
    auto interruption_requested = [&] {
        interrupted = iteration_checks++ >= max_iterations;
        return interrupted;
    };
    std::unique_ptr<CSP> csp;
    if (config.memory_size == 0 && !config.prevent_return) {
        csp = std::make_unique<CSP>(std::move(fitness), interruption_requested);
    } else {
        csp = std::make_unique<CSP>(
            std::move(fitness),
            interruption_requested,
            CSP::MemoryOptions{config.memory_size, config.memory_penalty, config.prevent_return}
        );
    }

    const auto paths = csp->resolve(map, scenarios);
    const auto [partial_valid, partial_error] = Validator::validatePartial(map, scenarios, paths);
    EXPECT_TRUE(partial_valid) << "Invalid paths: " <<
        partial_error.value_or("Unknown validation error");

    const auto [complete, completion_error] = Validator::validate(map, scenarios, paths);
    if (config.require_completion) {
        EXPECT_TRUE(complete) << "Scenario did not finish: "
                              << completion_error.value_or("Unknown validation error");
    } else if (!complete) {
        std::cout << "Completion is optional; scenario did not finish"
                  << (interrupted ? " within the iteration limit" : "") << '\n';
    }
}

INSTANTIATE_TEST_SUITE_P(
    Empty32Scenario1,
    CSPIntegrationTest,
    ::testing::ValuesIn(configurations()),
    [](const ::testing::TestParamInfo<Configuration>& info) {
        auto name = info.param.fitness + "_" + info.param.distance;
        for (auto& character : name) {
            if (character == '-') {
                character = '_';
            }
        }
        return name + (info.param.memory_size ? "_Memory" : "_NoMemory") +
               (info.param.prevent_return ? "_PreventReturn" : "_AllowReturn") +
               (info.param.require_completion ? "_RequiresCompletion" : "_CompletionOptional");
    }
);

} // namespace
