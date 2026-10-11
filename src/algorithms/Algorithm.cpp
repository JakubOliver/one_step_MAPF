//
// Created by jakub on 10/11/26.
//

#include "Algorithm.h"

#include <chrono>

std::vector<Agent> Algorithm::resolve(const Map &map, const std::vector<Scenario> &scenarios) {
    auto start = std::chrono::steady_clock::now();

    std::vector<Agent> agents = resolve_(map, scenarios);

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Time: " << duration.count() << " ms" << std::endl;

    return agents;
}
