//
// Created by jakub on 10/12/26.
//

#include "CSP.h"

#include "Grouping.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model_solver.h"

using namespace operations_research;
using namespace operations_research::sat;

int64_t from_pos_to_index(
    std::pair<std::size_t, std::size_t> position,
    std::size_t map_width
) {
    return static_cast<int64_t>(
        position.first + position.second * map_width
    );
}

void CSP::processGroup_mocked() {
    constexpr std::size_t map_width = 20;

    std::vector<GroupAgent> agents = {
        {{0, 0}, {10, 10}},
        {{1, 0}, {10, 11}}
    };

    CpModelBuilder cp_model;

    std::vector<IntVar> agents_vars;
    agents_vars.reserve(agents.size());

    LinearExpr objective;

    for (const auto& agent : agents) {
        std::vector<int64_t> Q_pos;
        std::vector<int64_t> costs;

        auto add_candidate = [&](const auto& pos) {
            Q_pos.push_back(
                from_pos_to_index(pos, map_width)
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

            costs.push_back(dx + dy);
        };

        add_candidate(agent.position);

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

    // Minimize total fitness.
    cp_model.Minimize(objective);

    const CpSolverResponse response = Solve(cp_model.Build());

    if (response.status() != CpSolverStatus::OPTIMAL &&
        response.status() != CpSolverStatus::FEASIBLE) {
        std::cout << "No solution found\n";
        return;
    }

    for (std::size_t i = 0; i < agents_vars.size(); ++i) {
        const int64_t index =
            SolutionIntegerValue(response, agents_vars[i]);

        const int64_t x = index % map_width;
        const int64_t y = index / map_width;

        std::cout << "Agent " << i
                  << ": (" << x << ", " << y << ")\n";
    }

    std::cout << "Total fitness: "
              << response.objective_value()
              << '\n';
}