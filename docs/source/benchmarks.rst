Benchmarks
==========

The ``benchmark`` executable measures distance initialization and its effect
on the CSP solver. The distance strategies are experimental work related to
`issue #19 <https://github.com/JakubOliver/one_step_MAPF/issues/19>`_.

Setup and inputs
----------------

Download the Moving AI maps and both scenario sets, then build the benchmark
in Release mode from the repository root:

.. code-block:: console

   python3 download_data.py
   cmake --preset release
   cmake --build --preset release --target benchmark
   ./out/build/release/benchmark --help

``benchmarks/maps.txt`` lists map and scenario pairs, one pair per line. Paths
are relative to the working directory and separated by whitespace; paths with
spaces are not supported. Lines beginning with ``#`` are ignored. Supply another
list to choose different cases. Each run uses the first requested number of
scenarios as agents; the default is 10. A scenario file with fewer agents than
requested is reported as an error and skipped.

Distance strategies
-------------------

``bfs``
   Compute an exact distance field from each unique goal to every reachable
   cell before CSP starts, using one breadth-first traversal per goal.
``astar``
   Compute the same complete distance fields before CSP starts. Since the
   field has no single destination for a heuristic to guide the search toward,
   this uses ``h = 0`` and is a Dijkstra traversal with a priority queue. On
   this unit-cost grid it has no heuristic advantage over BFS.
``lazy-bfs``
   Start a reverse BFS from each goal and extend it only when a requested
   position has not yet been reached. Reuse its queue and computed distances.
``lazy-astar``
   Compute a missing position-to-goal distance with A* and a Manhattan
   heuristic. Cache the result for that position and goal.
``lazy-jps``
   Compute a missing position-to-goal distance using four-directional JPS4 and
   cache the result for that position and goal. JPS4 follows the pruning rules
   described in `Jump Point Search Pathfinding in 4-connected Grids
   <https://arxiv.org/abs/2501.14816>`_.
``none``
   For the ``algorithm`` benchmark only, use Manhattan distance without a
   shortest-path cache. This ignores obstacles and can change CSP's decisions.

All strategies other than ``none`` calculate exact distances around static
obstacles. The lazy strategies validate start-to-goal connectivity during
initialization, but calculate distances only when requested. All exact methods
reject invalid goals and unreachable starts.

Running and interpreting results
--------------------------------

.. code-block:: console

   ./out/build/release/benchmark preprocessing bfs benchmarks/maps.txt 10
   ./out/build/release/benchmark preprocessing lazy-astar benchmarks/maps.txt 10
   ./out/build/release/benchmark algorithm bfs benchmarks/maps.txt 10 30 > bfs.csv
   ./out/build/release/benchmark algorithm lazy-jps benchmarks/maps.txt 10 30 > jps.csv
   ./out/build/release/benchmark algorithm none benchmarks/maps.txt 10 30 > none.csv

``preprocessing`` measures construction of the distance strategy, including
start validation. It excludes file loading. A lower initialization time for a
lazy strategy does not mean the same work is finished: later queries can still
perform searches.

``algorithm`` measures initialization and the complete CSP run, including
any distance queries. It excludes file loading and final path validation. CSP
uses Manhattan fitness, memory size 5 and memory penalty 1; immediate returns
are not forbidden. The last optional argument is the time limit in seconds per
map, defaulting to 30. The limit is checked cooperatively by CSP: it includes
initialization in the elapsed time but cannot interrupt a distance search or
initialization, so the actual run can exceed it.

Both commands write one CSV row per measured case to standard output, with
map, scenario, method, agent count, unique goal count, elapsed milliseconds and
status. The summary is written to standard error and gives completed cases out
of all inputs, followed by mean, maximum and minimum times for all measured
cases and for completed cases. Cases that fail before a time is recorded are
excluded from timing statistics. A ``timeout`` is incomplete and its elapsed
time is not the time needed to solve the case.

Compare the same maps and agent counts, and compare completed cases separately
from timeouts. Repeat measurements in Release mode. In particular, comparing
initialization alone can favor lazy methods because work is deferred; the
``algorithm`` benchmark includes that deferred work.
