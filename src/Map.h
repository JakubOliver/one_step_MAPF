//
// Created by jakub on 10/5/26.
//

#ifndef MAPF_MAP_H
#define MAPF_MAP_H

#include <map>

class Map {
public:
    const int height;
    const int width;
    const std::map<std::pair<int, int>, bool> map;
};


#endif //MAPF_MAP_H
