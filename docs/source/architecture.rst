Architecture
============

The core library is deliberately independent from the command-line and
visualizer front ends.

.. graphviz::

   digraph architecture {
       rankdir=LR;
       input [label="Map and scenario files"];
       loader [label="InputLoader"];
       model [label="Map + Scenario"];
       algorithms [label="Algorithm implementations"];
       paths [label="AgentPath collection"];
       validator [label="Validator"];
       exporter [label="Exporter"];
       visualizer [label="Optional visualizer"];
       input -> loader -> model -> algorithms -> paths;
       paths -> validator;
       paths -> exporter;
       paths -> visualizer;
   }

The abstract :cpp:class:`Algorithm` interface owns the common entry point for
solvers. Concrete implementations such as :cpp:class:`CSP` and
:cpp:class:`PriorityPlanning` keep their search state in the core library.
Validation is a separate step so that algorithms and output consumers share
the same collision and movement rules.

The visualizer is optional at configure time. It is not required to build or
test the MAPF core.
