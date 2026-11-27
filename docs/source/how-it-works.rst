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
made for B could conflict with a decision in another group. A blocked cell
between two agents cannot be their shared destination, so the map can separate
agents that appear close geometrically. The grouping also keeps the relevant
pairs for building the CSP constraints.

The first grouping implementation did not always join all connected agents;
this could create conflicts across groups and lose an agent. The BFS correction
is described in `PR #7 <https://github.com/JakubOliver/one_step_MAPF/pull/7>`_.
Grouping describes possible interaction in **one** step, not a permanent social
group or a prediction of all future interactions.

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
