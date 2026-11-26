#include <gtest/gtest.h>

#include "algorithms/DistanceCache.h"
#include "algorithms/OnDemandDistance.h"

#include <random>
#include <stdexcept>

struct OnDemandDistanceTestAccess {
    static int64_t cached(const OnDemandDistance& distance, AgentCord position, AgentCord goal) {
        return distance.fields.at(goal).distances.at(position.second * distance.map.width + position.first);
    }
};

namespace {

Map makeMap(std::size_t width, std::size_t height, const std::vector<bool>& blocked) {
    std::map<AgentCord, bool> cells;
    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            cells.emplace(AgentCord{x, y}, blocked[y * width + x]);
        }
    }
    return {.type = "test", .height = height, .width = width, .map = std::move(cells)};
}

void compareMap(const Map& map) {
    for (std::size_t y = 0; y < map.height; ++y) {
        for (std::size_t x = 0; x < map.width; ++x) {
            const AgentCord goal{x, y};
            if (map.isBlocked(goal)) {
                continue;
            }
            const std::vector<Scenario> scenarios = {{.startX = x, .startY = y, .goalX = x, .goalY = y}};
            const DistanceCache expected(map, scenarios);
            for (const auto* method : {"lazy-bfs", "lazy-astar", "astar", "lazy-jps"}) {
                OnDemandDistance actual(map, scenarios, method);
                for (std::size_t sy = 0; sy < map.height; ++sy) {
                    for (std::size_t sx = 0; sx < map.width; ++sx) {
                        const AgentCord start{sx, sy};
                        SCOPED_TRACE(::testing::Message() << method << " from " << sx << "," << sy
                                     << " to " << x << "," << y);
                        int64_t distance = -1;
                        try {
                            distance = expected.distance(start, goal);
                        } catch (const std::invalid_argument&) {
                        }
                        if (distance < 0) {
                            EXPECT_THROW(actual.distance(start, goal), std::invalid_argument);
                        } else {
                            EXPECT_EQ(actual.distance(start, goal), distance);
                            EXPECT_EQ(actual.distance(start, goal), distance);
                        }
                    }
                }
            }
        }
    }
}

} // namespace

TEST(OnDemandDistanceTest, MatchesBfsForAllThreeByThreeMaps) {
    for (unsigned mask = 0; mask < (1u << 9); ++mask) {
        std::vector<bool> blocked(9);
        for (unsigned i = 0; i < 9; ++i) {
            blocked[i] = (mask & (1u << i)) != 0;
        }
        SCOPED_TRACE(::testing::Message() << "obstacle mask " << mask);
        compareMap(makeMap(3, 3, blocked));
    }
}

TEST(OnDemandDistanceTest, MatchesBfsOnLargerRandomMaps) {
    std::mt19937 generator(12345);
    for (int trial = 0; trial < 30; ++trial) {
        std::vector<bool> blocked(36);
        for (std::size_t i = 0; i < blocked.size(); ++i) {
            blocked[i] = generator() % 4 == 0;
        }
        SCOPED_TRACE(::testing::Message() << "random map " << trial);
        compareMap(makeMap(6, 6, blocked));
    }
}

TEST(OnDemandDistanceTest, RejectsUnreachableStartsAndInvalidQueries) {
    const auto map = makeMap(3, 1, {false, true, false});
    for (const auto* method : {"lazy-bfs", "lazy-astar", "astar", "lazy-jps"}) {
        EXPECT_THROW((OnDemandDistance(map, {{.startX = 0, .goalX = 2}}, method)), std::invalid_argument);
        EXPECT_THROW((OnDemandDistance(map, {{.startX = 0, .goalX = 1}}, method)), std::invalid_argument);
        OnDemandDistance cache(map, {{.startX = 0, .goalX = 0}}, method);
        EXPECT_THROW(cache.distance({3, 0}, {0, 0}), std::invalid_argument);
        EXPECT_THROW(cache.distance({1, 0}, {0, 0}), std::invalid_argument);
        EXPECT_THROW(cache.distance({2, 0}, {0, 0}), std::invalid_argument);
    }
}

TEST(OnDemandDistanceTest, LazyMethodsOnlyCalculateDistancesWhenRequested) {
    const auto map = makeMap(4, 1, {false, false, false, false});
    const std::vector<Scenario> scenarios = {{.startX = 0, .goalX = 3}};
    for (const auto* method : {"lazy-bfs", "lazy-astar", "lazy-jps"}) {
        SCOPED_TRACE(method);
        OnDemandDistance cache(map, scenarios, method);
        EXPECT_EQ(OnDemandDistanceTestAccess::cached(cache, {0, 0}, {3, 0}), -1);
        EXPECT_EQ(OnDemandDistanceTestAccess::cached(cache, {1, 0}, {3, 0}), -1);
        EXPECT_EQ(cache.distance({0, 0}, {3, 0}), 3);
        EXPECT_EQ(OnDemandDistanceTestAccess::cached(cache, {0, 0}, {3, 0}), 3);
    }
}

TEST(OnDemandDistanceTest, EagerAStarCalculatesAllReachableDistancesAtInitialization) {
    const auto map = makeMap(5, 1, {false, false, false, true, false});
    OnDemandDistance cache(map, {{.startX = 0, .goalX = 2}}, "astar");
    EXPECT_EQ(OnDemandDistanceTestAccess::cached(cache, {0, 0}, {2, 0}), 2);
    EXPECT_EQ(OnDemandDistanceTestAccess::cached(cache, {1, 0}, {2, 0}), 1);
    EXPECT_EQ(OnDemandDistanceTestAccess::cached(cache, {4, 0}, {2, 0}), -1);
    EXPECT_THROW(cache.distance({4, 0}, {2, 0}), std::invalid_argument);
}
