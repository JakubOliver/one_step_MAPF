Gallery of observed situations
==============================

These screenshots and recordings come from project issues and show historical
runs, not necessarily the current behavior of every configuration. Images are
loaded from GitHub; the links below each example lead to the original context.
Related commits show when the relevant mechanisms were introduced; they do not
identify the exact revision used for a recording unless explicitly stated.

Corridor deadlock
-----------------

Agents travelling in opposite directions can block one another in a narrow
passage. A locally valid step does not by itself coordinate who should enter or
leave the corridor first.

.. raw:: html

   <a href="https://github.com/JakubOliver/one_step_MAPF/issues/14"><img src="https://github.com/user-attachments/assets/2009851e-f5fb-4ef4-b4a7-35259344a127" alt="Agents stuck in a warehouse corridor" loading="lazy" style="max-width: 100%; height: auto;"></a>

Source: `issue #14 <https://github.com/JakubOliver/one_step_MAPF/issues/14>`_.
A proposed coordination rule is discussed in `issue #24
<https://github.com/JakubOliver/one_step_MAPF/issues/24>`_.
Related commit: `one-step CSP solver
<https://github.com/JakubOliver/one_step_MAPF/commit/413f186>`_.

Moving agents away from goals
-----------------------------

An agent at its goal may need to step aside. When more than one agent must
move, the current cost scheme may not provide enough incentive for the whole
chain to clear the way.

.. raw:: html

   <a href="https://github.com/JakubOliver/one_step_MAPF/issues/23"><img src="https://github.com/user-attachments/assets/9cfaf132-3590-454d-b089-68334c39577d" alt="Example of agents needing to move away from their goals" loading="lazy" style="max-width: 100%; height: auto;"></a>

Source: `issue #23 <https://github.com/JakubOliver/one_step_MAPF/issues/23>`_.
The opposite problem, moving agents away unnecessarily, is recorded in
`issue #5 <https://github.com/JakubOliver/one_step_MAPF/issues/5>`_.
Related commit: `fitness variants
<https://github.com/JakubOliver/one_step_MAPF/commit/dd5c924>`_.

Dense open map and recorded runs
--------------------------------

Consultation 4 describes experiments with bounded memory on an empty 32×32
map. The issue contains two videos, commands and a screenshot of the larger
case. It reports 395 of 399 agents at their goals in that particular run.

.. raw:: html

   <a href="https://github.com/JakubOliver/one_step_MAPF/issues/5"><img src="https://github.com/user-attachments/assets/d96277fd-fc4f-4e02-a9dc-a273ec59e6c8" alt="Dense open map with agents remaining outside their goals" loading="lazy" style="max-width: 100%; height: auto;"></a>

* `Video of a run with bounded memory and shortest-path distance <https://github.com/user-attachments/assets/c4d234a5-6452-46d7-a781-67ecb3d78686>`_.
* `Video of the approximately 300-agent run <https://github.com/user-attachments/assets/398e1d7c-39ba-42e9-a092-f16ffb5eaebf>`_.
* `Original issue and commands <https://github.com/JakubOliver/one_step_MAPF/issues/5>`_.
* Related commits: `bounded memory <https://github.com/JakubOliver/one_step_MAPF/commit/d6543e9>`_
  and `shortest-path distances <https://github.com/JakubOliver/one_step_MAPF/commit/f7445eb>`_.

Small examples for grouping and CSP are described in :doc:`how-it-works`;
additional diagrams can be added as the examples and terminology are refined.
