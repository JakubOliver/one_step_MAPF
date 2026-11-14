#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <stdexcept>
#include <vector>

#include "algorithms/CSP.h"

namespace {

Map line(std::size_t width) {
    std::map<AgentCord, bool> cells;
    for (std::size_t x = 0; x < width; ++x) {
        cells.emplace(AgentCord{x, 0}, false);
    }
    return {"test", 1, width, std::move(cells)};
}

using Configuration = std::vector<AgentCord>;

template<typename T>
Configuration positions(const std::vector<T>& agents) {
    Configuration result;
    for (const auto& agent : agents) {
        result.push_back(agent.position);
    }
    return result;
}

TEST(CSPEnumerationTest, SingleAgentIsOrderedAndComplete) {
    const auto map = line(3);
    CSP csp;
    std::vector<Configuration> results;
    for (const auto& result : csp.enumerateGroup(std::vector<Agent>{{{1, 0}, {2, 0}}}, map)) {
        results.push_back(positions(result));
    }
    EXPECT_EQ(results, (std::vector<Configuration>{{{2, 0}}, {{1, 0}}, {{0, 0}}}));
}

TEST(CSPEnumerationTest, EqualFitnessDoesNotLoseSolutionsAndFitnessIsSampledOnce) {
    const auto map = line(3);
    int calls = 0;
    CSP csp([&](const Map&, const std::vector<Agent>&, const Agent&, bool) {
        ++calls;
        return int64_t{0};
    }, [] { return false; });
    std::set<Configuration> results;
    for (const auto& result : csp.enumerateGroup(std::vector<Agent>{{{1, 0}, {2, 0}}}, map)) {
        EXPECT_TRUE(results.insert(positions(result)).second);
    }
    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(calls, 3);
}

TEST(CSPEnumerationTest, TwoAgentsEnumerateExactlyValidMovesInFitnessOrder) {
    const auto map = line(3);
    const std::vector<Agent> agents = {{{0, 0}, {2, 0}}, {{1, 0}, {2, 0}}};
    CSP csp;
    std::set<Configuration> results;
    std::vector<int64_t> costs;
    auto fitness = CSP::getFitness("manhattan");
    for (const auto& result : csp.enumerateGroup(agents, map)) {
        EXPECT_TRUE(results.insert(positions(result)).second);
        int64_t cost = 0;
        for (std::size_t i = 0; i < result.size(); ++i) {
            EXPECT_EQ(result[i].id, agents[i].id);
            cost += fitness(map, agents, result[i], result[i].position == agents[i].position);
        }
        costs.push_back(cost);
    }
    EXPECT_TRUE(std::is_sorted(costs.begin(), costs.end()));
    EXPECT_EQ(results, (std::set<Configuration>{
        {{0, 0}, {1, 0}}, {{0, 0}, {2, 0}}, {{1, 0}, {2, 0}}
    }));
}

TEST(CSPEnumerationTest, SwapAndCollisionAreExcluded) {
    const auto map = line(2);
    CSP csp;
    std::set<Configuration> results;
    for (const auto& result : csp.enumerateGroup(std::vector<Agent>{
        {{0, 0}, {1, 0}}, {{1, 0}, {0, 0}}
    }, map)) {
        results.insert(positions(result));
    }
    EXPECT_EQ(results, (std::set<Configuration>{{{0, 0}, {1, 0}}}));
}

TEST(CSPEnumerationTest, BlockedAndOutOfBoundsMovesAreExcluded) {
    const Map map{"test", 1, 3, {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, true}}};
    CSP csp;
    std::set<Configuration> results;
    for (const auto& result : csp.enumerateGroup(std::vector<Agent>{{{1, 0}, {2, 0}}}, map)) {
        results.insert(positions(result));
    }
    EXPECT_EQ(results, (std::set<Configuration>{{{0, 0}}, {{1, 0}}}));
}

TEST(CSPEnumerationTest, MemoryPenaltyAndIndependentAlternativeHistories) {
    const auto map = line(3);
    MemoryAgent agent({0, 0}, {0, 0}, 2);
    agent.moveTo({1, 0});
    CSP csp([](const Map&, const std::vector<Agent>&, const Agent& candidate, bool staying) {
        return staying ? int64_t{10} : static_cast<int64_t>(candidate.position.first);
    }, [] { return false; }, {2, 5});
    std::vector<AgentCord> order;
    for (const auto& result : csp.enumerateGroup(std::vector<MemoryAgent>{agent}, map)) {
        order.push_back(result[0].position);
        EXPECT_TRUE(result[0].remembers({0, 0}));
        EXPECT_EQ(result[0].remembers({1, 0}), result[0].position != AgentCord(1, 0));
    }
    EXPECT_EQ(order, (std::vector<AgentCord>{{2, 0}, {0, 0}, {1, 0}}));
    EXPECT_FALSE(agent.remembers({1, 0}));
}

TEST(CSPEnumerationTest, ImmediateReturnIsExcludedButWaitingIsAllowed) {
    const auto map = line(3);
    MemoryAgent agent({0, 0}, {0, 0}, 0);
    agent.moveTo({1, 0});
    CSP csp(CSP::getFitness("manhattan"), [] { return false; }, {0, 0, true});
    std::set<Configuration> results;
    for (const auto& result : csp.enumerateGroup(std::vector<MemoryAgent>{agent}, map)) {
        results.insert(positions(result));
    }
    EXPECT_EQ(results, (std::set<Configuration>{{{1, 0}}, {{2, 0}}}));
}

TEST(CSPEnumerationTest, GroupProductContainsEveryCombinationExactlyOnce) {
    const auto map = line(6);
    CSP csp;
    const std::vector<std::vector<Agent>> groups = {
        {{{0, 0}, {1, 0}}}, {{{5, 0}, {4, 0}}}
    };
    std::set<Configuration> results;
    for (const auto& result : csp.enumerateGroups(groups, map)) {
        ASSERT_EQ(result.size(), 2);
        EXPECT_TRUE(results.insert({result[0][0].position, result[1][0].position}).second);
    }
    EXPECT_EQ(results, (std::set<Configuration>{
        {{0, 0}, {4, 0}}, {{0, 0}, {5, 0}}, {{1, 0}, {4, 0}}, {{1, 0}, {5, 0}}
    }));
}

TEST(CSPEnumerationTest, EmptyInputsHaveOneEmptyCombination) {
    const auto map = line(1);
    CSP csp;
    int count = 0;
    for (const auto& result : csp.enumerateGroup(std::vector<Agent>{}, map)) {
        EXPECT_TRUE(result.empty());
        ++count;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (const auto& result : csp.enumerateGroups(std::vector<std::vector<Agent>>{}, map)) {
        EXPECT_TRUE(result.empty());
        ++count;
    }
    EXPECT_EQ(count, 1);
}

TEST(CSPEnumerationTest, InterruptionIsNotReportedAsExhaustion) {
    const auto map = line(2);
    int checks = 0;
    CSP csp([&] { return checks++ > 0; });
    auto generator = csp.enumerateGroup(std::vector<Agent>{{{0, 0}, {1, 0}}}, map);
    auto it = generator.begin();
    ASSERT_NE(it, generator.end());
    EXPECT_THROW(++it, std::runtime_error);
}

TEST(CSPEnumerationTest, ThreeGroupsReuseCandidateCosts) {
    const auto map = line(9);
    int calls = 0;
    CSP csp([&](const Map&, const std::vector<Agent>&, const Agent&, bool) {
        ++calls;
        return int64_t{0};
    }, [] { return false; });
    const std::vector<std::vector<Agent>> groups = {
        {{{0, 0}, {0, 0}}}, {{{4, 0}, {4, 0}}}, {{{8, 0}, {8, 0}}}
    };
    std::set<Configuration> results;
    for (const auto& result : csp.enumerateGroups(groups, map)) {
        EXPECT_TRUE(results.insert({
            result[0][0].position, result[1][0].position, result[2][0].position
        }).second);
    }
    EXPECT_EQ(results.size(), 12);
    EXPECT_EQ(calls, 7);
}

TEST(CSPEnumerationTest, InfeasibleGroupProducesNoCombinations) {
    const Map map{"test", 1, 1, {{{0, 0}, true}}};
    CSP csp;
    const std::vector<std::vector<Agent>> groups = {{{{{0, 0}, {0, 0}}}}};
    int count = 0;
    for (const auto& result : csp.enumerateGroups(groups, map)) {
        (void)result;
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST(CSPEnumerationTest, GroupEnumerationIsLazy) {
    const auto map = line(2);
    int calls = 0;
    CSP csp([&](const Map&, const std::vector<Agent>&, const Agent&, bool) {
        ++calls;
        return int64_t{0};
    }, [] { return false; });
    auto generator = csp.enumerateGroup(std::vector<Agent>{{{0, 0}, {1, 0}}}, map);
    EXPECT_EQ(calls, 0);
    auto it = generator.begin();
    EXPECT_NE(it, generator.end());
    EXPECT_EQ(calls, 2);
}

TEST(CSPEnumerationTest, MultiAgentMemoryAndImmediateReturnAreRespected) {
    const auto map = line(4);
    MemoryAgent first({0, 0}, {0, 0}, 2);
    first.moveTo({1, 0});
    MemoryAgent second({3, 0}, {3, 0}, 2);
    CSP csp(CSP::getFitness("manhattan"), [] { return false; }, {2, 4, true});
    std::set<Configuration> results;
    for (const auto& result : csp.enumerateGroup(std::vector<MemoryAgent>{first, second}, map)) {
        EXPECT_TRUE(results.insert(positions(result)).second);
        EXPECT_NE(result[0].position, AgentCord(0, 0));
        EXPECT_NE(result[0].position, result[1].position);
        EXPECT_TRUE(result[0].remembers({0, 0}));
    }
    EXPECT_EQ(results, (std::set<Configuration>{
        {{1, 0}, {2, 0}}, {{1, 0}, {3, 0}}, {{2, 0}, {3, 0}}
    }));
}

} // namespace
