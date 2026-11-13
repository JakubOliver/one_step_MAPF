#ifndef MAPF_MEMORY_AGENT_H
#define MAPF_MEMORY_AGENT_H

#include "Agent.h"

#include <algorithm>
#include <cstddef>
#include <deque>

class MemoryAgent : public Agent {
public:
    MemoryAgent(AgentCord position, AgentCord goal, std::size_t capacity)
        : Agent(position, goal), capacity(capacity) {}

    bool remembers(AgentCord position) const {
        return std::find(history.begin(), history.end(), position) != history.end();
    }

    void moveTo(AgentCord next_position) {
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
};

#endif // MAPF_MEMORY_AGENT_H
