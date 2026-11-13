#ifndef MAPF_MEMORY_AGENT_H
#define MAPF_MEMORY_AGENT_H

#include "Agent.h"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <optional>

class MemoryAgent : public Agent {
public:
    MemoryAgent(AgentCord position, AgentCord goal, std::size_t capacity)
        : Agent(position, goal), capacity(capacity) {}

    bool remembers(AgentCord position) const {
        return std::find(history.begin(), history.end(), position) != history.end();
    }

    bool isImmediateReturn(AgentCord next_position) const {
        return previous_position.has_value() && next_position == *previous_position;
    }

    void moveTo(AgentCord next_position) {
        previous_position = position;
        if (next_position != position) {
            if (capacity > 0) {
                if (history.size() == capacity) {
                    history.pop_front();
                }
                history.push_back(position);
            }
            position = next_position;
        }
    }

private:
    std::size_t capacity;
    std::deque<AgentCord> history;
    std::optional<AgentCord> previous_position;
};

#endif // MAPF_MEMORY_AGENT_H
