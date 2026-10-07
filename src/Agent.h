//
// Created by jakub on 10/6/26.
//

#ifndef MAPF_AGENT_H
#define MAPF_AGENT_H

#include <cstddef>
#include <utility>
#include <vector>

class Agent {
private:
    std::vector<std::pair<std::size_t, std::size_t>> path;

public:
    Agent() = default;
    explicit Agent(std::vector<std::pair<std::size_t, std::size_t>> path);

    const std::vector<std::pair<std::size_t, std::size_t>>& getPath() const;
    std::pair<std::size_t, std::size_t> getPathAtTime(std::size_t time) const;
};


#endif //MAPF_AGENT_H
