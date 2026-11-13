#include <gtest/gtest.h>

#include "algorithms/DistanceCache.h"
#include "algorithms/Fitness.h"
#include "algorithms/CSP.h"

#include <stdexcept>

namespace {

Map detourMap() {
    return {
        .type = "test",
        .height = 2,
        .width = 3,
        .map = {
            {{0, 0}, false}, {{1, 0}, true}, {{2, 0}, false},
            {{0, 1}, false}, {{1, 1}, false}, {{2, 1}, false}
        }
    };
}

} // namespace

TEST(DistanceCacheTest, ComputesDistancesAroundObstaclesAndSupportsSharedGoals) {
    const auto map = detourMap();
    const std::vector<Scenario> scenarios = {
        {.startX = 0, .startY = 0, .goalX = 2, .goalY = 0},
        {.startX = 1, .startY = 1, .goalX = 2, .goalY = 0},
        {.startX = 2, .startY = 0, .goalX = 0, .goalY = 0}
    };
    const DistanceCache cache(map, scenarios);

    EXPECT_EQ(cache.distance({2, 0}, {2, 0}), 0);
    EXPECT_EQ(cache.distance({0, 0}, {2, 0}), 4);
    EXPECT_EQ(cache.distance({1, 1}, {2, 0}), 2);
    EXPECT_EQ(cache.distance({2, 0}, {0, 0}), 4);
    EXPECT_THROW(cache.distance({1, 0}, {2, 0}), std::invalid_argument);
    EXPECT_THROW(cache.distance({3, 0}, {2, 0}), std::invalid_argument);
}

TEST(DistanceCacheTest, RejectsInvalidGoalsAndUnreachableStarts) {
    const Map map{
        .type = "test", .height = 1, .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, true}, {{2, 0}, false}}
    };

    EXPECT_THROW((DistanceCache(map, {{.startX = 0, .startY = 0, .goalX = 2, .goalY = 0}})), std::invalid_argument);
    EXPECT_THROW((DistanceCache(map, {{.startX = 0, .startY = 0, .goalX = 1, .goalY = 0}})), std::invalid_argument);
    EXPECT_THROW((DistanceCache(map, {{.startX = 0, .startY = 0, .goalX = 3, .goalY = 0}})), std::invalid_argument);
}

TEST(DistanceCacheTest, ShortestPathFitnessGuidesAgentAroundObstacle) {
    const auto map = detourMap();
    const std::vector<Scenario> scenarios = {{.startX = 0, .startY = 0, .goalX = 2, .goalY = 0}};
    const auto distance = Fitness::shortestPathDistance(map, scenarios);
    const auto fitness = Fitness::getFitness("manhattan", distance);

    EXPECT_EQ(fitness(map, {}, Agent({0, 0}, {2, 0}), true), 9);
    EXPECT_EQ(fitness(map, {}, Agent({0, 1}, {2, 0}), false), 6);

    CSP csp(fitness, [] { return false; });
    const auto paths = csp.resolve(map, scenarios);
    ASSERT_EQ(paths.size(), 1);
    ASSERT_EQ(paths[0].getLength(), 5);
    EXPECT_EQ(paths[0].getPathAtTime(1), AgentCord(0, 1));
    EXPECT_EQ(paths[0].getPathAtTime(4), AgentCord(2, 0));
}

TEST(DistanceCacheTest, ShortestPathDistanceWorksWithAllFitnessFunctions) {
    const auto map = detourMap();
    const std::vector<Scenario> scenarios = {{.startX = 0, .startY = 0, .goalX = 2, .goalY = 0}};
    const auto distance = Fitness::shortestPathDistance(map, scenarios);
    const Agent agent({0, 0}, {2, 0});

    EXPECT_EQ(Fitness::getFitness("manhattan", distance)(map, {}, agent, false), 8);
    EXPECT_EQ(Fitness::getFitness("sqrt", distance)(map, {}, agent, false), 828);
    EXPECT_EQ(Fitness::getFitness("squared", distance)(map, {}, agent, false), 32);
    EXPECT_EQ(Fitness::getFitness("piecewise", distance)(map, {}, agent, false), 32);
    EXPECT_EQ(Fitness::getFitness("log", distance)(map, {}, agent, false), 1609);
    EXPECT_EQ(Fitness::getFitness("random", distance)(map, {}, agent, true), 0);
    EXPECT_THROW(Fitness::getFitness("unknown", distance), std::invalid_argument);

    int calls = 0;
    auto random = Fitness::getFitness("random", [&calls](const Agent&) -> uint64_t {
        ++calls;
        return 4;
    });
    random(map, {}, agent, false);
    EXPECT_EQ(calls, 1);
}
