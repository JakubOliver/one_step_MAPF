Algorithms and validation
=========================

The repository currently contains two solver families:

* :cpp:class:`PriorityPlanning` plans agents sequentially with A* and reserves
  vertices and transitions used by earlier paths.
* :cpp:class:`CSP` processes groups of interacting agents and selects moves
  using a configurable fitness function. It also supports interruption of a
  long-running group solve.

The :cpp:class:`Grouping` helper identifies groups of agents that need to be
handled together. After an algorithm produces paths, :cpp:class:`Validator`
checks that:

* every path matches its input scenario;
* every position is in bounds and not blocked;
* every step is a valid move;
* no two agents share a vertex at the same time;
* no two agents swap an edge in the same time step.

These checks are useful both for command-line runs and for regression tests.
