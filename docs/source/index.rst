one_step_MAPF documentation
===========================

This documentation combines two complementary sources:

* the conceptual and usage notes in this section, maintained as reStructuredText;
* the C++ API reference generated from the source code by Doxygen and embedded
  into Sphinx through Breathe.

The project implements algorithms for one-step multi-agent path finding
(MAPF). Given a grid map and a set of agent scenarios, an algorithm produces
paths that can be checked by the validator and exported for visualization.

.. toctree::
   :maxdepth: 2
   :caption: Guide

   getting-started
   architecture
   algorithms
   development

.. toctree::
   :maxdepth: 2
   :caption: Reference

   api

Indices and tables
------------------

* :ref:`genindex`
* :ref:`search`
