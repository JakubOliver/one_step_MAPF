//
// Created by jakub on 10/5/26.
//

#ifndef MAPF_MAP_H
#define MAPF_MAP_H

#include <map>
#include <iostream>
#include <string>

//TODO: add precomputation for all nodes

class Map {
public:
    const std::string type;
    const std::size_t height;
    const std::size_t width;
    const std::map<std::pair<std::size_t, std::size_t>, bool> map; // TODO: change to set

    bool isBlocked(std::size_t x, std::size_t y) const;
    bool isBlocked(std::pair<std::size_t, std::size_t> pos) const;

    void print(std::ostream& os) const {
        os << "Map: " << type << " " << height << " " << width;
    }

    inline friend std::ostream& operator<<(std::ostream& os, const Map& map) {
        map.print(os); return os;
    }
};


#endif //MAPF_MAP_H
