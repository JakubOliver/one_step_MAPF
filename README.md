# one_step_MAPF

# How to run

```
cmake --preset release
cmake --build --preset release
ctest --preset release
```

VS Code uses the selected CMake preset automatically. On the first configure,
CMake downloads the declared third-party dependencies, so it can take several
minutes. The build is written to `out/build/release`.

The test suite uses GoogleTest and is run through CTest.

Integration tests are disabled by default. To include them locally, run:

```sh
cmake --preset release -DMAPF_BUILD_INTEGRATION_TESTS=ON
cmake --build --preset release
ctest --preset release
```

The setting is cached; pass `-DMAPF_BUILD_INTEGRATION_TESTS=OFF` when configuring
to disable them again. To run only integration configurations that are expected
to complete, as CI does, use:

```sh
ctest --preset release -L integration -R '_RequiresCompletion'
```

To run all integration configurations, including those where completion is
optional, use:

```sh
ctest --preset release -L integration
```

Each configuration has a limit of 200 iterations and a 60-second timeout.
Set `MAPF_INTEGRATION_SHORT=1` to reduce the iteration limit to 50 for
configurations where completion is optional.

# Documentation

```sh
python3 -m pip install -r docs/requirements.txt
make -C docs html
```

To build without the map visualizer and its openFrameworks dependencies, use:

```sh
cmake --preset release -DMAPF_BUILD_VISUALIZER=OFF
cmake --build --preset release
```

The `--visualize` command-line option remains available in this configuration,
but reports that visualization support was disabled at compile time.

```aiignore
./out/build/release/MAPF data/warehouse-10-20-10-2-1.map data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize

#Wayland:
XDG_SESSION_TYPE=x11 ./out/build/release/MAPF data/warehouse-10-20-10-2-1.map data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize
```

for more info checkout the repository of the visualizer

## CSP fitness and agent memory

Choose `--fitness` from `manhattan` (default), `sqrt`, `squared`, `piecewise`,
`log`, or `random`. Enable a penalty for returning to recently departed positions
with `--memory-size N` (default 0, disabled) and `--memory-penalty P` (default 1).
The penalty's effect depends on the fitness scale.

Set `--distance shortest-path` to use precomputed distances around static
obstacles with any fitness; `--distance manhattan` is the default. The
shortest-path option rejects scenarios whose start cannot reach its goal;
other agents can still cause deadlocks. Alternatives `--distance lazy-bfs`,
`lazy-astar`, and `lazy-jps` compute exact distances on demand; `astar` precomputes
them using a single h=0 (Dijkstra) traversal per goal. The older `--fitness shortest-path`
is an alias for linear fitness with shortest-path distance.

```sh
./out/build/release/MAPF map.map scenario.scen --algorithm CSP --fitness manhattan --memory-size 5 --memory-penalty 3
./out/build/release/MAPF map.map scenario.scen --algorithm CSP --fitness squared --distance shortest-path
```

Optionally, `--prevent-immediate-return` forbids a return in the next step;
waiting is allowed, as is returning after a wait. See the
[approaches documentation](docs/source/approaches.rst) for details.

## Benchmarks

Download the Moving AI data and run from the project root:

```sh
python3 download_data.py
cmake --build --preset release --target benchmark
./out/build/release/benchmark --help
./out/build/release/benchmark preprocessing bfs benchmarks/maps.txt 10
./out/build/release/benchmark preprocessing lazy-bfs benchmarks/maps.txt 10
./out/build/release/benchmark preprocessing lazy-astar benchmarks/maps.txt 10
./out/build/release/benchmark preprocessing astar benchmarks/maps.txt 10
./out/build/release/benchmark preprocessing lazy-jps benchmarks/maps.txt 10
./out/build/release/benchmark algorithm bfs benchmarks/maps.txt 10 30 > bfs.csv
./out/build/release/benchmark algorithm none benchmarks/maps.txt 10 30 > none.csv
```

## C++ formatting

The repository's `.clang-format` configures clang-format 14 or newer. Editors
can use this file for formatting on save, or an individual file can be formatted
from the project root:

```sh
clang-format -i src/model/Agent.cpp
```

The formatter preserves intentional blank lines within functions, but cannot
determine where logical sections of a test should be separated. Add those
blank lines when writing the test. Formatting is not currently enforced by CI;
existing files can be formatted incrementally when they are changed.
