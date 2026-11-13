# Disaster Relief Routing Optimizer

A C++ console simulation for routing emergency vehicles through a city road network during disasters. Built for our Data Structures Lab (15B17C1371) at JIIT Noida.

The city is modeled as a weighted graph where intersections are nodes and roads are edges with travel times[cite: 2]. When an emergency is triggered at a location, the program finds the nearest active relief station, computes the shortest route along with backup paths, and automatically switches routes if a road or intersection gets blocked[cite: 2, 3].

---

## How It Works

* **Graph (`Graph.hpp`)**: Adjacency list storing nodes (coordinates, operational status, station flag) and roads (travel cost, active status, scheduled maintenance windows)[cite: 2, 3, 4].
* **Priority Queue (`MinHeap.hpp`)**: Custom binary min-heap used by both pathfinding algorithms[cite: 3].
* **Station Discovery (`Router.hpp`)**: Runs Dijkstra outward from the disaster node to locate the closest reachable relief station[cite: 3].
* **Routing & Failover (`Router.hpp`)**: Uses $A^*$ (with Euclidean distance heuristic) for the main path and generates backup paths by masking primary edges[cite: 3]. If an active road breaks or closes during the simulation, the monitor switches to a valid backup or recalculates a new path[cite: 2, 3].

---

## Files

* `main.cpp` — Menu interface, simulation clock, failover checks, and Dijkstra vs. $A^*$ benchmark.
* `Graph.hpp` — Graph structure, map file parser, and scheduled closure checks[cite: 2, 3].
* `MinHeap.hpp` — Binary min-heap implementation[cite: 3].
* `Router.hpp` — Dijkstra, $A^*$, backup path generator, and route validation[cite: 3].
* `city_map.txt` — Default 15-node city map with 3 relief stations and sample road closures.

---

## Build & Run

Requires a C++17 compiler (`clang++` or `g++`).

```bash
clang++ -std=c++17 -O2 -Wall main.cpp -o router
./router