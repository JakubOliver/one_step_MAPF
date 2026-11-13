#ifndef MAPF_DISTANCE_CACHE_H
#define MAPF_DISTANCE_CACHE_H

#include "../Map.h"
#include "../Scenario.h"

#include <cstdint>
#include <map>
#include <vector>

class DistanceCache {
public:
    DistanceCache(const Map& map, const std::vector<Scenario>& scenarios);

    int64_t distance(AgentCord position, AgentCord goal) const;

private:
    std::size_t width;
    std::size_t height;
    std::map<AgentCord, std::vector<int64_t>> distances;
};

#endif // MAPF_DISTANCE_CACHE_H
