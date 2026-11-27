How the one-step solver works
=============================

The project studies a simple question: how far can agents get if they choose
only their **next** move together, without assigning fixed priorities? The
one-step CSP solver repeats this local decision until all agents reach their
goals or the run is interrupted. The separate priority-planning algorithm
provides a baseline. This page describes the implementation on the main branch;
the :doc:`research diary <research-progress>` follows how it developed.

.. graphviz::

   digraph iteration {
      rankdir=LR;
      node [shape=box, style="rounded,filled", fillcolor="#edf5ff"];
      positions [label="Current positions"];
      groups [label="Find interacting\ngroups"];
      candidates [label="Generate moves\nand their costs"];
      solve [label="Choose compatible\nmoves per group"];
      move [label="Apply one step"];
      positions -> groups -> candidates -> solve -> move;
      move -> positions [label="repeat"];
   }

Map, agents and time
--------------------

The input is a grid with static obstacles and a start and goal for each agent.
At each time step an agent can wait or move to an orthogonally adjacent free
cell. All agents make a step simultaneously. They must not occupy the same
cell at the same time, and two agents cannot exchange positions in one step.
An agent already at its goal can leave it to let another agent pass.

The solver records the chosen positions as paths. A validator can check the
map, moves and collisions; full validation also checks whether the agents reach
their goals. A valid partial path is not necessarily a completed solution.

Grouping: who needs to decide together?
----------------------------------------

Solving for every agent in a single model can be unnecessarily expensive. Two
agents need to be connected if they can choose the same destination in the next
step. Adjacency also covers the possibility of swapping their positions. Agents
connected directly or through others form a group, found with breadth-first
search. Groups are recomputed at each step as agents move.

For example, if A can interact with B and B with C, all three must be in the
same group even when A and C cannot interact directly. Otherwise, a decision
made for B could conflict with a decision in another group.

The map matters as well as the distance between agents. Consider ``A . B``,
where the dot is a free cell: both agents could move into that cell, so they
must be connected. If it is a wall instead (``A # B``), there is no shared
destination for this pair in one step. For diagonally positioned agents there
are two possible intermediate cells; both must be blocked to eliminate their
shared destinations. Removing such impossible interactions can split a group
into smaller independent groups, although agents can still be connected
indirectly through others. The grouping also keeps the relevant pairs for
building the CSP constraints. The obstacle-aware change is described in
`PR #30 <https://github.com/JakubOliver/one_step_MAPF/pull/30>`_.

The first grouping implementation did not always join all connected agents;
this could create conflicts across groups and lose an agent. The BFS correction
is described in `PR #7 <https://github.com/JakubOliver/one_step_MAPF/pull/7>`_.
Grouping describes possible interaction in **one** step, not a permanent social
group or a prediction of all future interactions.

The resulting components are independent for the current step, but it is
still an open performance question how best to process them:

* **Parallel processing:** independent groups could be solved concurrently.
  Starting or scheduling work has a cost, so small groups may be faster to
  process sequentially. Benchmarks should investigate the threshold for
  parallel work, the distribution of group sizes, and a fixed worker pool
  versus other scheduling strategies. See `issue #27
  <https://github.com/JakubOliver/one_step_MAPF/issues/27>`_.
* **Combining small groups:** the smallest independent groups expose the most
  parallelism, but each CSP invocation has setup overhead. Combining several
  independent small groups into one model might reduce that overhead at the
  cost of a larger solve and less parallelism. This tradeoff needs measurement;
  isolated agents and completed groups already have special handling. See
  `issue #28 <https://github.com/JakubOliver/one_step_MAPF/issues/28>`_.

These are research ideas, not implemented optimizations or established
thresholds. Merging independent groups preserves the same one-step additive
objective under the assumptions explained below, but need not select the same
assignment when multiple solutions have equal cost.

Encoding one group as a CSP
---------------------------

A constraint satisfaction problem (CSP) specifies variables, the values they
can take, and rules that valid combinations must satisfy. Here the project uses
OR-Tools CP-SAT to optimize such a model:

* Each agent has a variable for its next position. Its domain contains the
  current cell and valid neighboring cells; blocked or out-of-bounds cells are
  excluded. A cell is encoded as ``x + y * map_width``.
* A candidate index links each position to its precomputed fitness cost.
* Agents whose possible destinations overlap receive an all-different
  constraint, preventing two agents from choosing the same cell.
* For relevant adjacent pairs, a forbidden assignment prevents exchanging
  their current cells. Distinct destinations alone would not prevent a swap.
* The objective minimizes the sum of the selected candidate costs in the group.

Mathematical formulation
~~~~~~~~~~~~~~~~~~~~~~~~

For a group :math:`G`, let :math:`p_i` be the current position of agent
:math:`i`, and let :math:`N(p_i)` be its orthogonally adjacent traversable
cells. Its next-position variable :math:`q_i` has domain

.. math::

   q_i \in D_i = \{p_i\} \cup N(p_i), \qquad i \in G.

When immediate returns are forbidden, the previous position is removed from
:math:`D_i` if it is a move candidate; waiting remains allowed. In the solver,
each cell :math:`(x,y)` is encoded as the integer :math:`x + yW`, where
:math:`W` is the map width.

To exclude vertex collisions and edge swaps, a valid assignment satisfies

.. math::

   q_i &\ne q_j, \\
   \neg\bigl((q_i = p_j) \land (q_j = p_i)\bigr),
   \qquad i,j \in G,\; i < j.

These equations describe the logical conditions for all pairs. The actual
model uses ``AllDifferent`` for components of overlapping domains and adds
forbidden swap assignments only where the swap is possible. Pairs whose domains
cannot conflict need no explicit pairwise constraint.

For each agent, the model enumerates candidates :math:`d_{i,0},\ldots,
d_{i,m_i-1}` and their corresponding costs :math:`c_{i,0},\ldots,
c_{i,m_i-1}`. A candidate index :math:`k_i` connects position and cost:

.. math::

   k_i &\in \{0,\ldots,m_i-1\}, \\
   q_i &= d_{i,k_i}, \\
   z_i &= c_{i,k_i}.

CP-SAT encodes these last two relations with element constraints. Costs are
calculated for each candidate before solving; they include the selected
fitness and, when enabled, the memory penalty. The objective is

.. math::

   \min \sum_{i\in G} z_i.

For example, the default linear fitness for candidate position :math:`q_i`
is :math:`2\,\operatorname{dist}(q_i,g_i) + [q_i=p_i]`, where :math:`g_i`
is the goal and the bracket equals 1 for waiting and 0 otherwise. The selected
distance can be Manhattan or a static-map shortest-path distance. Other fitness
functions and optional memory penalties change the candidate costs; their
exact definitions are listed in :doc:`approaches`.

The model is used for groups with multiple agents unless all are already at
their goals. Those complete groups wait, while a single agent is evaluated
directly without constructing a CP-SAT model.

Consider two adjacent agents A and B. Choosing B's current cell for A and A's
current cell for B would produce different destinations, but is forbidden as
an edge swap. A can move into B's cell if B moves elsewhere and the other
constraints allow it. If everyone waits, a valid group has a collision-free
choice; nevertheless, an interrupted solve may not return an optimal step.

The objective is optimal for the **current group and current step**. It does
not optimize complete paths or prove that agents will eventually reach their
goals. Independent groups have no possible one-step collision between them;
their separate additive objectives can therefore be combined for that step.
This observation does not establish global path optimality.

Fitness, distance and memory
----------------------------

Fitness expresses which valid step the solver prefers; lower is better. The
cost is built from two separate choices:

* **Distance:** Manhattan distance is cheap but ignores obstacles. A shortest
  path distance accounts for static obstacles, but not other agents. The
  project offers full BFS precomputation and experimental on-demand searches.
* **Transformation:** linear, square-root, squared, piecewise, logarithmic and
  random variants assign different costs to distances and waiting. A convex
  transformation can give distant agents more weight; a concave one discounts
  larger distances. The random variant has its own wait behavior, so it is not
  simply noise added to the linear fitness.

The :doc:`approaches` page gives the exact functions and options. One subtlety
is that penalties and fitness functions have different scales: a memory penalty
of one does not have the same relative effect for every fitness.

Optional bounded memory records recently departed positions and penalizes a
return to one of them. Waiting does not add a position to that memory. A
separate option forbids returning to the immediately preceding position; after
a wait, returning is permitted. Both mechanisms encourage exploration but can
still leave agents waiting or cycling.

Special cases and performance
-----------------------------

A group with one agent can choose its cheapest valid candidate directly,
without creating a CSP model. A group whose agents are all at their goals waits
without invoking the solver. These decisions are made only after grouping: an
agent at its goal can still be moved out of the way if it shares a group with
an unfinished agent.

The model reuses interaction pairs identified by grouping. It creates swap
constraints only for pairs that can actually exchange positions, and separates
independent overlapping-domain components when applying all-different. These
changes reduce unnecessary model construction; their performance impact depends
on the scenario and should be measured rather than assumed.

Why one-step decisions can fail
--------------------------------

A locally attractive step may lead into a deadlock, oscillation or a narrow
corridor where opposing agents cannot pass. Even an exact shortest path around
walls ignores the future actions of other agents. Penalizing revisits helps in
some cases but does not prove convergence. The :doc:`gallery` illustrates
observed cases, while :doc:`research-progress` records the experiments and
open questions.

Why independent optimization works for one step
------------------------------------------------

Let :math:`G_1,\ldots,G_k` be groups with feasible candidate combinations
:math:`X_1,\ldots,X_k`. If there are no constraints between groups and each
candidate cost is fixed before solving, the total objective is a sum of group
costs. Therefore

.. math::

   \min_{x\in X_1\times\cdots\times X_k} \sum_j F_j(x_j)
   = \sum_j \min_{x_j\in X_j} F_j(x_j).

Every global choice costs at least the sum of the individual minima. Choosing
one minimizer from each group reaches that sum, so a second optimization over
their optima cannot improve this one-step objective. This argument depends on
complete interaction detection, a product of feasible sets and an additive
objective. Costs or constraints depending on *chosen moves* in another group
would require extending the interaction graph or changing the decomposition.
It says nothing about optimal complete trajectories or eventual arrival.

Two graphs inside one iteration
-------------------------------

There is a subtle difference between the groups and the components used for
vertex constraints. Grouping conservatively records pairs that can interact
based on position and obstacles. Inside the solver the actual domains can be
smaller, for example when immediate returns are forbidden. A second graph joins
pairs whose actual destination domains overlap. The implementation uses a
union-find structure to create one ``AllDifferent`` constraint per component
of this second graph. Swap constraints are checked separately against the
recorded pairs.

For example, A can share a candidate cell with B, and B with C. The three can
share one ``AllDifferent`` even if A's and C's domains are disjoint: the
constraint between those two adds no restriction because their values cannot
coincide. The grouping records all interaction edges, including edges between
already visited agents, not just the BFS discovery tree. Otherwise a cycle of
interactions could leave out a necessary collision check.

An alternative is one inequality for each pair with overlapping domains, or a
single ``AllDifferent`` for the entire group. Both are possible formulations;
whether a smaller model or a global constraint performs better depends on the
solver and instance. No speedup should be inferred without benchmarking. See
`PR #30 <https://github.com/JakubOliver/one_step_MAPF/pull/30>`_.

What fitness actually trades off
--------------------------------

The solver does not simply count how many agents move closer. It compares the
sum of numerical costs. With the linear cost, improving a distance by one
reduces its distance contribution by two. Moving three goal agents one cell
away raises that contribution by six. A single agent moving one cell closer
cannot compensate for those three changes through distance alone. Waiting
penalties and other terms may change the comparison, but a tiny incentive to
move cannot generally express the future benefit of clearing an entire chain.

Conversely, rewarding exploration too much may make agents leave their goals
unnecessarily. This is why push-out behavior needs examples involving both a
single blocked agent and a chain, rather than testing only isolated agents.
See `issue #5 <https://github.com/JakubOliver/one_step_MAPF/issues/5>`_ and
`issue #23 <https://github.com/JakubOliver/one_step_MAPF/issues/23>`_.

Even changing only the distance transformation changes several things at once.
For linear fitness, moving from distance 10 to 9 changes the distance cost by
2; for squared fitness, it changes from 200 to 162, a difference of 38. A
memory penalty of 1 therefore represents a different relative incentive. The
same issue occurs with wait penalties and the much larger scale of the square
root and logarithm variants. Comparing the shapes fairly requires choosing a
reference distance range and calibrating penalties relative to a typical
one-step cost change. The current parameters are experimental; calibration is
a proposed evaluation method, not an implemented automatic adjustment.

Distance fields and repeated queries
------------------------------------

For one goal on an unweighted four-neighbor grid, a reverse BFS computes the
distance to every reachable cell in time proportional to the traversable grid
and its edges. Agents sharing a goal can share the resulting field. The
up-front work and memory grow with the number of distinct goals; subsequent
queries are inexpensive lookups.

A point-to-point A* search can explore fewer cells for one query, but the CSP
may ask for distances from many candidate positions over many iterations.
Repeated queries and caching change the tradeoff. Lazy reverse BFS retains a
frontier and extends it when necessary, reusing work; lazy A* searches for
missing positions and caches results. A seemingly faster initialization can
therefore move work into the solving phase. JPS can skip symmetric positions
in point searches, but this does not automatically produce a complete distance
field. The experimental JPS implementation still needs the verification tracked
in `issue #33 <https://github.com/JakubOliver/one_step_MAPF/issues/33>`_.
