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
    static constexpr std::array<std::pair<int, int>, 4> canMoves = {{
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    }};

    static constexpr std::array<std::pair<int, int>, 4> diagMoves = {{
        {1, 1},
        {-1, -1},
        {-1, 1},
        {1, -1}
    }};
public:
    std::pair<std::size_t, std::size_t> position;
    std::pair<std::size_t, std::size_t> goal;

    GroupAgent(std::size_t x, std::size_t y) : GroupAgent(x, y, 0, 0) {}
    GroupAgent(std::size_t x, std::size_t y, std::size_t gx, std::size_t gy) : GroupAgent(std::make_pair(x, y), std::make_pair(gx, gy)) {}
    GroupAgent(std::pair<std::size_t, std::size_t> pos, std::pair<std::size_t, std::size_t> g) : position(pos), goal(g) {}

    std::generator<std::pair<std::size_t, std::size_t>> getNeighbors() const;
    std::generator<std::pair<std::size_t, std::size_t>> getMoveTo() const;
};

class Grouping {
public:
    //TODO: prepsat na iterator
    std::vector<std::vector<GroupAgent>> getGroups(const std::map<std::pair<std::size_t, std::size_t>, GroupAgent>& agents);
};


#endif //MAPF_GROUPING_H
