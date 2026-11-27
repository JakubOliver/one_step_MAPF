Research progress and open questions
====================================

This is a working research diary, based on the repository history, GitHub
issues and pull requests, notes from consultations 2–4, earlier project
conversations and the 26 November research consultation brief. It records the
state of the main branch and historical observations as of 27 November 2026.
A historical run is evidence about that run, not a guarantee for the current
implementation. See :doc:`how-it-works` for the current algorithm and
:doc:`benchmarks` for the recorded benchmark methodology and raw results.
The discussion of mechanisms and experiment design continues in
:doc:`research-questions`.

Milestones
----------

The timeline shows selected changes and research milestones, not percentage
completion. Dates for merged work refer to the merge or main-branch commit.

.. graphviz::

   digraph timeline {
      rankdir=LR;
      node [shape=box, style="rounded,filled", fillcolor="#edf5ff"];
      a [label="5–11 Oct\nInput, A*, baseline,\nvisualizer, grouping"];
      b [label="23–30 Oct\nOne-step CSP,\nvalidation, grouping fix"];
      c [label="2–13 Nov\nFitness, memory,\nshortest-path distances"];
      d [label="26 Nov\nCSP simplification,\nbenchmarks"];
      a -> b -> c -> d;
   }

* **5–11 October — foundations.** Map and scenario loading, A*, priority
  planning, visualization export and an initial grouping algorithm were added.
  Consultation 2 records the baseline and visualizer, as well as concern that
  benchmark scenarios should be manageable for the baseline. Commits: `A*
  <https://github.com/JakubOliver/one_step_MAPF/commit/55cc7e7>`_,
  `visualization export <https://github.com/JakubOliver/one_step_MAPF/commit/1bd6dc8>`_,
  `initial grouping <https://github.com/JakubOliver/one_step_MAPF/commit/7c495a0>`_.
* **23–30 October — one-step CSP and correctness.** The initial solver chooses
  one simultaneous step subject to collision constraints (`PR #1
  <https://github.com/JakubOliver/one_step_MAPF/pull/1>`_). A validator checks
  the resulting paths (`PR #3 <https://github.com/JakubOliver/one_step_MAPF/pull/3>`_).
  The grouping fix replaced an incomplete grouping method with BFS (`PR #7
  <https://github.com/JakubOliver/one_step_MAPF/pull/7>`_). Commits: `CSP
  <https://github.com/JakubOliver/one_step_MAPF/commit/413f186>`_,
  `validator <https://github.com/JakubOliver/one_step_MAPF/commit/1f2b847>`_,
  `grouping fix <https://github.com/JakubOliver/one_step_MAPF/commit/0db6132>`_.
* **2–13 November — exploration and distance.** Configurable fitness functions,
  memory, and a rule against immediate returns provided ways to explore beyond
  the locally cheapest move (`PR #8 <https://github.com/JakubOliver/one_step_MAPF/pull/8>`_,
  `PR #12 <https://github.com/JakubOliver/one_step_MAPF/pull/12>`_). Shortest
  paths around obstacles became an alternative to Manhattan distance (`PR #15
  <https://github.com/JakubOliver/one_step_MAPF/pull/15>`_). Commits: `fitness
  variants <https://github.com/JakubOliver/one_step_MAPF/commit/dd5c924>`_,
  `memory and other approaches <https://github.com/JakubOliver/one_step_MAPF/commit/bf6ec08>`_,
  `distance precomputation <https://github.com/JakubOliver/one_step_MAPF/commit/f7445eb>`_.
* **26 November — smaller models and benchmarks.** Group interaction pairs
  are reused to omit unnecessary constraints; partial validation and broader
  integration tests were added (`PR #30
  <https://github.com/JakubOliver/one_step_MAPF/pull/30>`_). Completed groups
  bypass the solver (`PR #31 <https://github.com/JakubOliver/one_step_MAPF/pull/31>`_).
  A benchmark entry point and several distance strategies made preprocessing
  and complete runs easier to compare (`PR #34
  <https://github.com/JakubOliver/one_step_MAPF/pull/34>`_). Commits: `CSP
  improvements <https://github.com/JakubOliver/one_step_MAPF/commit/0522005>`_,
  `completed groups <https://github.com/JakubOliver/one_step_MAPF/commit/c18b6d5>`_,
  `benchmarks <https://github.com/JakubOliver/one_step_MAPF/commit/9848ac9>`_.

Experiments and observations
----------------------------

The local optimum problem
~~~~~~~~~~~~~~~~~~~~~~~~~

**Question.** What happens when choosing the best next step repeatedly does
not produce a good complete path?

Agents can oscillate or remain stuck, including in corridors. Memory penalizes
recently departed positions; another option forbids an immediate return.
Consultation 4 reports that bounded memory helped substantially in tested
examples. On an empty 32×32 map, a recorded run with approximately 300 agents
reached their goals, while a larger run left agents stuck; issue #5 reports
395 of 399 at their goals in that particular larger case. The issue contains
commands and videos: `issue #5
<https://github.com/JakubOliver/one_step_MAPF/issues/5>`_. These are individual
observations, not a success rate or an established capacity limit.
Related implementation: `bounded memory
<https://github.com/JakubOliver/one_step_MAPF/commit/d6543e9>`_ and `shortest-path
distances <https://github.com/JakubOliver/one_step_MAPF/commit/f7445eb>`_.

An earlier debugging conversation documented 66/99 agents at their goals
after 15 seconds with Manhattan distance and 95/99 in a later interrupted
shortest-path run. These were individual historical runs, not a controlled
experiment. The first reproduction attributed one observed stall to a wall:
the necessary detour initially increased Manhattan distance. Both runs still
left agents outside their goals. See `issue #13
<https://github.com/JakubOliver/one_step_MAPF/issues/13>`_.

**Next question.** Measure success rates across maps, agent densities and memory
settings, and distinguish deadlocks from timeouts and long runs. The separate
`corridor deadlock issue <https://github.com/JakubOliver/one_step_MAPF/issues/14>`_
remains open.

Fitness and agents already at their goals
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Question.** Can changing the shape of the distance cost or the penalty for
waiting help agents make progress?

Linear, square-root, squared, piecewise, logarithmic and random fitness
variants exist. Consultation 4 reports that the tested concave and convex
transformations did not help much overall. A combination that changes behavior
near the goal was proposed, but had not been sufficiently evaluated; see
`issue #18 <https://github.com/JakubOliver/one_step_MAPF/issues/18>`_. The
current piecewise option is an implementation to test, not evidence that the
idea works.
Related commits: `first fitness variants
<https://github.com/JakubOliver/one_step_MAPF/commit/dd5c924>`_ and `nonlinear
distance functions <https://github.com/JakubOliver/one_step_MAPF/commit/fc42b73>`_.

Agents at goals sometimes need to move aside. Earlier fitness behavior could
make a group prefer pushing several agents away from their goals (`issue #5
<https://github.com/JakubOliver/one_step_MAPF/issues/5>`_). Conversely, a small
penalty for staying can be too weak for chains of agents that need to be moved
(`issue #23 <https://github.com/JakubOliver/one_step_MAPF/issues/23>`_). These
cases illustrate the tension between preserving completed positions and making
space for others.

Distances and preprocessing
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Question.** Is it worthwhile to compute a more informative distance around
obstacles, and how should that work be distributed between initialization and
later queries?

Manhattan distance ignores walls. BFS distance fields provide exact static-map
distances, while lazy BFS, lazy A* and lazy JPS explore on demand; full-field
``astar`` uses a zero heuristic, effectively Dijkstra on the unit-cost grid.
An earlier experiment led to reports that alternative preprocessing did not
improve the tested end-to-end runs and that lazy search could be slow (`issue
#19 <https://github.com/JakubOliver/one_step_MAPF/issues/19>`_). Initialization
alone is not an equivalent comparison because lazy methods defer work. The
recorded runs used historical implementations and names, and must not be
presented as current performance rankings. JPS correctness still needs
investigation (`issue #33 <https://github.com/JakubOliver/one_step_MAPF/issues/33>`_).
Related commits: `shortest-path precomputation
<https://github.com/JakubOliver/one_step_MAPF/commit/f7445eb>`_ and `benchmark
and distance strategies <https://github.com/JakubOliver/one_step_MAPF/commit/9848ac9>`_.

The earlier discussions also expose an important experimental detour. An
intermediate eager A* implementation ran a separate point-to-point search
for every cell and goal. On the 32×32 empty map with 50 goals this meant about
51,000 searches rather than 50 reverse traversals; the reported preprocessing
times were roughly 2.6 seconds and 15 milliseconds respectively. This
compared different amounts of repeated work, not the inherent speed of a
single A* against BFS. The final ``astar`` instead computes each full field
with one priority-queue traversal and a zero heuristic: it is Dijkstra's
algorithm, not goal-directed A*. Lazy A* remains a point-query strategy. These
historical measurements are illustrative, not a benchmark of the current code.

Another distinction from that discussion is that lazy initialization still
validates start-to-goal connectivity, even though it defers distance queries.
Thus neither initialization nor the full run is automatically free of
preprocessing. Historical commands and names changed during the experiment;
the exact context is recorded in :doc:`benchmarks` and `issue #19
<https://github.com/JakubOliver/one_step_MAPF/issues/19>`_.

Grouping and CSP overhead
~~~~~~~~~~~~~~~~~~~~~~~~~

Grouping enables independent local decisions, but creating a solver for each
small group also has a cost. An isolated agent is already handled directly;
completed groups wait. Whether merging small independent groups into a single
solver call would help remains a benchmark question (`issue #28
<https://github.com/JakubOliver/one_step_MAPF/issues/28>`_). Parallel group
processing also needs measurement, including scheduling overhead and an
appropriate group-size threshold (`issue #27
<https://github.com/JakubOliver/one_step_MAPF/issues/27>`_).
Related commits: `single-agent optimization
<https://github.com/JakubOliver/one_step_MAPF/commit/999fd50>`_ and `completed
groups <https://github.com/JakubOliver/one_step_MAPF/commit/c18b6d5>`_.

Testing correctness and completion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

While integrating the grouping changes, an early local run tested 48
configurations on one fixed empty-map scenario with a limit of 200 iterations.
Ten completed and 38 did not; after partial validation was added, all 48
produced valid partial paths in a subsequent run. These were historical test
runs, not a comparative success-rate study: they cover one scenario, and a
limit or random choices affect outcomes. The distinction prompted separate
checks for collision-free partial paths and completion. See `PR #30
<https://github.com/JakubOliver/one_step_MAPF/pull/30>`_.

An iteration limit lets a test finish and validate the partial paths. A
process timeout can stop the test before validation, so the two limits answer
different questions. The current CI runs integration configurations marked as
requiring completion; the full set, including configurations where completion
is optional, can be run locally as described in the README. A passing CI run
therefore does not establish that every supported configuration completes on
every scenario.

Further research
----------------

* **Corridors and stagnation:** detect a lack of progress and investigate
  coordination or direction selection in narrow passages. These are ideas,
  not implemented guarantees (`#24 <https://github.com/JakubOliver/one_step_MAPF/issues/24>`_,
  `#26 <https://github.com/JakubOliver/one_step_MAPF/issues/26>`_).
* **Displacement:** improve chains of agents moving away from goals and
  investigate perpendicular evasion (`#23 <https://github.com/JakubOliver/one_step_MAPF/issues/23>`_,
  `#22 <https://github.com/JakubOliver/one_step_MAPF/issues/22>`_).
* **Guidance and lookahead:** investigate guide paths or heatmaps, longer
  lookahead and possible LaCAM integration; their effect and scope are open
  (`#17 <https://github.com/JakubOliver/one_step_MAPF/issues/17>`_).
* **Alternative solutions and solvers:** an iterator over one-step solutions is
  a proof of concept in open `PR #16
  <https://github.com/JakubOliver/one_step_MAPF/pull/16>`_, not part of the
  main solver described here. ASP is another proposed avenue (`#21
  <https://github.com/JakubOliver/one_step_MAPF/issues/21>`_). Excluding a
  previous objective value can skip distinct solutions with the same cost;
  excluding individual solutions may add overhead.
* **Evaluation:** compare against the baseline on a range both algorithms can
  handle, record completion and validity separately, and repeat measurements
  with the configuration, revision and timeout documented.

Consultation notes also raise how to define priority-free behavior when agents
communicate or temporarily remember reaching a goal. This is a research and
scope question, not yet a settled design decision.
