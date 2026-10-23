#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

#include "algorithms/Grouping.h"

namespace {

using Position = AgentCord;
using PositionGroup = std::vector<Position>;
using PositionGroups = std::vector<PositionGroup>;

std::map<Position, Agent> makeAgents(
    const std::vector<Position>& positions
) {
    std::map<Position, Agent> agents;

    for (const auto& position : positions) {
        agents.emplace(
            position,
            Agent(position, Position{0, 0})
        );
    }

    return agents;
}

PositionGroups canonicalizeGroups(
    const std::vector<std::vector<Agent>>& groups
) {
    PositionGroups positions;
    positions.reserve(groups.size());

    for (const auto& group : groups) {
        PositionGroup groupPositions;
        groupPositions.reserve(group.size());

        for (const auto& agent : group) {
            groupPositions.push_back(agent.position);
        }

        std::sort(groupPositions.begin(), groupPositions.end());
        positions.push_back(std::move(groupPositions));
    }

    std::sort(positions.begin(), positions.end());
    return positions;
}

PositionGroups canonicalizeExpected(PositionGroups groups) {
    for (auto& group : groups) {
        std::sort(group.begin(), group.end());
    }

    std::sort(groups.begin(), groups.end());
    return groups;
}

TEST(GroupingTest, PlacesDistantAgentsInSeparateGroups) {
    const std::vector<Position> positions = {
        {0, 0},
        {0, 10},
        {10, 0},
        {10, 10}
    };
    const auto agents = makeAgents(positions);

    const auto groups = Grouping().getGroups(agents);

    const auto expected = canonicalizeExpected({
        {{0, 0}},
        {{0, 10}},
        {{10, 0}},
        {{10, 10}}
    });
    EXPECT_EQ(canonicalizeGroups(groups), expected);
}

TEST(GroupingTest, PlacesFifteenChainedAgentsInOneGroup) {
    std::vector<Position> positions;
    positions.reserve(15);

    for (std::size_t index = 0; index < 15; index++) {
        positions.emplace_back(index * 2, 0);
    }

    const auto agents = makeAgents(positions);
    const auto groups = Grouping().getGroups(agents);

    const auto expected = canonicalizeExpected({positions});
    EXPECT_EQ(canonicalizeGroups(groups), expected);
}

TEST(GroupingTest, SeparatesMixedConnectedAndIsolatedAgents) {
    const std::vector<Position> positions = {
        {0, 0},
        {2, 0},
        {4, 0},
        {20, 20},
        {21, 21},
        {40, 40}
    };
    const auto agents = makeAgents(positions);

    const auto groups = Grouping().getGroups(agents);

    const auto expected = canonicalizeExpected({
        {{0, 0}, {2, 0}, {4, 0}},
        {{20, 20}, {21, 21}},
        {{40, 40}}
    });
    EXPECT_EQ(canonicalizeGroups(groups), expected);
}

}
