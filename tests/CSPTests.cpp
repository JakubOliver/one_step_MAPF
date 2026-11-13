#include <gtest/gtest.h>

#include <iostream>
#include <limits>
#include <vector>

#include "Agent.h"
#include "algorithms/CSP.h"
#include "algorithms/Grouping.h"
#include "util/Arguments.h"
#include "util/RunAlgorithm.h"

class CSPTest : public ::testing::Test {
protected:
    static std::vector<MemoryAgent> processMemoryGroup(
        CSP& csp,
        const std::vector<MemoryAgent>& agents,
        const Map& map
    ) {
        return csp.processGroupTyped(agents, map);
    }

    static std::vector<MemoryAgent> processWithoutImmediateReturn(
        CSP& csp,
        const std::vector<MemoryAgent>& agents,
        const Map& map
    ) {
        return csp.processGroupTyped<MemoryAgent, true>(agents, map);
    }

    static std::vector<Agent> processGroup(
        CSP& csp,
        const std::vector<Agent>& agents,
        const Map& map
    ) {
        return csp.processGroup(agents, map);
    }

    static Agent getAgentById(const std::vector<Agent>& agents, AgentId id) {
        for (const auto& agent : agents) {
            if (agent.id == id) {
                return agent;
            }
        }
        throw std::runtime_error("Agent with the specified ID not found");
    }

    static void validateMovement(CSP& csp, const Map& map, std::vector<Agent>& agents, std::vector<std::vector<AgentCord>>& expectedPath){
        for (std::size_t i = 0; i < expectedPath.size(); ++i) {
            auto agentsAfterRound = processGroup(csp, agents, map);

            for (std::size_t angetId = 0; angetId < agents.size(); ++angetId) {
                auto agent = getAgentById(agentsAfterRound, angetId);

                EXPECT_EQ(agent.position, expectedPath[i][angetId]) << "agent " << angetId << " at iteration " << i;
                EXPECT_EQ(agent.id, angetId);
            }

            agents = std::move(agentsAfterRound);
        }
    }
};

TEST_F(CSPTest, MovesAgentsTowardGoalsWithoutVertexCollision) {
    const Map map{
        .type = "test",
        .height = 2,
        .width = 3,
        .map = {
            {{0, 0}, false},
            {{1, 0}, false},
            {{2, 0}, false},
            {{0, 1}, false},
            {{1, 1}, false},
            {{2, 1}, false}
        }
    };
    const std::vector<Agent> agents = {
        {{0, 0}, {2, 0}},
        {{1, 0}, {1, 1}}
    };
    CSP csp;

    const auto updatedAgents = processGroup(csp, agents, map);

    ASSERT_EQ(updatedAgents.size(), 2);
    EXPECT_EQ(updatedAgents[0].position, AgentCord(1, 0));
    EXPECT_EQ(updatedAgents[1].position, AgentCord(1, 1));
    EXPECT_NE(updatedAgents[0].position, updatedAgents[1].position);
}

TEST_F(CSPTest, RejectsSwapInTwoCellMap) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 2,
        .map = {
            {{0, 0}, false},
            {{1, 0}, false}
        }
    };
    const std::vector<Agent> agents = {
        {{0, 0}, {1, 0}},
        {{1, 0}, {0, 0}}
    };
    CSP csp;

    const auto updatedAgents = processGroup(csp, agents, map);

    for (const auto& agent : updatedAgents) {
        if (agent.position == AgentCord(0, 0)) {
            EXPECT_EQ(agent.goal, AgentCord(1, 0));
            EXPECT_EQ(agent.id, 0);
        } else if (agent.position == AgentCord(1, 0)) {
            EXPECT_EQ(agent.goal, AgentCord(0, 0));
            EXPECT_EQ(agent.id, 1);
        } else {
            FAIL() << "Unexpected agent position: (" << agent.position.first
                   << ", " << agent.position.second << ")";
        }
    }
}

TEST_F(CSPTest, StayAtSamePosition) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {
            {{0, 0}, false},
            {{1, 0}, true},
            {{2, 0}, false}
        }
    };
    const std::vector<Agent> agents = {
        {{0, 0}, {1, 0}},
        {{2, 0}, {1, 0}}
    };
    CSP csp;

    const auto updatedAgents = processGroup(csp, agents, map);

    for (const auto& agent : updatedAgents) {
        if (agent.position == AgentCord(0, 0)) {
            ASSERT_EQ(agent.goal, AgentCord(1, 0));
            ASSERT_EQ(agent.id, 0);
        } else if (agent.position == AgentCord(2, 0)) {
            ASSERT_EQ(agent.goal, AgentCord(1, 0));
            ASSERT_EQ(agent.id, 1);
        } else {
            FAIL() << "Unexpected agent position: (" << agent.position.first
                   << ", " << agent.position.second << ")";
        }
    }
}

TEST_F(CSPTest, AgentMovesTowardsTheGoal) {
    // A _ G
    const Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {
            {{0, 0}, false},
            {{1, 0}, false},
            {{2, 0}, false}
        }
    };
    std::vector<Agent> agents = {
        {{0, 0}, {2, 0}},
    };
    std::vector<std::vector<AgentCord>> expectedPath = {
        {{1, 0}},
        {{2, 0}},
        {{2, 0}}
    };
    CSP csp;

    CSPTest::validateMovement(csp, map, agents, expectedPath);
}

TEST_F(CSPTest, SingleAgentEvaluatesOnlyValidCandidatesAndUsesFitness) {
    const Map map{
        .type = "test",
        .height = 2,
        .width = 2,
        .map = {
            {{0, 0}, false},
            {{1, 0}, true},
            {{0, 1}, false},
            {{1, 1}, false}
        }
    };
    const std::vector<Agent> agents = {{{0, 0}, {1, 0}}};
    std::vector<std::pair<AgentCord, bool>> evaluated;
    CSP csp(
        [&evaluated](const Map&, const std::vector<Agent>& group,
                     const Agent& candidate, bool is_staying) -> int64_t {
            EXPECT_EQ(group.size(), 1);
            evaluated.emplace_back(candidate.position, is_staying);
            return is_staying ? 10 : -1;
        },
        [] { return false; }
    );

    const auto updated = processGroup(csp, agents, map);

    ASSERT_EQ(updated.size(), 1);
    EXPECT_EQ(updated[0].position, AgentCord(0, 1));
    EXPECT_EQ(updated[0].id, agents[0].id);
    ASSERT_EQ(evaluated.size(), 2);
    EXPECT_EQ(evaluated[0], std::make_pair(AgentCord(0, 0), true));
    EXPECT_EQ(evaluated[1], std::make_pair(AgentCord(0, 1), false));
}

TEST_F(CSPTest, SingleAgentCanPreferStaying) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 2,
        .map = {{{0, 0}, false}, {{1, 0}, false}}
    };
    const std::vector<Agent> agents = {{{0, 0}, {1, 0}}};
    CSP csp(
        [](const Map&, const std::vector<Agent>&, const Agent&, bool is_staying) {
            return is_staying ? int64_t{-5} : int64_t{0};
        },
        [] { return false; }
    );

    const auto updated = processGroup(csp, agents, map);

    ASSERT_EQ(updated.size(), 1);
    EXPECT_EQ(updated[0].position, AgentCord(0, 0));
}

TEST_F(CSPTest, AgentsMovesTowardsTheGoal) {
    // A0 _ G0 
    // A1 _ G1
    const Map map{
        .type = "test",
        .height = 2,
        .width = 3,
        .map = {
            {{0, 0}, false},
            {{1, 0}, false},
            {{2, 0}, false},
            {{0, 1}, false},
            {{1, 1}, false},
            {{2, 1}, false}
        }
    };
    std::vector<Agent> agents = {
        {{0, 0}, {2, 0}},
        {{0, 1}, {2, 1}}
    };
    std::vector<std::vector<AgentCord>> expectedPath = {
        {{1, 0}, {1, 1}},
        {{2, 0}, {2, 1}},
        {{2, 0}, {2, 1}}
    };
    CSP csp;

    CSPTest::validateMovement(csp, map, agents, expectedPath);
}

TEST_F(CSPTest, AgentsMovesTowardsTheGoal2) {
    // A0 # G1
    // A1 _ G0
    const Map map{
        .type = "test",
        .height = 2,
        .width = 3,
        .map = {
            {{0, 0}, false},
            {{1, 0}, true},
            {{2, 0}, false},
            {{0, 1}, false},
            {{1, 1}, false},
            {{2, 1}, false}
        }
    };
    std::vector<Agent> agents = {
        {{0, 0}, {2, 1}},
        {{0, 1}, {2, 0}}
    };
    std::vector<std::vector<AgentCord>> expectedPath = {
        {{0, 1}, {1, 1}},
        {{1, 1}, {2, 1}},
        {{2, 1}, {2, 0}},
        {{2, 1}, {2, 0}},
    };
    CSP csp;

    CSPTest::validateMovement(csp, map, agents, expectedPath);
}

TEST_F(CSPTest, AgentsMovesTowardsTheGoal3) {
    //       X
    //     0  1    2   3
    //   0 #  G2   #   _
    //   1 A2 G1   #   _
    // Y 2 #  A0 A1/G0 _
    //   3 #  _    #   _
    const Map map{
        .type = "test",
        .height = 4,
        .width = 4,
        .map = {
            {{0, 0}, true},
            {{1, 0}, false},
            {{2, 0}, true},
            {{3, 0}, false},
            {{0, 1}, false},
            {{1, 1}, false},
            {{2, 1}, true},
            {{3, 1}, false},
            {{0, 2}, true},
            {{1, 2}, false},
            {{2, 2}, false},
            {{3, 2}, false},
            {{0, 3}, true},
            {{1, 3}, false},
            {{2, 3}, true},
            {{3, 3}, false}
        }
    };
    std::vector<Agent> agents = {
        {{1, 2}, {2, 2}},
        {{2, 2}, {1, 1}},
        {{0, 1}, {1, 0}}
    };
    std::vector<std::vector<AgentCord>> expectedPath = {
        {{1, 3}, {1, 2}, {1, 1}},
        {{1, 2}, {1, 1}, {1, 0}},
        {{2, 2}, {1, 1}, {1, 0}},
        {{2, 2}, {1, 1}, {1, 0}},
    };
    CSP csp;

    CSPTest::validateMovement(csp, map, agents, expectedPath);
}

TEST_F(CSPTest, ReturnsPartialPathsWhenInterruptedBetweenIterations) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {
            {{0, 0}, false},
            {{1, 0}, false},
            {{2, 0}, false}
        }
    };
    const std::vector<Scenario> scenarios = {
        {
            .startX = 0,
            .startY = 0,
            .goalX = 2,
            .goalY = 0
        }
    };
    std::size_t interruption_checks = 0;
    CSP csp(
        [&interruption_checks]() {
            return interruption_checks++ > 0;
        }
    );

    const auto paths = csp.resolve(map, scenarios);

    ASSERT_EQ(paths.size(), 1);
    ASSERT_EQ(paths[0].getLength(), 2);
    EXPECT_EQ(paths[0].getPathAtTime(0), AgentCord(0, 0));
    EXPECT_EQ(paths[0].getPathAtTime(1), AgentCord(1, 0));
}

TEST(MemoryAgentTest, RemembersOnlyRecentDeparturesAndIgnoresWaiting) {
    MemoryAgent agent({0, 0}, {3, 0}, 2);
    agent.moveTo({1, 0});
    agent.moveTo({1, 0});
    agent.moveTo({2, 0});
    EXPECT_TRUE(agent.remembers({0, 0}));
    EXPECT_TRUE(agent.remembers({1, 0}));

    agent.moveTo({3, 0});
    EXPECT_FALSE(agent.remembers({0, 0}));
    EXPECT_TRUE(agent.remembers({1, 0}));
    EXPECT_TRUE(agent.remembers({2, 0}));
}

TEST(MemoryAgentTest, GroupingPreservesMemory) {
    MemoryAgent agent({0, 0}, {2, 0}, 2);
    agent.moveTo({1, 0});
    std::map<AgentCord, MemoryAgent> agents;
    agents.emplace(agent.position, agent);

    const auto groups = Grouping().getGroups(agents);
    ASSERT_EQ(groups.size(), 1);
    ASSERT_EQ(groups[0].size(), 1);
    EXPECT_TRUE(groups[0][0].remembers({0, 0}));
}

TEST_F(CSPTest, MemoryPenaltyChangesChoiceButDoesNotForbidReturn) {
    Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, false}}
    };
    MemoryAgent agent({0, 0}, {0, 0}, 2);
    agent.moveTo({1, 0});
    const auto fitness = [](const Map&, const std::vector<Agent>&, const Agent& candidate, bool staying) -> int64_t {
        return staying ? 10 : static_cast<int64_t>(candidate.position.first);
    };
    CSP csp(fitness, [] { return false; }, {2, 5});

    auto result = processMemoryGroup(csp, {agent}, map);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].position, AgentCord(2, 0));
    EXPECT_TRUE(result[0].remembers({1, 0}));

    const Map blocked_map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, true}}
    };
    result = processMemoryGroup(csp, {agent}, blocked_map);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].position, AgentCord(0, 0));
}

TEST_F(CSPTest, RejectsInvalidMemoryConfiguration) {
    EXPECT_THROW((CSP(CSP::getFitness("manhattan"), [] { return false; }, {0, 1})), std::invalid_argument);
    EXPECT_THROW((CSP(CSP::getFitness("manhattan"), [] { return false; }, {1, 0})), std::invalid_argument);
}

TEST_F(CSPTest, MemoryIsUsedAcrossCspIterations) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, false}}
    };
    const std::vector<Scenario> scenarios = {{
        .startX = 0,
        .startY = 0,
        .goalX = 2,
        .goalY = 0
    }};
    const auto fitness = [](const Map&, const std::vector<Agent>&, const Agent& candidate, bool staying) -> int64_t {
        return staying ? 10 : static_cast<int64_t>(candidate.position.first);
    };
    std::size_t checks = 0;
    CSP csp(fitness, [&checks] { return checks++ >= 3; }, {2, 5});

    const auto paths = csp.resolve(map, scenarios);

    ASSERT_EQ(paths.size(), 1);
    ASSERT_EQ(paths[0].getLength(), 3);
    EXPECT_EQ(paths[0].getPathAtTime(1), AgentCord(1, 0));
    EXPECT_EQ(paths[0].getPathAtTime(2), AgentCord(2, 0));
}

TEST(CSPFitnessTest, NonlinearFitnessCostsAndWaitPenalties) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 4,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, false}, {{3, 0}, false}}
    };
    const std::vector<Agent> agents = {{{0, 0}, {3, 0}}};
    const Agent at_goal({3, 0}, {3, 0});
    const auto squared = CSP::getFitness("squared");
    const auto logarithmic = CSP::getFitness("log");

    EXPECT_EQ(squared(map, agents, agents[0], false), 18);
    EXPECT_EQ(squared(map, agents, agents[0], true), 19);
    EXPECT_EQ(squared(map, agents, at_goal, false), 0);
    EXPECT_EQ(logarithmic(map, agents, agents[0], false), 1386);
    EXPECT_EQ(logarithmic(map, agents, agents[0], true), 1586);
    EXPECT_EQ(logarithmic(map, agents, at_goal, false), 0);
}

TEST(CSPFitnessTest, SquaredDistanceRejectsOverflow) {
    const Map map{.type = "test", .height = 1, .width = 1, .map = {}};
    const Agent agent({0, 0}, {static_cast<std::size_t>(3000000000ULL), 0});
    const auto fitness = CSP::getFitness("squared");

    EXPECT_THROW(fitness(map, {}, agent, false), std::overflow_error);
}

TEST_F(CSPTest, NonlinearFitnessWorksWithMemoryPenalty) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, false}}
    };
    MemoryAgent agent({0, 0}, {0, 0}, 1);
    agent.moveTo({1, 0});

    for (const auto* name : {"squared", "piecewise", "log"}) {
        CSP csp(CSP::getFitness(name), [] { return false; }, {1, 2000});
        const auto result = processMemoryGroup(csp, {agent}, map);
        ASSERT_EQ(result.size(), 1);
        EXPECT_EQ(result[0].position, AgentCord(1, 0)) << name;
    }
}

TEST(CSPFitnessTest, PiecewiseCostsAcrossThresholdAndWait) {
    const Map map{.type = "test", .height = 1, .width = 1, .map = {}};
    const auto fitness = CSP::getFitness("piecewise");
    const Agent at_goal({0, 0}, {0, 0});
    const Agent near({0, 0}, {3, 0});
    const Agent boundary({0, 0}, {4, 0});
    const Agent far({0, 0}, {5, 0});

    EXPECT_EQ(fitness(map, {}, at_goal, false), 0);
    EXPECT_EQ(fitness(map, {}, near, false), 18);
    EXPECT_EQ(fitness(map, {}, boundary, false), 32);
    EXPECT_EQ(fitness(map, {}, far, false), 48);
    EXPECT_EQ(fitness(map, {}, far, true), 49);
}

TEST(CSPFitnessTest, PiecewiseDistanceRejectsOverflow) {
    const Map map{.type = "test", .height = 1, .width = 1, .map = {}};
    const Agent agent({0, 0}, {std::numeric_limits<std::size_t>::max(), 0});
    const auto fitness = CSP::getFitness("piecewise");

    if (sizeof(std::size_t) >= sizeof(int64_t)) {
        EXPECT_THROW(fitness(map, {}, agent, false), std::overflow_error);
    }
}

TEST_F(CSPTest, ImmediateReturnIsForbiddenButAllowedAfterWaiting) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 2,
        .map = {{{0, 0}, false}, {{1, 0}, false}}
    };
    MemoryAgent agent({0, 0}, {0, 0}, 0);
    agent.moveTo({1, 0});
    CSP csp(CSP::getFitness("manhattan"), [] { return false; }, {0, 0, true});

    auto result = processWithoutImmediateReturn(csp, {agent}, map);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].position, AgentCord(1, 0));

    result = processWithoutImmediateReturn(csp, result, map);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].position, AgentCord(0, 0));
}

TEST_F(CSPTest, ImmediateReturnIsForbiddenWithMemoryPenalty) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, false}}
    };
    MemoryAgent agent({0, 0}, {0, 0}, 2);
    agent.moveTo({1, 0});
    const auto fitness = [](const Map&, const std::vector<Agent>&, const Agent& candidate, bool staying) -> int64_t {
        return staying ? 10 : static_cast<int64_t>(candidate.position.first);
    };
    CSP csp(fitness, [] { return false; }, {2, 1, true});

    const auto result = processWithoutImmediateReturn(csp, {agent}, map);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].position, AgentCord(2, 0));
}

TEST_F(CSPTest, ImmediateReturnIsPreventedAcrossIterations) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, false}}
    };
    const std::vector<Scenario> scenarios = {{
        .startX = 0, .startY = 0, .goalX = 2, .goalY = 0
    }};
    const auto fitness = [](const Map&, const std::vector<Agent>&, const Agent& candidate, bool staying) -> int64_t {
        return staying ? 10 : static_cast<int64_t>(candidate.position.first);
    };
    std::size_t checks = 0;
    CSP csp(fitness, [&checks] { return checks++ >= 3; }, {0, 0, true});

    const auto paths = csp.resolve(map, scenarios);
    ASSERT_EQ(paths.size(), 1);
    ASSERT_EQ(paths[0].getLength(), 3);
    EXPECT_EQ(paths[0].getPathAtTime(1), AgentCord(1, 0));
    EXPECT_EQ(paths[0].getPathAtTime(2), AgentCord(2, 0));
}

namespace {

std::optional<util::Arguments> parseTestArguments(
    std::vector<std::string> arguments,
    bool visualizer_available = true
) {
    std::vector<char*> argv;
    for (auto& argument : arguments) {
        argv.push_back(argument.data());
    }
    return util::parseArguments(
        static_cast<int>(argv.size()),
        argv.data(),
        visualizer_available
    );
}

} // namespace

TEST(ArgumentsTest, PreservesDefaultsAndExplicitOptions) {
    const auto defaults = parseTestArguments({"MAPF", "map", "scenario"});
    ASSERT_TRUE(defaults.has_value());
    EXPECT_EQ(defaults->algorithm, "PriorityPlanning");
    EXPECT_EQ(defaults->fitness, "manhattan");
    EXPECT_EQ(defaults->memory_size, 0);
    EXPECT_EQ(defaults->memory_penalty, 1);
    EXPECT_FALSE(defaults->prevent_immediate_return);
    EXPECT_FALSE(defaults->validate);

    const auto options = parseTestArguments({
        "MAPF", "map", "scenario", "--algorithm", "CSP", "--fitness", "log",
        "--memory-size", "5", "--memory-penalty", "3",
        "--prevent-immediate-return", "--output", "paths.csv", "--validate"
    });
    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->map_file, "map");
    EXPECT_EQ(options->scenario_file, "scenario");
    EXPECT_EQ(options->algorithm, "CSP");
    EXPECT_EQ(options->fitness, "log");
    EXPECT_EQ(options->memory_size, 5);
    EXPECT_EQ(options->memory_penalty, 3);
    EXPECT_TRUE(options->prevent_immediate_return);
    EXPECT_EQ(options->output_file, "paths.csv");
    EXPECT_TRUE(options->validate);
}

TEST(ArgumentsTest, RejectsMissingArgumentsAndUnavailableVisualizer) {
    testing::internal::CaptureStderr();
    const auto missing = parseTestArguments({"MAPF"});
    const auto unavailable = parseTestArguments(
        {"MAPF", "map", "scenario", "--visualize"},
        false
    );
    const auto errors = testing::internal::GetCapturedStderr();

    EXPECT_FALSE(missing.has_value());
    EXPECT_FALSE(unavailable.has_value());
    EXPECT_NE(errors.find("Visualization support is disabled"), std::string::npos);
}

TEST(AlgorithmRunnerTest, RunsCspWithOptionsAndRejectsInvalidConfiguration) {
    const Map map{
        .type = "test",
        .height = 1,
        .width = 2,
        .map = {{{0, 0}, false}, {{1, 0}, false}}
    };
    const std::vector<Scenario> scenarios = {{
        .startX = 0, .startY = 0, .goalX = 1, .goalY = 0
    }};
    auto arguments = parseTestArguments({
        "MAPF", "map", "scenario", "--algorithm", "CSP",
        "--memory-size", "2", "--prevent-immediate-return"
    });
    ASSERT_TRUE(arguments.has_value());
    const auto paths = util::runAlgorithm(*arguments, map, scenarios);
    ASSERT_EQ(paths.size(), 1);
    EXPECT_EQ(paths[0].getPathAtTime(1), AgentCord(1, 0));

    arguments->memory_size = -1;
    EXPECT_THROW(util::runAlgorithm(*arguments, map, scenarios), std::invalid_argument);
    arguments->algorithm = "unknown";
    EXPECT_THROW(util::runAlgorithm(*arguments, map, scenarios), std::invalid_argument);
}
