#ifndef MAPF_ON_DEMAND_DISTANCE_H
#define MAPF_ON_DEMAND_DISTANCE_H

#include "model/Map.h"
#include "model/Scenario.h"

#include <cstdint>
#include <map>
#include <queue>
#include <string>
#include <vector>

class OnDemandDistance {
public:
    OnDemandDistance(const Map& map, const std::vector<Scenario>& scenarios, const std::string& method);
    int64_t distance(AgentCord position, AgentCord goal);

private:
    friend struct OnDemandDistanceTestAccess;

    struct Field {
        std::vector<int64_t> distances;
        std::queue<AgentCord> pending;
    };

    const Map& map;
    std::string method;
    std::map<AgentCord, Field> fields;

    void lazyDistance(AgentCord position, Field& field);
    int64_t search(AgentCord position, AgentCord goal, bool jump_points) const;
};

#endif // MAPF_ON_DEMAND_DISTANCE_H
