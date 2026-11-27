//
// Created by jakub on 10/5/26.
//

#include "model/Map.h"

bool Map::isInBounds(std::size_t x, std::size_t y) const {
    return isInBounds({x, y});
}

bool Map::isInBounds(AgentCord pos) const {
    return pos.first < width && pos.second < height;
}

bool Map::isBlocked(std::size_t x, std::size_t y) const {
    return isBlocked({x, y});
}

bool Map::isBlocked(AgentCord pos) const {
    return map.at(pos);
}

// The boundary of the map acts as a wall.
bool Map::isInCorridor(AgentCord pos) const {
    if (!isInBounds(pos) || isBlocked(pos)) {
        return false;
    }
    const auto [x, y] = pos;
    const int walls = (y == 0 || isBlocked(x, y - 1))
        + (x == width - 1 || isBlocked(x + 1, y))
        + (y == height - 1 || isBlocked(x, y + 1))
        + (x == 0 || isBlocked(x - 1, y));
    return walls >= 2;
}
