//
// Created by jakub on 10/6/26.
//

#include "Agent.h"

std::pair<std::size_t, std::size_t> Agent::getPathAtTime(std::size_t time) const {
    if (time >= path.size()) {
        return path.at(path.size() - 1);
    }

    return path.at(time);
}
