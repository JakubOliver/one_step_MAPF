//
// Created by jakub on 10/5/26.
//

#ifndef MAPF_MAP_H
#define MAPF_MAP_H

#include "AgentCord.h"

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <utility>

//TODO: add precomputation for all nodes

class Map {
public:
    const std::string type;
    const std::size_t height;
    const std::size_t width;
    const std::map<AgentCord, bool> map; // TODO: change to set

    bool isInBounds(std::size_t x, std::size_t y) const;
    bool isInBounds(AgentCord pos) const;
    bool isBlocked(std::size_t x, std::size_t y) const;
    bool isBlocked(AgentCord pos) const;

    void print(std::ostream& os) const {
        os << "Map: " << type << " " << height << " " << width;
    }

    inline friend std::ostream& operator<<(std::ostream& os, const Map& map) {
        map.print(os); return os;
    }
};


#endif //MAPF_MAP_H
