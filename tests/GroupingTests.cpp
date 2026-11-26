#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "model/Agent.h"
#include "algorithms/csp/Grouping.h"

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
    const std::vector<Group<Agent>>& groups
) {
    PositionGroups positions;
    positions.reserve(groups.size());

    for (const auto& group : groups) {
        PositionGroup groupPositions;
        groupPositions.reserve(group.agents.size());

        for (const auto& agent : group.agents) {
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

TEST(GroupingTest, MergesGroupsConnectedByLaterAgent) {
    // Gives frist and second cord different groups, but third connects them
    const std::vector<Position> positions = {
        {0, 2},
        {2, 0},
        {2, 2}
    };
    const auto agents = makeAgents(positions);

    const auto groups = Grouping().getGroups(agents);

    const auto expected = canonicalizeExpected({positions});
    EXPECT_EQ(canonicalizeGroups(groups), expected);
}

TEST(GroupingTest, SeparatesAgentsAcrossBlockedMiddleCell) {
    const Map map{.type = "test", .height = 1, .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, true}, {{2, 0}, false}}};
    const auto agents = makeAgents({{0, 0}, {2, 0}});

    EXPECT_EQ(canonicalizeGroups(Grouping().getGroups(agents, map)),
              canonicalizeExpected({{{0, 0}}, {{2, 0}}}));
}

TEST(GroupingTest, DiagonalAgentsNeedOnlyOneSharedOpenCell) {
    const auto agents = makeAgents({{0, 0}, {1, 1}});
    const Map one_open{.type = "test", .height = 2, .width = 2,
        .map = {{{0, 0}, false}, {{1, 0}, true}, {{0, 1}, false}, {{1, 1}, false}}};
    const Map blocked{.type = "test", .height = 2, .width = 2,
        .map = {{{0, 0}, false}, {{1, 0}, true}, {{0, 1}, true}, {{1, 1}, false}}};

    EXPECT_EQ(Grouping().getGroups(agents, one_open).size(), 1);
    EXPECT_EQ(Grouping().getGroups(agents, blocked).size(), 2);
}

TEST(GroupingTest, KeepsIndirectConnectionsAcrossOpenCells) {
    const Map map{.type = "test", .height = 1, .width = 5,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{2, 0}, false},
                {{3, 0}, false}, {{4, 0}, false}}};
    const auto agents = makeAgents({{0, 0}, {2, 0}, {4, 0}});

    EXPECT_EQ(Grouping().getGroups(agents, map).size(), 1);
}

TEST(GroupingTest, ChecksVerticalMiddleCellAndKeepsAdjacentAgentsTogether) {
    const Map map{.type = "test", .height = 4, .width = 1,
        .map = {{{0, 0}, false}, {{0, 1}, true}, {{0, 2}, false}, {{0, 3}, false}}};
    const auto agents = makeAgents({{0, 0}, {0, 2}, {0, 3}});

    EXPECT_EQ(canonicalizeGroups(Grouping().getGroups(agents, map)),
              canonicalizeExpected({{{0, 0}}, {{0, 2}, {0, 3}}}));
}

TEST(GroupingTest, DiagonalAgentsCanMeetAtEitherIntermediateCell) {
    const auto agents = makeAgents({{0, 0}, {1, 1}});
    const Map map{.type = "test", .height = 2, .width = 2,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{0, 1}, true}, {{1, 1}, false}}};

    EXPECT_EQ(Grouping().getGroups(agents, map).size(), 1);
}

TEST(GroupingTest, RecordsEveryEdgeInCycleOnce) {
    const Map map{.type = "test", .height = 2, .width = 2,
        .map = {{{0, 0}, false}, {{1, 0}, false}, {{0, 1}, false}, {{1, 1}, false}}};
    const auto agents = makeAgents({{0, 0}, {1, 0}, {0, 1}, {1, 1}});
    const auto groups = Grouping().getGroups(agents, map);

    ASSERT_EQ(groups.size(), 1);
    const auto& group = groups.front();
    ASSERT_EQ(group.agents.size(), 4);
    EXPECT_EQ(group.edges.size(), 6);
    std::set<std::pair<Position, Position>> edges;
    for (const auto& [first, second] : group.edges) {
        ASSERT_LT(first, group.agents.size());
        ASSERT_LT(second, group.agents.size());
        EXPECT_LT(first, second);
        auto positions = std::minmax(group.agents[first].position, group.agents[second].position);
        EXPECT_TRUE(edges.emplace(positions.first, positions.second).second);
    }
    EXPECT_EQ(edges.size(), 6);
}

TEST(GroupingTest, OmitsBlockedEdgeWithinConnectedGroup) {
    const Map map{.type = "test", .height = 2, .width = 3,
        .map = {{{0, 0}, false}, {{1, 0}, true}, {{2, 0}, false},
                {{0, 1}, false}, {{1, 1}, false}, {{2, 1}, false}}};
    const auto agents = makeAgents({{0, 0}, {2, 0}, {1, 1}});
    const auto groups = Grouping().getGroups(agents, map);

    ASSERT_EQ(groups.size(), 1);
    EXPECT_EQ(groups.front().edges.size(), 2);
}

}
