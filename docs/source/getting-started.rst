Getting started
===============

Build and run
-------------

The project uses CMake presets. A typical release build is:

.. code-block:: console

   cmake --preset release
   cmake --build --preset release
   ctest --preset release

The visualizer can be disabled when its Linux/openFrameworks dependencies are
not available:

.. code-block:: console

   cmake --preset release -DMAPF_BUILD_VISUALIZER=OFF
   cmake --build --preset release

The command-line executable accepts a map file and a scenario file. The
``--visualize`` option is available when visualization support was enabled.

Input and output
----------------

``InputLoader`` reads the map and scenario files into :cpp:type:`Map` and
``std::vector<Scenario>``. An :cpp:class:`Algorithm` produces one
:cpp:class:`AgentPath` per scenario. The resulting paths can be checked by
:cpp:class:`Validator` and written by :cpp:class:`Exporter`.
