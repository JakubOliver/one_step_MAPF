#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "algorithms/CSP.h"

class CSPTest : public ::testing::Test {
protected:
    static std::vector<Agent> processGroup(
        CSP& csp,
        const std::vector<Agent>& agents,
        const Map& map
    ) {
        return csp.processGroup(agents, map);
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
    std::cout << "Updated agents: ";

    for (const auto& agent : updatedAgents) {
        std::cout << agent.position.first << ", " << agent.position.second << "; ";
    }

    for (const auto& agent : updatedAgents) {
        if (agent.position == AgentCord(0, 0)) {
            EXPECT_EQ(agent.goal, AgentCord(1, 0));
        } else if (agent.position == AgentCord(1, 0)) {
            EXPECT_EQ(agent.goal, AgentCord(0, 0));
        } else {
            FAIL() << "Unexpected agent position: (" << agent.position.first
                   << ", " << agent.position.second << ")";
        }
    }
}
