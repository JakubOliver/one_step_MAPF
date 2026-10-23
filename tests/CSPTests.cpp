#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "algorithms/CSP.h"
#include "algorithms/Grouping.h"

class CSPTest : public ::testing::Test {
protected:
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
