Research questions and proposed mechanisms
==========================================

This page develops the ideas behind the research diary rather than presenting
new solver features. Unless explicitly marked implemented, the mechanisms below
are proposals for discussion and experiments. They draw on consultations 2–4,
prior project discussions and the 26 November research consultation brief.
The brief itself states that it did not include fresh benchmarks or a complete
source audit. The :doc:`how-it-works` page describes the current implementation.

What is the intended role of the solver?
----------------------------------------

One-step optimization can be studied as a standalone planner: repeatedly pick a
best successor and hope the resulting trajectory reaches every goal. It can
also be used as a successor generator inside a larger search such as LaCAM,
where alternative next configurations might be explored when the first choice
leads nowhere. These are different research questions. A standalone planner
needs a way to recover when its local choices cycle; a search-oriented successor
generator additionally needs alternatives, ordering and a way to avoid
repeating states. Neither integration nor completeness follows automatically.

The baseline already in this repository is sequential priority planning using
A*. It is not automatically the PIBT algorithm. If the thesis asks whether
constraint optimization can replace PIBT's priority mechanism, comparing with a
separately implemented and validated PIBT baseline would make that question
more direct. The appropriate baselines and the precise meaning of
``priority-free`` remain decisions for the research scope.

Stagnation is a signal, not a proof
-----------------------------------

**Problem.** Agents can repeat a configuration, remain in place, or keep
moving without achieving lasting progress. Memory reduces some short returns
but waiting does not add a memory entry and longer cycles can remain. In a
previous reproduction of `issue #13
<https://github.com/JakubOliver/one_step_MAPF/issues/13>`_, the Manhattan-based
run was interrupted after 15 seconds at 66 of 99 agents at their goals; four
global configurations repeated near the end. A later experiment with
obstacle-aware distance reached 95 of 99 in 15 seconds and was also
interrupted. These are historical individual runs with intervening changes,
not a controlled comparison or a current benchmark.

**Possible trigger.** Track, for example, steps since an agent improved its
best goal distance, revisited positions, or repeated joint configurations.
Group membership changes over time, so per-agent history may survive regrouping
more naturally than a counter attached to a transient group. None of these
signals proves deadlock: a necessary detour can temporarily increase distance,
and agents may need to wait for another group to clear a passage.

**Possible responses.** Temporarily adjust costs or noise, enforce a local
coordination rule, or solve with a longer horizon only for stalled agents.
Each choice needs an exit condition, a way to prevent starvation, and tests
where the apparent stagnation is merely a necessary wait. Randomly forbidding
moves also needs a feasibility policy; retaining waiting alone prevents some
infeasibility but does not ensure progress. These are proposed mechanisms for
`issue #26 <https://github.com/JakubOliver/one_step_MAPF/issues/26>`_.

Corridors and displacement chains
----------------------------------

In a narrow passage, two opposing flows may both need to retreat before either
can progress. The immediate sum of distances can prefer waiting or oscillation
over a temporary loss. Candidate ideas from `issue #24
<https://github.com/JakubOliver/one_step_MAPF/issues/24>`_ and the brief are a
majority or weighted direction vote, retaining that direction until the passage
clears, or temporarily reserving entry. Each must define how to detect a
corridor, where agents can retreat, how long a decision lasts, and fairness for
traffic in the other direction. Some configurations have no feasible escape;
a corridor rule cannot make an unsolvable instance solvable.

A longer lookahead is a more general possible recovery, but enlarges the model
and still requires deciding when and for which agents to use it. A direction
lock or changing weights could behave like an implicit priority. Whether that
is acceptable is a thesis definition to settle, not a property established by
calling the mechanism priority-free.

A related case is a chain of agents occupying their goals. A useful move may
require several agents to leave their goals before the blocked agent can pass.
Possible ideas include a blocking cost, temporary weights, keeping track of who
caused displacement, or a lexicographic objective that separates different
criteria. Each needs a precise definition and examples where agents must later
return. A perpendicular escape direction is another hypothesis (`issue #22
<https://github.com/JakubOliver/one_step_MAPF/issues/22>`_); obstacles can make
that direction invalid or lead to a new conflict. If a blocking term depends on
other groups' selected moves, the independence argument in
:doc:`how-it-works` must be reconsidered.

Memory and guidance carry different information
------------------------------------------------

Current bounded memory is a soft penalty on returning to recently departed
cells. Immediate-return prevention is a separate hard restriction; a wait
followed by a return is allowed. Neither proves freedom from cycles. A more
restrictive history could exclude needed backtracking, while a probabilistic
restriction introduces seed and feasibility questions.

A guide path or heatmap would instead express a preferred future direction or
congestion-aware route. It might be used as a soft signal rather than a binding
plan, but then its interaction with one-step decisions and other agents still
needs definition. The guide graph idea and a reference to relevant literature
are recorded in `issue #17
<https://github.com/JakubOliver/one_step_MAPF/issues/17>`_; the idea is not an
implemented or validated improvement here.

Alternative successor configurations
------------------------------------

These are distinct tasks: obtain one minimum-cost assignment; enumerate all
assignments with the same optimum; and generate the best :math:`k` assignments
in nondecreasing cost. Re-solving after forbidding the complete previous
assignment keeps different assignments with equal cost. Forbidding the
previous cost instead loses all other assignments with that value.

The experimental `PR #16
<https://github.com/JakubOliver/one_step_MAPF/pull/16>`_ explores enumeration.
The earlier discussion describes a group-level iterator and a way to combine
group results without globally sorting them. It is not the current main-branch
solver. Candidate costs should remain fixed for the duration of an enumeration,
particularly when they contain random samples; otherwise the ordering can change
between successive solves. Interruption must be distinguishable from exhausting
all assignments.

For independent groups with local alternatives already ordered by cost, a
possible extension is a min-heap of tuples of local indices. Start with the
first solution of each group; after removing the least costly tuple, generate
neighbors by advancing one group index and deduplicate tuples. With additive
costs and ordered local streams, this can enumerate global combinations in
cost order without constructing the full Cartesian product in advance. This is
a proposal, not a claim about the current iterator's behavior; ties, memory,
interruption and potentially exponential output still need handling.

ASP is another possible modeling approach (`issue #21
<https://github.com/JakubOliver/one_step_MAPF/issues/21>`_). Compare time to
first and subsequent solutions, model construction, enumeration semantics and
integration overhead before concluding that it is better for this workload.

Designing informative experiments
---------------------------------

A useful ablation changes one factor at a time: baseline CSP, then obstacle-aware
distance, memory, calibrated fitness shape, and eventually a recovery method.
Compare the same maps, scenarios, agent counts and budgets; record seeds where
randomness is involved. Include open maps, rooms, warehouses and corridors,
with enough repetitions to expose variation. Distinguish a timeout, a valid
partial trajectory and a completed valid solution.

Report completion rate alongside runtime, preprocessing time, path quality and
per-step latency. Define how sum of costs counts arrival and any later departure
from a goal before comparing it. Group-size distributions and repeated
configurations can help explain *why* a run is slow or stalls. Do not average
only successful runs without also reporting failures, or treat a timeout's elapsed
time as the time needed for completion. The present benchmark commands and
historical measurements are documented in :doc:`benchmarks`; this study is a
proposed next step, not an assertion that these measurements already exist.
