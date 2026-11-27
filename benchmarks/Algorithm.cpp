#include "Benchmark.h"

#include "algorithms/csp/CSP.h"
#include "algorithms/csp/Fitness.h"
#include "validation/Validator.h"

#include <iostream>
#include <utility>

namespace benchmark {
namespace {

class NullBuffer : public std::streambuf {
protected:
    int_type overflow(int_type character) override {
        return traits_type::not_eof(character);
    }

    std::streamsize xsputn(const char*, std::streamsize count) override {
        return count;
    }
};

class SuppressOutput {
public:
    SuppressOutput() : original(std::cout.rdbuf(&buffer)) {
    }
    ~SuppressOutput() {
        std::cout.rdbuf(original);
    }

private:
    NullBuffer buffer;
    std::streambuf* original;
};

} // namespace

Result algorithm(
    const Map& map,
    const std::vector<Scenario>& scenarios,
    const std::string& method,
    std::chrono::seconds timeout
) {
    const auto start = Clock::now();
    auto fitness = method == "none"
        ? Fitness::getFitness("manhattan")
        : Fitness::getFitness("manhattan", Fitness::shortestPathDistance(map, scenarios, method));
    const auto deadline = start + timeout;
    bool interrupted = false;
    CSP solver(
        std::move(fitness),
        [&] {
            interrupted = Clock::now() >= deadline;
            return interrupted;
        },
        {5, 1, false}
    );

    std::vector<AgentPath> paths;
    {
        SuppressOutput suppress_output;
        paths = solver.resolve(map, scenarios);
    }
    const auto elapsed = Clock::now() - start;
    if (interrupted) {
        return {elapsed, "timeout"};
    }
    if (!Validator::validate(map, scenarios, paths).first) {
        return {elapsed, "invalid"};
    }
    return {elapsed, "completed"};
}

} // namespace benchmark
