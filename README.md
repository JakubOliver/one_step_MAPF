# one_step_MAPF

# How to run

```
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

VS Code uses the same `debug` preset automatically. On the first configure,
CMake downloads the declared third-party dependencies, so it can take several
minutes. The build is written to `out/build/debug`; old `build` and
`cmake-build-*` directories are not used by VS Code.

The test suite uses GoogleTest and is run through CTest.

To build without the map visualizer and its openFrameworks dependencies, use:

```sh
cmake --preset debug -DMAPF_BUILD_VISUALIZER=OFF
cmake --build --preset debug
```

The `--visualize` command-line option remains available in this configuration,
but reports that visualization support was disabled at compile time.

```aiignore
./out/build/debug/MAPF data/warehouse-10-20-10-2-1.map data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize

#Wayland:
XDG_SESSION_TYPE=x11 ./out/build/debug/MAPF data/warehouse-10-20-10-2-1.map data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize
```

for more info checkout the repository of the visualizer
