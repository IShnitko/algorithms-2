# Possible configs for the program
### Random graph represented by matrix (Dijkstra algorithm)
```
.alg_type dijkstra_matrix
.num_v 5
.density 0.7
.start_vertex 0
.out_matrix true
.out_list false
```
### Random graph represented by list of neighbours (Dijkstra algorithm)
```
.alg_type dijkstra_list
.num_v 8
.density 0.5
.start_vertex 0
.out_matrix false
.out_list true
```
### Random graph represented by list of neighbours (Bellman-Ford algorithm)
```
.alg_type belman_ford_list
.num_v 8
.density 0.5
.start_vertex 0
.out_matrix false
.out_list true
```
### Random graph represented by matrix (Bellman-Ford algorithm)
```
.alg_type belman_ford_matrix_edge_list
.num_v 8
.density 0.5
.start_vertex 0
.out_matrix true
.out_list false
```
### Random graph represented by matrix (Bellman-Ford algorithm)
```
.alg_type belman_ford_matrix_no_edge_list
.num_v 8
.density 0.5
.start_vertex 0
.out_matrix true
.out_list false
```
### Random graph represented by list of neighbours (Prim algorithm)
```
.alg_type prim_list
.num_v 500
.density 0.1
.start_vertex 0
.out_matrix false
.out_list false
```
### Random graph represented by matrix (Prim algorithm)
```
.alg_type prim_matrix
.num_v 8
.density 0.5
.start_vertex 0
.out_matrix true
.out_list false
```
### Random graph represented by list of neighbours (Prim algorithm)
```
.alg_type kruskal_list
.num_v 300
.density 0.9
.start_vertex 0
.out_matrix false
.out_list false
```
### Random graph represented by matrix (Prim algorithm)
```
.alg_type kruskal_matrix
.num_v 8
.density 0.5
.start_vertex 0
.out_matrix true
.out_list false
```
# Possible configs for algorithms with input graphs
### config.txt
```
.alg_type dijkstra_list
.file_name graph_directed.txt
.start_vertex 0
.out_list true
.out_matrix false
```
### graph.txt
```
5 10
0 1 10
0 2 5
1 2 2
1 3 1
2 1 3
2 3 9
2 4 2
3 4 4
4 0 7
4 3 6
```
### config.txt
```
.alg_type belman_ford_list
.file_name graph_negative_weights.txt
.start_vertex 0
.out_list false
.out_matrix true
```
### graph.txt
```
4 5
0 1 4
0 2 5
1 3 3
2 1 -2
3 2 -1
```
### config.txt
```
.alg_type prim_list
.file_name graph_undirected.txt
.start_vertex 0
.out_list true
.out_matrix false
```
### graph.txt
```
5 7
0 1 2
0 3 6
1 2 3
1 3 8
1 4 5
2 4 7
3 4 9
```
### config.txt
```
.alg_type kruskal_list
.file_name graph_undirected.txt
.out_list false
.out_matrix true
```
### graph.txt
```
5 7
0 1 2
0 3 6
1 2 3
1 3 8
1 4 5
2 4 7
3 4 9
```