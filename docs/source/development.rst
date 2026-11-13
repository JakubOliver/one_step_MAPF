Development
===========

Tests
-----

Run the complete test suite with:

.. code-block:: console

   ctest --preset debug

Documentation locally
---------------------

Install the tools listed in ``docs/requirements.txt`` and make sure Doxygen
is available on ``PATH``. Then run:

.. code-block:: console

   python3 -m pip install -r docs/requirements.txt
   make -C docs html

The generated site is in ``docs/build/html``. The Doxygen XML is an intermediate
artifact and is intentionally not committed; it is regenerated on every docs
build.

CI compiler cache
-----------------

The ``C++ tests`` workflow refreshes the GCC 14 ``ccache`` snapshot on
``main`` daily at 02:13 UTC. To refresh it manually, select ``main`` and run
**C++ tests** from the Actions tab. Successful builds save a new snapshot;
PR builds can restore it using the shared cache prefix.

Check the restore and save step logs and ``ccache`` statistics to verify a
run. GitHub searches the current branch scope before ``main`` and evicts old
caches according to its `cache policy
<https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching#usage-limits-and-eviction-policy>`_.
