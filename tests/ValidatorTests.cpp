#include <gtest/gtest.h>

#include <cstddef>
#include <initializer_list>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Agent.h"
#include "AgentPath.h"
#include "Map.h"
#include "Scenario.h"
#include "validation/Validator.h"

namespace {

Map makeMap(
    std::size_t width,
    std::size_t height,
    const std::vector<AgentCord>& blocked = {}
) {
    std::map<AgentCord, bool> cells;

    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            cells[{x, y}] = false;
        }
    }

    for (const auto& position : blocked) {
        cells[position] = true;
    }

    return {
        .type = "test",
        .height = height,
        .width = width,
        .map = std::move(cells)
    };
}

Scenario makeScenario(AgentCord start, AgentCord goal) {
    return {
        .bucket = 0,
        .map = "test",
        .width = 0,
        .height = 0,
        .startX = start.first,
        .startY = start.second,
        .goalX = goal.first,
        .goalY = goal.second,
        .optimalDistance = 0
    };
}

AgentPath makePath(std::initializer_list<AgentCord> positions) {
    return AgentPath(std::vector<AgentCord>(positions));
}

void expectValid(
    const std::pair<bool, std::optional<std::string>>& result
) {
    EXPECT_TRUE(result.first);
    EXPECT_FALSE(result.second.has_value());
}

void expectInvalid(
    const std::pair<bool, std::optional<std::string>>& result
) {
    EXPECT_FALSE(result.first);
    ASSERT_TRUE(result.second.has_value());
    EXPECT_FALSE(result.second->empty());
}

TEST(AgentTest, AcceptsWaitAndOrthogonalMoves) {
    EXPECT_TRUE(Agent::isValidMove({1, 1}, {1, 1}));
    EXPECT_TRUE(Agent::isValidMove({1, 1}, {2, 1}));
    EXPECT_TRUE(Agent::isValidMove({1, 1}, {0, 1}));
    EXPECT_TRUE(Agent::isValidMove({1, 1}, {1, 2}));
    EXPECT_TRUE(Agent::isValidMove({1, 1}, {1, 0}));
}

TEST(AgentTest, RejectsDiagonalMovesAndTeleportation) {
    EXPECT_FALSE(Agent::isValidMove({1, 1}, {2, 2}));
    EXPECT_FALSE(Agent::isValidMove({1, 1}, {3, 1}));
    EXPECT_FALSE(Agent::isValidMove({1, 1}, {1, 3}));
    EXPECT_FALSE(Agent::isValidMove(
        {0, 0},
        {
            std::numeric_limits<std::size_t>::max(),
            std::numeric_limits<std::size_t>::max()
        }
    ));
}

TEST(ValidatorTest, AcceptsValidPathsRegardlessOfScenarioOrder) {
    const auto map = makeMap(4, 2);
    const std::vector<Scenario> scenarios = {
        makeScenario({0, 0}, {2, 0}),
        makeScenario({0, 1}, {3, 1})
    };
    const std::vector<AgentPath> paths = {
        makePath({{0, 1}, {1, 1}, {1, 1}, {2, 1}, {3, 1}}),
        makePath({{0, 0}, {1, 0}, {2, 0}})
    };

    expectValid(Validator::validate(map, scenarios, paths));
}

TEST(ValidatorTest, RejectsMissingAndExtraPaths) {
    const auto map = makeMap(3, 1);
    const std::vector<Scenario> scenarios = {
        makeScenario({0, 0}, {1, 0})
    };

    expectInvalid(Validator::validate(map, scenarios, {}));
    expectInvalid(Validator::validate(
        map,
        scenarios,
        {
            makePath({{0, 0}, {1, 0}}),
            makePath({{2, 0}})
        }
    ));
}

TEST(ValidatorTest, RejectsEmptyAndMismatchedPaths) {
    const auto map = makeMap(3, 1);
    const std::vector<Scenario> scenarios = {
        makeScenario({0, 0}, {2, 0})
    };

    expectInvalid(Validator::validate(map, scenarios, {AgentPath()}));
    expectInvalid(Validator::validate(
        map,
        scenarios,
        {makePath({{0, 0}, {1, 0}})}
    ));
}

TEST(ValidatorTest, MatchesDuplicateScenariosOneToOne) {
    const auto map = makeMap(2, 2);
    const auto duplicate = makeScenario({0, 0}, {1, 0});
    const std::vector<Scenario> scenarios = {duplicate, duplicate};

    expectInvalid(Validator::validate(
        map,
        scenarios,
        {
            makePath({{0, 0}, {1, 0}}),
            makePath({{0, 1}, {1, 1}})
        }
    ));
}

TEST(ValidatorTest, RejectsVertexCollision) {
    const auto map = makeMap(3, 2);
    const std::vector<Scenario> scenarios = {
        makeScenario({0, 0}, {2, 0}),
        makeScenario({1, 1}, {1, 0})
    };
    const std::vector<AgentPath> paths = {
        makePath({{0, 0}, {1, 0}, {2, 0}}),
        makePath({{1, 1}, {1, 0}})
    };

    expectInvalid(Validator::validate(map, scenarios, paths));
}

TEST(ValidatorTest, RejectsCollisionAfterShorterPathEnds) {
    const auto map = makeMap(3, 2);
    const std::vector<Scenario> scenarios = {
        makeScenario({1, 0}, {1, 0}),
        makeScenario({0, 1}, {2, 0})
    };
    const std::vector<AgentPath> paths = {
        makePath({{1, 0}}),
        makePath({{0, 1}, {0, 0}, {1, 0}, {2, 0}})
    };

    expectInvalid(Validator::validate(map, scenarios, paths));
}

TEST(ValidatorTest, RejectsEdgeSwap) {
    const auto map = makeMap(2, 1);
    const std::vector<Scenario> scenarios = {
        makeScenario({0, 0}, {1, 0}),
        makeScenario({1, 0}, {0, 0})
    };
    const std::vector<AgentPath> paths = {
        makePath({{0, 0}, {1, 0}}),
        makePath({{1, 0}, {0, 0}})
    };

    expectInvalid(Validator::validate(map, scenarios, paths));
}

TEST(ValidatorTest, RejectsBlockedMissingAndOutOfBoundsPositions) {
    const auto map = makeMap(3, 1, {{1, 0}});
    const std::vector<Scenario> blockedScenario = {
        makeScenario({0, 0}, {2, 0})
    };
    expectInvalid(Validator::validate(
        map,
        blockedScenario,
        {makePath({{0, 0}, {1, 0}, {2, 0}})}
    ));

    const Map incompleteMap{
        .type = "test",
        .height = 1,
        .width = 2,
        .map = {{{0, 0}, false}}
    };
    const std::vector<Scenario> missingScenario = {
        makeScenario({0, 0}, {1, 0})
    };
    expectInvalid(Validator::validate(
        incompleteMap,
        missingScenario,
        {makePath({{0, 0}, {1, 0}})}
    ));

    const std::vector<Scenario> outsideScenario = {
        makeScenario({0, 0}, {3, 0})
    };
    expectInvalid(Validator::validate(
        makeMap(3, 1),
        outsideScenario,
        {makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}})}
    ));
}

TEST(ValidatorTest, RejectsDiagonalMovesAndTeleportation) {
    const auto map = makeMap(3, 3);

    expectInvalid(Validator::validate(
        map,
        {makeScenario({0, 0}, {1, 1})},
        {makePath({{0, 0}, {1, 1}})}
    ));
    expectInvalid(Validator::validate(
        map,
        {makeScenario({0, 0}, {2, 0})},
        {makePath({{0, 0}, {2, 0}})}
    ));
}

}
