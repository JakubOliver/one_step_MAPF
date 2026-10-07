# one_step_MAPF

# How to run

```
cmake -S . -B build
cmake --build build -j
```
```aiignore
./MAPF ../data/warehouse-10-20-10-2-1.map ../data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize

#Wayland:
XDG_SESSION_TYPE=x11 ./MAPF ../data/warehouse-10-20-10-2-1.map ../data/scen-even/warehouse-10-20-10-2-1-even-0.scen --visualize
```

for more info checkout the repository of the visualizer