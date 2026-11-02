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
