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


Recorded results from issue #19
-------------------------------

The following terminal output is copied from issue #19 as a historical record.
It reflects the code and command names at the time of measurement, which may
not match the current implementation. In particular, historical ``jps`` is
now called ``lazy-jps``; the historical ``astar`` command appears under both
A* sections of the issue, before the later naming and implementation changes.
Initialization of lazy methods also changed after these measurements. Timings
are from individual runs and should not be treated as current benchmarks.

.. raw:: html

   <details>
   <summary>10 Agents, BFS — preprocessing bfs</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark preprocessing bfs benchmarks/maps.txt
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   "data/mapf-map/empty-8-8.map","data/mapf-scen-even/scen-even/empty-8-8-even-1.scen",bfs,10,10,0.13684,completed
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",bfs,10,10,0.604617,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",bfs,10,10,3.16744,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",bfs,10,10,8.24357,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",bfs,10,10,3.19643,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",bfs,10,10,3.17884,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",bfs,10,10,3.07444,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",bfs,10,10,3.21413,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",bfs,10,10,3.17531,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",bfs,10,10,15.8708,completed
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",bfs,10,10,25.4843,completed
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",bfs,10,10,44.2247,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",bfs,10,10,116.299,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",bfs,10,10,201.809,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",bfs,10,10,69.0458,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",bfs,10,10,68.9369,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",bfs,10,10,69.2622,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",bfs,10,10,15.8201,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",bfs,10,10,15.7576,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",bfs,10,10,15.5583,completed
   
   Completed: 20/20
   All measured (ms): avg=34.303, max=201.809, min=0.13684
   Completed (ms): avg=34.303, max=201.809, min=0.13684

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>10 Agents, BFS — algorithm bfs</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark algorithm bfs benchmarks/maps.txt
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   "data/mapf-map/empty-8-8.map","data/mapf-scen-even/scen-even/empty-8-8-even-1.scen",bfs,10,10,73.1229,completed
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",bfs,10,10,133.278,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",bfs,10,10,123.039,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",bfs,10,10,67.6546,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",bfs,10,10,153.345,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",bfs,10,10,156.941,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",bfs,10,10,134.772,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",bfs,10,10,83.351,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",bfs,10,10,69.8336,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",bfs,10,10,37.4353,completed
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",bfs,10,10,95.1483,completed
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",bfs,10,10,82.8447,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",bfs,10,10,131.667,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",bfs,10,10,206.678,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",bfs,10,10,113.124,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",bfs,10,10,72.5102,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",bfs,10,10,85.4436,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",bfs,10,10,59.1948,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",bfs,10,10,32.1432,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",bfs,10,10,116.455,completed
   
   Completed: 20/20
   All measured (ms): avg=101.399, max=206.678, min=32.1432
   Completed (ms): avg=101.399, max=206.678, min=32.1432

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>50 Agents, BFS — preprocessing bfs</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark preprocessing bfs benchmarks/maps.txt 50
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",bfs,50,50,3.09084,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",bfs,50,50,15.4666,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",bfs,50,50,44.5493,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",bfs,50,50,15.2068,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",bfs,50,50,15.3255,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",bfs,50,50,15.3593,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",bfs,50,50,15.3336,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",bfs,50,50,15.2691,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",bfs,50,50,76.5235,completed
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",bfs,50,50,123.787,completed
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",bfs,50,50,216.083,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",bfs,50,50,554.39,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",bfs,50,50,954.138,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",bfs,50,50,344.588,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",bfs,50,50,348.806,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",bfs,50,50,348.213,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",bfs,50,50,78.2882,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",bfs,50,50,78.315,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",bfs,50,50,77.9492,completed
   
   Completed: 19/20
   All measured (ms): avg=175.825, max=954.138, min=3.09084
   Completed (ms): avg=175.825, max=954.138, min=3.09084

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>50 Agents, BFS — algorithm bfs (60 s limit)</summary>

.. code-block:: text

   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark algorithm bfs benchmarks/maps.txt 50 60
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",bfs,50,50,785.765,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",bfs,50,50,1083.81,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",bfs,50,50,1357.83,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",bfs,50,50,1231.46,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",bfs,50,50,1093.06,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",bfs,50,50,1424.39,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",bfs,50,50,1128.11,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",bfs,50,50,1537.12,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",bfs,50,50,1715.32,completed
   CSP interrupted; returning paths computed so far
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",bfs,50,50,60000.5,timeout
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",bfs,50,50,1362.79,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",bfs,50,50,1670.54,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",bfs,50,50,1778.54,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",bfs,50,50,1437.29,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",bfs,50,50,1451.63,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",bfs,50,50,1659.39,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",bfs,50,50,1274.23,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",bfs,50,50,1620,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",bfs,50,50,1344.68,completed
   
   Completed: 18/20
   All measured (ms): avg=4471.39, max=60000.5, min=785.765
   Completed (ms): avg=1386.44, max=1778.54, min=785.765

.. raw:: html

   </details>

Source: `BFS benchmark results <https://github.com/JakubOliver/one_step_MAPF/issues/19#issuecomment-7199877474>`_.

.. raw:: html

   <details>
   <summary>BFS-lazy — preprocessing lazy-bfs</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark preprocessing lazy-bfs benchmarks/maps.txt 50
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",lazy-bfs,50,50,0.173671,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",lazy-bfs,50,50,0.504203,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",lazy-bfs,50,50,1.27145,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",lazy-bfs,50,50,0.438177,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",lazy-bfs,50,50,0.467011,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",lazy-bfs,50,50,0.466012,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",lazy-bfs,50,50,0.460694,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",lazy-bfs,50,50,0.470515,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",lazy-bfs,50,50,2.52257,completed
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",lazy-bfs,50,50,4.39324,completed
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",lazy-bfs,50,50,6.76251,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",lazy-bfs,50,50,18.6179,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",lazy-bfs,50,50,28.9195,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",lazy-bfs,50,50,9.09375,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",lazy-bfs,50,50,9.19007,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",lazy-bfs,50,50,9.28088,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",lazy-bfs,50,50,1.87732,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",lazy-bfs,50,50,2.14922,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",lazy-bfs,50,50,2.04744,completed
   
   Completed: 19/20
   All measured (ms): avg=5.21611, max=28.9195, min=0.173671
   Completed (ms): avg=5.21611, max=28.9195, min=0.173671

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>BFS-lazy — algorithm lazy-bfs</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark algorithm lazy-bfs benchmarks/maps.txt 50
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",lazy-bfs,50,50,750.432,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",lazy-bfs,50,50,1071.05,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",lazy-bfs,50,50,1334.93,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",lazy-bfs,50,50,1228.32,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",lazy-bfs,50,50,1095.36,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",lazy-bfs,50,50,1416.6,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",lazy-bfs,50,50,1134.95,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",lazy-bfs,50,50,1531.68,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",lazy-bfs,50,50,1666.44,completed
   CSP interrupted; returning paths computed so far
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",lazy-bfs,50,50,30003.2,timeout
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",lazy-bfs,50,50,1297.38,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",lazy-bfs,50,50,1486.51,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",lazy-bfs,50,50,1499.12,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",lazy-bfs,50,50,1267.7,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",lazy-bfs,50,50,1283.05,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",lazy-bfs,50,50,1477.61,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",lazy-bfs,50,50,1245.61,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",lazy-bfs,50,50,1593.59,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",lazy-bfs,50,50,1311.98,completed
   
   Completed: 18/20
   All measured (ms): avg=2826.08, max=30003.2, min=750.432
   Completed (ms): avg=1316.24, max=1666.44, min=750.432

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>A* — preprocessing astar</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark preprocessing astar benchmarks/maps.txt 50
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",astar,50,50,3.91697,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",astar,50,50,20.4468,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",astar,50,50,49.4592,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",astar,50,50,19.3442,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",astar,50,50,19.4208,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",astar,50,50,19.6116,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",astar,50,50,19.4721,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",astar,50,50,19.4091,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",astar,50,50,92.7968,completed
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",astar,50,50,147.833,completed
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",astar,50,50,256.737,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",astar,50,50,659.021,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",astar,50,50,1144.57,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",astar,50,50,416.457,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",astar,50,50,418.272,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",astar,50,50,417.791,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",astar,50,50,93.8588,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",astar,50,50,92.7204,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",astar,50,50,91.6367,completed
   
   Completed: 19/20
   All measured (ms): avg=210.672, max=1144.57, min=3.91697
   Completed (ms): avg=210.672, max=1144.57, min=3.91697

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>A* — algorithm astar</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark algorithm astar benchmarks/maps.txt 50    
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",astar,50,50,752.448,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",astar,50,50,1091.76,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",astar,50,50,1363.59,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",astar,50,50,1245.44,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",astar,50,50,1106.95,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",astar,50,50,1430.56,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",astar,50,50,1145.77,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",astar,50,50,1561.28,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",astar,50,50,1723.75,completed
   CSP interrupted; returning paths computed so far
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",astar,50,50,30002.3,timeout
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",astar,50,50,1406.15,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",astar,50,50,1754.64,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",astar,50,50,1979.62,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",astar,50,50,1507.37,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",astar,50,50,1526.26,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",astar,50,50,1738.93,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",astar,50,50,1303.22,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",astar,50,50,1654.13,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",astar,50,50,1337.98,completed
   
   Completed: 18/20
   All measured (ms): avg=2928.01, max=30002.3, min=752.448
   Completed (ms): avg=1423.88, max=1979.62, min=752.448

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>A* - lazy — preprocessing astar</summary>

.. code-block:: text

   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark preprocessing astar benchmarks/maps.txt 50
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",astar,50,50,1.0067,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",astar,50,50,2.82635,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",astar,50,50,7.67444,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",astar,50,50,2.7225,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",astar,50,50,2.83993,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",astar,50,50,3.16393,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",astar,50,50,3.36944,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",astar,50,50,3.83546,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",astar,50,50,18.7459,completed
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",astar,50,50,25.4148,completed
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",astar,50,50,58.3504,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",astar,50,50,144.511,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",astar,50,50,250.268,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",astar,50,50,54.6213,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",astar,50,50,60.5999,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",astar,50,50,55.1398,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",astar,50,50,18.1458,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",astar,50,50,19.1167,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",astar,50,50,12.5343,completed
   
   Completed: 19/20
   All measured (ms): avg=39.2046, max=250.268, min=1.0067
   Completed (ms): avg=39.2046, max=250.268, min=1.0067

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>A* - lazy — algorithm astar</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark algorithm astar benchmarks/maps.txt 50
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",astar,50,50,787.379,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",astar,50,50,1143.32,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",astar,50,50,1644.27,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",astar,50,50,1322.26,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",astar,50,50,1167.14,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",astar,50,50,1532.24,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",astar,50,50,1241.61,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",astar,50,50,1671.5,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",astar,50,50,2896.79,completed
   CSP interrupted; returning paths computed so far
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",astar,50,50,30004.1,timeout
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",astar,50,50,9334.39,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",astar,50,50,23092,completed
   CSP interrupted; returning paths computed so far
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",astar,50,50,30186.1,timeout
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",astar,50,50,6761.3,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",astar,50,50,7840.46,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",astar,50,50,7200.96,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",astar,50,50,2453.83,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",astar,50,50,3080.19,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",astar,50,50,2088.76,completed
   
   Completed: 17/20
   All measured (ms): avg=7128.87, max=30186.1, min=787.379
   Completed (ms): avg=4426.96, max=23092, min=787.379

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>JPS-lazy — preprocessing jps</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark preprocessing jps benchmarks/maps.txt 50
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",jps,50,50,1.69357,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",jps,50,50,7.20044,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",jps,50,50,13.7089,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",jps,50,50,5.31247,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",jps,50,50,5.33507,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",jps,50,50,4.97379,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",jps,50,50,6.03507,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",jps,50,50,6.45146,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",jps,50,50,32.509,completed
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",jps,50,50,31.7294,completed
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",jps,50,50,65.2494,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",jps,50,50,159.611,completed
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",jps,50,50,263.413,completed
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",jps,50,50,101.431,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",jps,50,50,106.607,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",jps,50,50,109.933,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",jps,50,50,29.6226,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",jps,50,50,27.2466,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",jps,50,50,29.4683,completed
   
   Completed: 19/20
   All measured (ms): avg=53.0279, max=263.413, min=1.69357
   Completed (ms): avg=53.0279, max=263.413, min=1.69357

.. raw:: html

   </details>

.. raw:: html

   <details>
   <summary>JPS-lazy — algorithm jps (60 s limit)</summary>

.. code-block:: text

   (.venv) [~/Documents/Skola/Vysoka/MFF/Predmety/Bakalarka/one_step_MAPF]
   jakub@fedora$ git:(more-advance-distance-precomputation) ✗ ./out/build/release/benchmark algorithm jps benchmarks/maps.txt 50 60
   map,scenario,method,agents,unique_goals,elapsed_ms,status
   data/mapf-map/empty-8-8.map: Scenario has fewer agents than requested
   "data/mapf-map/empty-16-16.map","data/mapf-scen-even/scen-even/empty-16-16-even-1.scen",jps,50,50,777.888,completed
   "data/mapf-map/empty-32-32.map","data/mapf-scen-even/scen-even/empty-32-32-even-1.scen",jps,50,50,1291.06,completed
   "data/mapf-map/empty-48-48.map","data/mapf-scen-even/scen-even/empty-48-48-even-1.scen",jps,50,50,2044.96,completed
   "data/mapf-map/maze-32-32-2.map","data/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen",jps,50,50,1413.87,completed
   "data/mapf-map/maze-32-32-4.map","data/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen",jps,50,50,1301.19,completed
   "data/mapf-map/random-32-32-10.map","data/mapf-scen-even/scen-even/random-32-32-10-even-1.scen",jps,50,50,1587.06,completed
   "data/mapf-map/random-32-32-20.map","data/mapf-scen-even/scen-even/random-32-32-20-even-1.scen",jps,50,50,1380.91,completed
   "data/mapf-map/room-32-32-4.map","data/mapf-scen-even/scen-even/room-32-32-4-even-1.scen",jps,50,50,1846.84,completed
   "data/mapf-map/room-64-64-8.map","data/mapf-scen-even/scen-even/room-64-64-8-even-1.scen",jps,50,50,4108.23,completed
   CSP interrupted; returning paths computed so far
   "data/mapf-map/warehouse-10-20-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen",jps,50,50,60001.3,timeout
   "data/mapf-map/warehouse-10-20-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-10-20-10-2-2-even-1.scen",jps,50,50,10121.2,completed
   "data/mapf-map/warehouse-20-40-10-2-1.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-1-even-1.scen",jps,50,50,25585.7,completed
   CSP interrupted; returning paths computed so far
   "data/mapf-map/warehouse-20-40-10-2-2.map","data/mapf-scen-even/scen-even/warehouse-20-40-10-2-2-even-1.scen",jps,50,50,60047.1,timeout
   "data/mapf-map/maze-128-128-1.map","data/mapf-scen-even/scen-even/maze-128-128-1-even-1.scen",jps,50,50,11851.1,completed
   "data/mapf-map/maze-128-128-2.map","data/mapf-scen-even/scen-even/maze-128-128-2-even-1.scen",jps,50,50,13517,completed
   "data/mapf-map/maze-128-128-10.map","data/mapf-scen-even/scen-even/maze-128-128-10-even-1.scen",jps,50,50,14084,completed
   "data/mapf-map/random-64-64-10.map","data/mapf-scen-even/scen-even/random-64-64-10-even-1.scen",jps,50,50,3160.51,completed
   "data/mapf-map/random-64-64-20.map","data/mapf-scen-even/scen-even/random-64-64-20-even-1.scen",jps,50,50,3971.86,completed
   "data/mapf-map/room-64-64-16.map","data/mapf-scen-even/scen-even/room-64-64-16-even-1.scen",jps,50,50,3398.08,completed
   
   Completed: 17/20
   All measured (ms): avg=11657.4, max=60047.1, min=777.888
   Completed (ms): avg=5967.14, max=25585.7, min=777.888

.. raw:: html

   </details>

Source: `Additional benchmark results <https://github.com/JakubOliver/one_step_MAPF/issues/19#issuecomment-7201438309>`_.


