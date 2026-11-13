//
// Created by jakub on 10/12/26.
//

#include "CSP.h"

#include "Grouping.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <type_traits>
#include <limits>
#include <vector>

#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model_solver.h"

using namespace operations_research;
using namespace operations_research::sat;

RandomFitness::RandomFitness()
    : RandomFitness(std::random_device{}()) {
}

RandomFitness::RandomFitness(std::uint32_t seed)
    : generator(seed),
      distribution(1.0, 1.0) {
}

/*
Randomize fitness works better (in completion) rather than determistic version, 
but there is problem how to have all agent stopping at the same time. Probably
some condition apply change only if agent is in surrouding would make sense. 
*/
int64_t RandomFitness::operator()(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    const int64_t dx = std::llabs(
        static_cast<int64_t>(agent.position.first) -
        static_cast<int64_t>(agent.goal.first)
    );
    const int64_t dy = std::llabs(
        static_cast<int64_t>(agent.position.second) -
        static_cast<int64_t>(agent.goal.second)
    );

    if (is_staying){
        return 0;
    }

    return dx + dy +
           std::llround(distribution(generator));

    // return dx + dy +
    //        std::llround(distribution(generator)) +
    //        (is_staying ? 1 : 0);
}

CSP::CSP()
    : CSP(
        manhattanDistanceFitness,
        []() {
            return false;
        }
    ) {
}

CSP::CSP(std::function<bool()> interruption_requested)
    : CSP(manhattanDistanceFitness, std::move(interruption_requested)) {
}

CSP::CSP(
    FitnessFunction fitness,
    std::function<bool()> interruption_requested
)
    : interruption_requested(std::move(interruption_requested)),
      fitness(std::move(fitness)) {
    if (!this->fitness) {
        throw std::invalid_argument(
            "CSP fitness function must be provided"
        );
    }

    if (!this->interruption_requested) {
        throw std::invalid_argument(
            "CSP interruption callback must be provided"
        );
    }
}

CSP::CSP(
    FitnessFunction fitness,
    std::function<bool()> interruption_requested,
    MemoryOptions memory
) : CSP(std::move(fitness), std::move(interruption_requested)) {
    if (memory.capacity == 0 || memory.penalty <= 0) {
        throw std::invalid_argument("CSP memory capacity and penalty must be positive");
    }
    this->memory = memory;
}

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
    return processGroupTyped(agents, map);
}

int64_t CSP::manhattanDistanceFitness(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    const int64_t dx = std::llabs(
        static_cast<int64_t>(agent.position.first) -
        static_cast<int64_t>(agent.goal.first)
    );
    const int64_t dy = std::llabs(
        static_cast<int64_t>(agent.position.second) -
        static_cast<int64_t>(agent.goal.second)
    );

    return 2 * (dx + dy) + (is_staying ? 1 : 0);
}

int64_t CSP::sqrtDistanceFitness(
    const Map&,
    const std::vector<Agent>&,
    const Agent& agent,
    bool is_staying
) {
    constexpr double offset = 4.0;
    constexpr int64_t scale = 1000;

    const int64_t dx = std::llabs(
        static_cast<int64_t>(agent.position.first) -
        static_cast<int64_t>(agent.goal.first)
    );
    const int64_t dy = std::llabs(
        static_cast<int64_t>(agent.position.second) -
        static_cast<int64_t>(agent.goal.second)
    );

    const auto cost = std::llround(
        scale * (
            std::sqrt(static_cast<double>(dx + dy) + offset) -
            std::sqrt(offset)
        )
    );

    return cost + (is_staying ? 200 : 0);
}

CSP::FitnessFunction CSP::getFitness(const std::string& name) {
    using FitnessFactory = std::function<FitnessFunction()>;

    static const std::vector<std::pair<std::string, FitnessFactory>> fitnesses = {
        {"manhattan", [] {
            return manhattanDistanceFitness;
        }},
        {"sqrt", [] {
            return sqrtDistanceFitness;
        }},
        {"random", [] {
            return FitnessFunction(RandomFitness());
        }}
    };

    for (const auto& [fitness_name, fitness_factory] : fitnesses) {
        if (fitness_name == name) {
            return fitness_factory();
        }
    }

    throw std::invalid_argument(
        "Unknown CSP fitness: " + name +
        ". Available fitnesses: manhattan, sqrt, random"
    );
}

template<typename AgentType>
std::vector<AgentType> CSP::processGroupTyped(
    const std::vector<AgentType>& agents,
    const Map& map
) {
    group_solve_interrupted = false;

    CpModelBuilder cp_model;

    std::vector<IntVar> agents_vars;
    agents_vars.reserve(agents.size());

    LinearExpr objective;

    // Existing fitness functions operate on Agent values; preserve their interface.
    std::vector<Agent> base_agents;
    if constexpr (!std::is_same_v<AgentType, Agent>) {
        base_agents.reserve(agents.size());
        for (const auto& agent : agents) {
            base_agents.push_back(agent);
        }
    }
    const auto& fitness_agents = [&]() -> const std::vector<Agent>& {
        if constexpr (std::is_same_v<AgentType, Agent>) {
            return agents;
        } else {
            return base_agents;
        }
    }();

    for (const auto& agent : agents) {
        std::vector<int64_t> Q_pos;
        std::vector<int64_t> costs;

        auto add_candidate = [&](const auto& pos, bool is_staying = false) {
            if (!map.isInBounds(pos) || map.isBlocked(pos)) {
                return;
            }

            Q_pos.push_back(
                from_pos_to_index(pos, map.width)
            );

            Agent candidate = agent;
            candidate.position = pos;
            auto cost = fitness(map, fitness_agents, candidate, is_staying);
            if constexpr (std::is_same_v<AgentType, MemoryAgent>) {
                if (!is_staying && agent.remembers(pos)) {
                    if (cost > std::numeric_limits<int64_t>::max() - memory.penalty) {
                        throw std::overflow_error("CSP memory penalty overflows fitness");
                    }
                    cost += memory.penalty;
                }
            }
            costs.push_back(cost);
        };

        add_candidate(agent.position, true); 
        //TODO: test whether only agent.position==agent.goal does not make more sense
        // because that whould make penalizating staying only at final position

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

    // NOTE: To generate one solution for successively worse fitness value,
    // solve repeatedly and add objective >= previous_objective + 1. 
    // Butt skips over solutions with same fitness, so TODO for me to check 
    // how close the fitness are and if it makeses sense to skipp etc.
    // For note better sollution sould be to desable found solution and 
    // run the solver again, but this introduces overhead etc. 

    SatParameters parameters;
    parameters.set_catch_sigint_signal(false);

    const CpSolverResponse response = SolveWithParameters(
        cp_model.Build(),
        parameters
    );

    // A valid group always has the solution in which all agents stay in place.
    // A non-optimal response therefore means that the search was interrupted.
    if (response.status() != CpSolverStatus::OPTIMAL) {
        group_solve_interrupted = true;

        if (response.status() != CpSolverStatus::FEASIBLE) {
            std::cerr << "CSP group solve stopped before finding a solution\n";
        }

        return {};
    }

    std::vector<AgentType> updated_agents = agents;

    for (std::size_t i = 0; i < agents_vars.size(); ++i) {
        const int64_t index =
            SolutionIntegerValue(response, agents_vars[i]);

        const int64_t x = index % map.width;
        const int64_t y = index / map.width;

        const AgentCord next_position{
            static_cast<std::size_t>(x),
            static_cast<std::size_t>(y)
        };
        if constexpr (std::is_same_v<AgentType, MemoryAgent>) {
            updated_agents[i].moveTo(next_position);
        } else {
            updated_agents[i].position = next_position;
        }

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
    if (memory.capacity > 0) {
        return resolveTyped<MemoryAgent>(map, scenarios);
    }
    return resolveTyped<Agent>(map, scenarios);
}

template<typename AgentType>
std::vector<AgentPath> CSP::resolveTyped(
    const Map& map,
    const std::vector<Scenario>& scenarios
) {
    std::map<AgentCord, AgentType> agents;
    std::map<AgentCord, std::size_t> agent_indices;
    std::vector<std::vector<AgentCord>> paths(scenarios.size());

    for (std::size_t i = 0; i < scenarios.size(); ++i) {
        const auto& scenario = scenarios[i];
        const AgentCord start{scenario.startX, scenario.startY};
        const AgentCord goal{scenario.goalX, scenario.goalY};

        auto make_agent = [&]() {
            if constexpr (std::is_same_v<AgentType, MemoryAgent>) {
                return AgentType(start, goal, memory.capacity);
            } else {
                return AgentType(start, goal);
            }
        };
        const auto [agent_it, inserted] = agents.emplace(start, make_agent());

        agent_indices.emplace(agent_it->first, i);
        paths[i].push_back(start);
    }

    Grouping grouping;
    std::set<std::vector<AgentCord>> visited_configurations; //TODO: implement memory, but glboal memory does not make sense (probably) it is too costly, so maybe just some fix size local memory 

    auto get_configuration = [&]() {
        std::vector<AgentCord> configuration(scenarios.size());

        for (const auto& [position, agent_index] : agent_indices) {
            configuration[agent_index] = position;
        }

        return configuration;
    };

    //visited_configurations.insert(get_configuration());
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
        if (interruption_requested()) {
            std::cerr << "CSP interrupted; returning paths computed so far\n";
            break;
        }

        const auto groups = grouping.getGroups(agents);
        std::map<AgentCord, AgentType> updated_agents;
        std::map<AgentCord, std::size_t> updated_agent_indices;
        bool iteration_interrupted = false;

        for (const auto& group : groups) {
            const auto processed_group = processGroupTyped(
                group,
                map
            );

            if (group_solve_interrupted) {
                iteration_interrupted = true;
                break;
            }

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
            }
        }

        if (iteration_interrupted) {
            std::cerr << "CSP interrupted; returning paths computed so far\n";
            break;
        }

        agents = std::move(updated_agents);
        agent_indices = std::move(updated_agent_indices);

        for (const auto& [position, agent_index] : agent_indices) {
            paths[agent_index].push_back(position);
        }

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
