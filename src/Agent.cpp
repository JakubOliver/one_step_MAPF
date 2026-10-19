//
// Created by jakub on 10/6/26.
//

#include "Agent.h"

#include <stdexcept>
#include <utility>

AgentPath::AgentPath(std::vector<std::pair<std::size_t, std::size_t>> path)
    : path(std::move(path)) {
    if (this->path.empty()) {
        throw std::invalid_argument("An agent path cannot be empty");
    }
}

const std::vector<std::pair<std::size_t, std::size_t>>& AgentPath::getPath() const {
    return path;
}

std::pair<std::size_t, std::size_t> AgentPath::getPathAtTime(std::size_t time) const {
    if (path.empty()) {
        throw std::out_of_range("An agent with an empty path has no position");
    }

    if (time >= path.size()) {
        return path.at(path.size() - 1);
    }

    return path.at(time);
}

std::size_t AgentPath::getLength() const {
    return path.size();
}
