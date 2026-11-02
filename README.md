# one_step_MAPF

# How to run

```
cmake --preset release
cmake --build --preset release
ctest --preset release
```

VS Code uses the selected CMake preset automatically. On the first configure,
CMake downloads the declared third-party dependencies, so it can take several
minutes. The build is written to `out/build/release`.

The test suite uses GoogleTest and is run through CTest.

```sh
ctest --preset release
```

or

```sh
ctest --preset release
```

# Documentation

```sh
python3 -m pip install -r docs/requirements.txt
make -C docs html
```

To build without the map visualizer and its openFrameworks dependencies, use:

```sh
cmake --preset release -DMAPF_BUILD_VISUALIZER=OFF
cmake --build --preset release
```

The `--visualize` command-line option remains available in this configuration,
but reports that visualization support was disabled at compile time.

```aiignore
./out/build/release/MAPF data/warehouse-10-20-10-2-1.map data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize

#Wayland:
XDG_SESSION_TYPE=x11 ./out/build/release/MAPF data/warehouse-10-20-10-2-1.map data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize
```

for more info checkout the repository of the visualizer
