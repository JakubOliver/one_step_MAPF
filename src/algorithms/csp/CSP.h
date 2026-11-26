//
// Created by jakub on 10/12/26.
//

#ifndef MAPF_CSP_H
#define MAPF_CSP_H

#include <cstdint>
#include <functional>
#include <string>

#include "algorithms/csp/Fitness.h"
#include "algorithms/csp/Grouping.h"
#include "model/Agent.h"
#include "algorithms/csp/MemoryAgent.h"
#include "algorithms/Algorithm.h"

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

private:

    std::function<bool()> interruption_requested;
    bool group_solve_interrupted = false;
    FitnessFunction fitness;
    MemoryOptions memory{0, 0};

    std::vector<AgentPath> resolve_(
        const Map& map,
        const std::vector<Scenario>& scenarios
    ) override;

    template<typename AgentType, bool PreventImmediateReturn = false>
    std::vector<AgentType> processGroupTyped(
        const Group<AgentType>& group,
        const Map& map
    );

    template<typename AgentType, bool PreventImmediateReturn = false>
    std::vector<AgentPath> resolveTyped(
        const Map& map,
        const std::vector<Scenario>& scenarios
    );

};

#endif //MAPF_CSP_H
