# Graph Algorithms Library

## Overview
This project implements various graph algorithms for Minimum Spanning Tree (MST) and Shortest Path (SP) problems. The library supports both directed and undirected graphs with configurable representations (adjacency list or matrix).

## Features
- **Minimum Spanning Tree Algorithms**:
    - Prim's algorithm (list and matrix implementations)
    - Kruskal's algorithm (list and matrix implementations)

- **Shortest Path Algorithms**:
    - Dijkstra's algorithm (list and matrix implementations)
    - Bellman-Ford algorithm (list and matrix implementations)

- **Graph Representations**:
    - Adjacency list
    - Incidence matrix (directed and undirected)

## Prerequisites
- CMake (version 3.10+)
- C++ compiler with C++17 support:
    - Linux: GCC 7+ or Clang 5+
    - Windows: Visual Studio 2019+

## Building the Project

### Linux/WSL
```bash
git clone https://github.com/IShnitko/algorithms-2.git
cd algorithms-2
git checkout linux-compatible
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j$(nproc)
```

## Running the Program
The program requires a configuration file:

```bash
# Linux/WSL
./GraphProject config.txt
```


### Configuration File Format
Create a `config.txt` file with parameters:

```ini
.alg_type kruskal_matrix
.num_v 8
.density 0.5
.start_vertex 0
.out_matrix true
.out_list false
```

### Supported Algorithms
- `DIJKSTRA_LIST`
- `DIJKSTRA_MATRIX`
- `BELMAN_FORD_LIST`
- `BELMAN_FORD_MATRIX_EDGE_LIST`
- `BELMAN_FORD_MATRIX_NO_EDGE_LIST`
- `PRIM_LIST`
- `PRIM_MATRIX`
- `KRUSKAL_LIST`
- `KRUSKAL_MATRIX`

### Graph File Format
Graph files should have the following format:
```
<vertex_count> <edge_count>
<source> <destination> <weight>
<source> <destination> <weight>
...
```

Example (`graph.txt`):
```
5 7
0 1 10
0 2 20
1 2 30
1 3 40
2 3 50
3 4 60
4 0 70
```
See more examples in [possible configurations](possible_configs.md).
## Sample Output
```
==== CONFIGURATION ====
Algorithm: kruskal_matrix
Vertices: 8
Density: 0.50
Start vertex: 0
Output matrix: true
Output list: false
Input file: none
========================

1. Generate random graph and run algorithm
--------------------------------
Creating graph with 8 vertices
Creating undirected incidence matrix: vertices=8, density=14
Undirected Incidence Matrix:
 1207 4083 2097 3895    0    0    0    0    0    0    0    0    0    0
    0    0    0    0  839 1140 3573  474    0    0    0    0    0    0
 1207    0    0    0    0    0 3573    0 3750 2992    0    0    0    0
    0    0 2097    0    0    0    0    0    0 2992 3056 1973 2851    0
    0    0    0    0    0    0    0  474    0    0 3056    0    0 3611
    0 4083    0    0    0 1140    0    0    0    0    0    0 2851 3611
    0    0    0 3895  839    0    0    0    0    0    0 1973    0    0
    0    0    0    0    0    0    0    0 3750    0    0    0    0    0
--------------------------------
Results for kruskal_matrix:
Execution time: 0.000371 ms
MST (Kruskal):
  1 - 4 (weight: 474)
  1 - 6 (weight: 839)
  1 - 5 (weight: 1140)
  0 - 2 (weight: 1207)
  3 - 6 (weight: 1973)
  0 - 3 (weight: 2097)
  2 - 7 (weight: 3750)
================================

```

## File Structure
```
graph-algorithms/
├── CMakeLists.txt
├── algorithms/
│   ├── mst/
│   │   ├── prim.cpp
│   │   └── kruskal.cpp
│   └── sp/
│       ├── dijkstra.cpp
│       └── bellman_ford.cpp
├── config/
│   └── configuration.cpp
├── graph/
│   ├── configuration.cpp
│   └── graph.cpp
├── io/
│   ├── file_io.cpp
│   └── display.cpp
├── utils/
│   ├── timer.h
│   ├── dsu.cpp
│   ├── min_heap.cpp
│   ├── random.cpp
│   └── path_utils.cpp
├── config.txt
├── graph.txt (optional)
├── CMakeLists.txt
├── README.md
├── possible_configs.md
└── main.cpp
```

## Troubleshooting
1. **File not found errors**:
    - Use absolute paths in configuration files
    - Place files in the root directory

2. **Build errors**:
    - Ensure you have CMake and a compatible C++ compiler installed
    - Update CMake to latest version (minimum 3.10)

3. **Runtime errors**:
    - Verify graph files have correct format
    - Check algorithm supports graph type (directed/undirected)
    - Ensure vertex indices start at 0

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
