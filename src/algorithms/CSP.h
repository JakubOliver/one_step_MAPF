//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include <cstdint>
#include <functional>
#include <generator>
#include <string>

#include "Fitness.h"
#include "../Agent.h"
#include "../MemoryAgent.h"
#include "Algorithm.h"

class CSPTest;

class CSP : public Algorithm {
private:
    friend class CSPTest;

public:
    using FitnessFunction = Fitness::FitnessFunction;

    static FitnessFunction getFitness(const std::string& name) {
        return Fitness::getFitness(name);
    }

    struct MemoryOptions {
        std::size_t capacity;
        int64_t penalty;
        bool prevent_immediate_return = false;
    };

    CSP();
    explicit CSP(std::function<bool()> interruption_requested);
    explicit CSP(
        FitnessFunction fitness,
        std::function<bool()> interruption_requested
    );
    CSP(
        FitnessFunction fitness,
        std::function<bool()> interruption_requested,
        MemoryOptions memory
    );

    // Experimental one-step enumeration. The CSP and map must outlive the generator.
    // Groups supplied to enumerateGroups must be independent, as produced by Grouping.
    template<typename AgentType>
    std::generator<std::vector<AgentType>> enumerateGroup(
        std::vector<AgentType> agents,
        const Map& map
    );

    template<typename AgentType>
    std::generator<std::vector<std::vector<AgentType>>> enumerateGroups(
        std::vector<std::vector<AgentType>> groups,
        const Map& map
    );

private:

    std::function<bool()> interruption_requested;
    bool group_solve_interrupted = false;
    FitnessFunction fitness;
    MemoryOptions memory{0, 0};

    std::vector<AgentPath> resolve_(
        const Map& map,
        const std::vector<Scenario>& scenarios
    ) override;

    std::vector<Agent> processGroup(
        const std::vector<Agent>& agents,
        const Map& map
    );

    template<typename AgentType, bool PreventImmediateReturn = false>
    std::vector<AgentType> processGroupTyped(
        const std::vector<AgentType>& agents,
        const Map& map
    );

    template<typename AgentType, bool PreventImmediateReturn = false>
    std::vector<AgentPath> resolveTyped(
        const Map& map,
        const std::vector<Scenario>& scenarios
    );

};

#endif //MAPF_CSP_H
