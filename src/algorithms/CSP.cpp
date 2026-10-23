//
// Created by jakub on 10/12/26.
//

#include "CSP.h"

#include "Grouping.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model_solver.h"

using namespace operations_research;
using namespace operations_research::sat;

int64_t from_pos_to_index(
    AgentCord position,
    std::size_t map_width
) {
    return static_cast<int64_t>(
        position.first + position.second * map_width
    );
}

std::vector<Agent> CSP::processGroup(
    const std::vector<Agent>& agents,
    const Map& map
) {
    CpModelBuilder cp_model;

    std::vector<IntVar> agents_vars;
    agents_vars.reserve(agents.size());

    LinearExpr objective;

    for (const auto& agent : agents) {
        std::vector<int64_t> Q_pos;
        std::vector<int64_t> costs;

        auto add_candidate = [&](const auto& pos, std::size_t penalty = 0) {
            if (!map.isInBounds(pos) || map.isBlocked(pos)) {
                return;
            }

            Q_pos.push_back(
                from_pos_to_index(pos, map.width)
            );

            const int64_t dx =
                std::llabs(
                    static_cast<int64_t>(pos.first) -
                    static_cast<int64_t>(agent.goal.first)
                );

            const int64_t dy =
                std::llabs(
                    static_cast<int64_t>(pos.second) -
                    static_cast<int64_t>(agent.goal.second)
                );

            //costs.push_back(dx + dy);
            costs.push_back(2 *(dx + dy) + penalty);
        };

        add_candidate(agent.position, 1); // Staying in the same place is penalized to encourage movements towards the goal.

        for (const auto& pos : agent.getMoveTo()) {
            add_candidate(pos);
        }

        // q'_a in Q_pos^a
        IntVar next_position = cp_model.NewIntVar(
            Domain::FromValues(Q_pos)
        );

        agents_vars.push_back(next_position);

        // Candidate selection index
        IntVar choice = cp_model.NewIntVar(
            Domain(
                0,
                static_cast<int64_t>(Q_pos.size() - 1)
            )
        );

        // Connects candidates with position
        cp_model.AddElement(choice, Q_pos, next_position);

        // Fitness
        IntVar cost = cp_model.NewIntVar(
            Domain::FromValues(costs)
        );

        cp_model.AddElement(choice, costs, cost);

        objective += cost;
    }

    // Enforces No vertex collisions
    cp_model.AddAllDifferent(agents_vars);

    // Prevents two agents from swapping their current positions in one step.
    for (std::size_t first = 0; first < agents.size(); ++first) {
        for (std::size_t second = first + 1; second < agents.size(); ++second) {
            const std::vector<IntVar> pair_vars = {
                agents_vars[first],
                agents_vars[second]
            };
            auto forbidden_swap = cp_model.AddForbiddenAssignments(pair_vars);

            forbidden_swap.AddTuple({
                from_pos_to_index(agents[second].position, map.width),
                from_pos_to_index(agents[first].position, map.width)
            });
        }
    }

    // Minimize total fitness.
    cp_model.Minimize(objective);

    const CpSolverResponse response = Solve(cp_model.Build());

    //This sholud not happnen, because if the input data are correct, then 
    // there is always a solution, that all agents stay, in ther current possition
    if (response.status() != CpSolverStatus::OPTIMAL &&
        response.status() != CpSolverStatus::FEASIBLE) {
        std::cout << "No solution found\n";
        return {};
    }

    std::vector<Agent> updated_agents = agents;

    for (std::size_t i = 0; i < agents_vars.size(); ++i) {
        const int64_t index =
            SolutionIntegerValue(response, agents_vars[i]);

        const int64_t x = index % map.width;
        const int64_t y = index / map.width;

        updated_agents[i].position = {
            static_cast<std::size_t>(x),
            static_cast<std::size_t>(y)
        };

        /*
        std::cout << "Agent " << i
                  << ": (" << x << ", " << y << ")\n";
        */
    }

    /*
    std::cout << "Total fitness: "
              << response.objective_value()
              << '\n';
    */

    return updated_agents;
}

std::vector<AgentPath>CSP::resolve_(
    const Map& map,
    const std::vector<Scenario>& scenarios
) {
    std::map<AgentCord, Agent> agents;
    std::map<AgentCord, std::size_t> agent_indices;
    std::vector<std::vector<AgentCord>> paths(scenarios.size());

    for (std::size_t i = 0; i < scenarios.size(); ++i) {
        const auto& scenario = scenarios[i];
        const AgentCord start{scenario.startX, scenario.startY};
        const AgentCord goal{scenario.goalX, scenario.goalY};

        const auto [agent_it, inserted] = agents.emplace(
            start,
            Agent(start, goal)
        );

        agent_indices.emplace(agent_it->first, i);
        paths[i].push_back(start);
    }

    Grouping grouping;
    std::set<std::vector<AgentCord>> visited_configurations;

    auto get_configuration = [&]() {
        std::vector<AgentCord> configuration(scenarios.size());

        for (const auto& [position, agent_index] : agent_indices) {
            configuration[agent_index] = position;
        }

        return configuration;
    };

    visited_configurations.insert(get_configuration());
    std::size_t iteration = 0;

    while (
        !std::all_of(
            agents.begin(),
            agents.end(),
            [](const auto& agent) {
                return agent.second.isAtGoal();
            }
        )
    ) {
        const auto groups = grouping.getGroups(agents);
        std::map<AgentCord, Agent> updated_agents;
        std::map<AgentCord, std::size_t> updated_agent_indices;

        for (const auto& group : groups) {
            const auto processed_group = processGroup(
                group,
                map
            );

            for (std::size_t i = 0; i < group.size(); ++i) {
                const auto agent_index = agent_indices.at(
                    group[i].position
                );
                const auto& processed_agent = processed_group[i];


                const auto [agent_it, inserted] = updated_agents.emplace(
                    processed_agent.position,
                    processed_agent
                );

                updated_agent_indices.emplace(
                    agent_it->first,
                    agent_index
                );
                paths[agent_index].push_back(processed_agent.position);
            }
        }

        agents = std::move(updated_agents);
        agent_indices = std::move(updated_agent_indices);

        iteration++;
        const auto agents_at_goal = std::count_if(
            agents.begin(),
            agents.end(),
            [](const auto& agent) {
                return agent.second.isAtGoal();
            }
        );

        std::cout << "CSP iteration " << iteration << ": "
                  << agents_at_goal << '/' << agents.size()
                  << " agents at goal\n";
    }

    std::vector<AgentPath> agent_paths;
    agent_paths.reserve(paths.size());

    for (auto& path : paths) {
        agent_paths.emplace_back(std::move(path));
    }

    return agent_paths;
}
