Approaches and experiments
==========================

This page records the implemented solver options and ideas from the third
consultation. Ideas marked **not implemented** are directions for discussion,
not commitments to a particular design. The CSP fitness and memory options
apply only when ``--algorithm CSP`` is selected.

Solver approaches
-----------------

* **Priority planning — implemented.** ``--algorithm PriorityPlanning`` plans
  agents sequentially with A*, reserving vertices and transitions for earlier
  paths. This is the default algorithm.
* **One-step CSP — implemented.** ``--algorithm CSP`` divides interacting
  agents into groups and minimizes the sum of candidate costs within each
  group. Each candidate is a wait or an orthogonal move; vertex collisions and
  edge swaps are forbidden. This local optimization does not guarantee that
  agents reach their goals or avoid deadlocks.
* **LaCAM integration — not implemented.** Connecting the one-step approach to
  LaCAM is an open question from the consultation.
* **Multi-step lookahead — not implemented.** Consider additional future steps
  when choosing the current move, depending on the role of LaCAM.
* **Alternative solutions — not implemented.** Repeatedly solve while excluding
  previous solutions or fitness values. Excluding a fitness value can also skip
  other solutions with the same value; excluding individual solutions may incur
  additional overhead.
* **Global sum of group optima — not implemented.** The consultation notes
  suggest that, if groups are independent, the sum of their local optima may
  already be the global sum optimum. The usefulness of a separate optimization
  remains to be investigated.

Agent variants and memory
-------------------------

* **Basic agent — implemented.** The default CSP variant uses ``Agent`` without
  local memory.
* **Agent with bounded local memory — implemented.** With a positive
  ``--memory-size N``, CSP uses ``MemoryAgent``, which remembers the last N
  positions the agent departed. Waiting does not add a record. Returning to a
  remembered position adds ``--memory-penalty P`` once to the selected fitness;
  it does not forbid the move. The penalty defaults to 1 and must be positive.
  Memory is disabled by default (``--memory-size 0``).
* **Recent-goal priority — not implemented.** Remember for a few iterations
  that an agent reached its goal, and use this information to adjust priority
  when agents displace one another. The exact policy and its suitability for
  the thesis need clarification.
* **Prevent immediate return — implemented.** ``--prevent-immediate-return``
  forbids a move back to the position occupied in the preceding time step.
  Waiting is always allowed, and a return after waiting is allowed. This flag
  works independently of, or together with, memory penalties and any fitness.
  It may still cause agents to wait or enter other cycles; it does not guarantee
  that they reach their goals.
* **Forbid cycles or revisits — not implemented.** Prohibit returning to
  previously visited positions, possibly only with a chosen probability. The
  history window, probability, and behavior when moves become infeasible are
  open design choices.

CSP fitness functions
---------------------

Select a fitness with ``--fitness NAME``. CSP minimizes the sum of these costs
for each group. Let ``d`` be the selected distance from a candidate position to
the agent's goal, and let a wait mean remaining at its current position.
``--distance manhattan`` is the default; ``--distance shortest-path`` uses BFS
to precompute distances around static obstacles for each distinct goal. It
rejects unreachable starts, but other agents can still cause deadlocks.

* ``manhattan`` — **implemented; default.** Cost is ``2 * d + 1`` for a wait
  and ``2 * d`` for a move.
* ``sqrt`` — **implemented.** Cost is
  ``round(1000 * (sqrt(d + 4) - sqrt(4)))``, plus 200 for a wait. This is a
  concave distance transformation.
* ``squared`` — **implemented.** Cost is ``2 * d * d``, plus 1 for a wait.
  This convex function gives greater weight to agents farther from their goals.
* ``piecewise`` — **implemented.** For ``d <= 4``, cost is ``2 * d * d``;
  beyond 4 it is ``32 + 16 * (d - 4)``. Add 1 for a wait. The linear part
  continues the quadratic curve at the threshold with the same slope. The
  threshold of 4 is an experimental fixed choice.
* ``log`` — **implemented.** Cost is ``round(1000 * log(1 + d))``, plus 200
  for a wait. This concave function uses the natural logarithm and increasingly
  discounts larger distances.
* ``random`` — **implemented.** A move costs ``d`` plus a rounded sample from
  a normal distribution with mean 1 and standard deviation 1. A wait costs 0.
  Samples are generated when candidate costs are constructed.
* ``shortest-path`` — compatibility alias for ``--fitness manhattan --distance
  shortest-path``. The distance option can also be used with any other fitness.
* **Other nonlinear distance functions — not implemented.** Other powers
  and other convex or concave transformations are candidates for
  experimentation. Symmetric situations may still cause deadlocks.
* **Additional noise variants — not implemented.** Uniform noise and a
  configurable noise mechanism are ideas beyond the existing ``random``
  fitness. The current random fitness has its own wait behavior, so it is not
  simply noise added to the other fitness functions.

Memory penalties are added to the selected fitness, so their effect depends on
its scale. The same parameters need not have comparable effects for
``manhattan`` and ``sqrt``.

Other exploration ideas
-----------------------

* **Randomly restrict moves — not implemented.** Forbid selected candidate
  moves with a small probability to encourage exploration. The policy must
  account for cases where restrictions make a group infeasible.
* **Generated test scenarios — not implemented as an approach.** The
  consultation raises a question about using AI to generate test scenarios and
  disclosing that use in the thesis; it does not define a solver feature.
* **Grid path finding — open topic.** The notes mention grid path finding without
  specifying an additional approach to implement.

Example
-------

.. code-block:: console

   ./out/build/release/MAPF map.map scenario.scen --algorithm CSP --fitness manhattan --memory-size 5 --memory-penalty 3

This combines Manhattan fitness with a five-position memory. Replace ``map.map`` and ``scenario.scen`` with paths to input files. See
:doc:`getting-started` for build and input details.
