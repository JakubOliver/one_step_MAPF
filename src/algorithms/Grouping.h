//
// Created by jakub on 10/11/26.
//

#ifndef MAPF_GROUPING_H
#define MAPF_GROUPING_H

#include <cstddef>
#include <vector>
#include <generator>
#include <array>
#include <map>

struct GroupAgent {
private:
    static constexpr std::array<std::pair<std::size_t, std::size_t>, 4> canMoves = {{
    {1,0},
    {-1,0},
    {0,1},
    {0,-1}
    }};

    static constexpr std::array<std::pair<std::size_t, std::size_t>, 4> diagMoves = {{
        {1,1},
        {-1,-1},
        {-1,1},
        {1,-1}
    }};
public:
    std::pair<std::size_t, std::size_t> position;

    GroupAgent(std::size_t x, std::size_t y) : position(std::make_pair(x, y)) {}
    std::generator<std::pair<std::size_t, std::size_t>> getNeighbors() const;
};

class Grouping {
public:
    //TODO: prepsat na iterator
    std::vector<std::vector<GroupAgent>> getGroups(const std::map<std::pair<std::size_t, std::size_t>, GroupAgent>& agents);
};


#endif //MAPF_GROUPING_H
