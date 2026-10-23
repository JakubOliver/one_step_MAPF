//
// Created by jakub on 10/6/26.
//

#include "AgentPath.h"

#include <stdexcept>
#include <utility>

AgentPath::AgentPath(std::vector<AgentCord> path)
    : path(std::move(path)) {
    if (this->path.empty()) {
        throw std::invalid_argument("An agent path cannot be empty");
    }
}

const std::vector<AgentCord>& AgentPath::getPath() const {
    return path;
}

AgentCord AgentPath::getPathAtTime(std::size_t time) const {
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
