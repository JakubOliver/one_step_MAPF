//
// Created by jakub on 10/5/26.
//

#include "Map.h"

bool Map::isBlocked(std::size_t x, std::size_t y) const {
    return Map::isBlocked({x, y});
}

bool Map::isBlocked(std::pair<std::size_t, std::size_t> pos) const {
    return map.at(pos);
}
