# Screenshot Capture Guide

This guide specifies exactly which examples to run and what to capture for each algorithm showcase.

## Traversal Algorithms

### 1. Depth-First Search (DFS)
**File to create:** `dfs_output.png`

**Example to run:** `graph_engine_demo.exe`
**Section to capture:** "Traversal Algorithms" → "DFS from vertex 1"
**Expected output:**
```
DFS from vertex 1:
Traversal order: 1 3 7 6 2 5 4
Has cycle: No
```

### 2. Breadth-First Search (BFS)
**File to create:** `bfs_output.png`

**Example to run:** `graph_engine_demo.exe`
**Section to capture:** "Traversal Algorithms" → "BFS from vertex 1"
**Expected output:**
```
BFS from vertex 1:
Traversal order: 1 2 3 4 5 6 7
Distances: (7:2) (6:2) (5:2) (4:2) (3:1) (2:1) (1:0)
```

## Shortest Path Algorithms

### 3. Dijkstra's Algorithm
**File to create:** `dijkstra_output.png`

**Example to run:** `graph_engine_demo.exe`
**Section to capture:** "Shortest Path Algorithms" → "Dijkstra's algorithm from vertex 1 to vertex 6"
**Expected output:**
```
Dijkstra's algorithm from vertex 1 to vertex 6:
No path exists
```

### 4. Bellman-Ford Algorithm
**File to create:** `bellman_ford_output.png`

**Example to run:** `route_planner.exe`
**Section to capture:** Look for Bellman-Ford usage in route planning
**Expected output:** Route planning results with negative weight handling

### 5. Floyd-Warshall Algorithm
**File to create:** `floyd_warshall_output.png`

**Example to run:** `graph_engine_demo.exe`
**Section to capture:** "Shortest Path Algorithms" → "All-pairs shortest distances (Floyd-Warshall)"
**Expected output:**
```
All-pairs shortest distances (Floyd-Warshall):
Distance from 1 to 2: 4
Distance from 1 to 3: 2
Distance from 1 to 4: 9
Distance from 1 to 5: 11
Distance from 1 to 6: 14
Distance from 2 to 3: 1
Distance from 2 to 4: 5
Distance from 2 to 5: 7
Distance from 2 to 6: 10
Distance from 3 to 4: 8
Distance from 3 to 5: 10
Distance from 3 to 6: 13
Distance from 4 to 5: 2
Distance from 4 to 6: 5
Distance from 5 to 6: 3
```

## Minimum Spanning Tree Algorithms

### 6. Kruskal's Algorithm
**File to create:** `kruskal_output.png`

**Example to run:** `graph_engine_demo.exe`
**Section to capture:** "Minimum Spanning Tree Algorithms" → "Kruskal's MST"
**Expected output:**
```
Kruskal's MST:
Total weight: 13
MST edges:
  2 -- 3 (weight: 1)
  3 -- 1 (weight: 2)
  4 -- 5 (weight: 2)
  5 -- 6 (weight: 3)
  4 -- 2 (weight: 5)
```

### 7. Prim's Algorithm
**File to create:** `prim_output.png`

**Example to run:** `graph_engine_demo.exe`
**Section to capture:** "Minimum Spanning Tree Algorithms" → "Prim's MST"
**Expected output:**
```
Prim's MST:
Total weight: 13
MST edges:
  6 -- 5 (weight: 3)
  5 -- 4 (weight: 2)
  4 -- 2 (weight: 5)
  2 -- 3 (weight: 1)
  3 -- 1 (weight: 2)
```

## Advanced Algorithms

### 8. Strongly Connected Components (SCC)
**File to create:** `scc_output.png`

**Example to run:** `dependency_resolver.exe`
**Section to capture:** Look for SCC analysis in dependency resolution
**Expected output:** Component analysis results

### 9. Maximum Flow (Ford-Fulkerson)
**File to create:** `maxflow_output.png`

**Example to run:** `graph_engine_demo.exe`
**Section to capture:** "Maximum Flow Algorithms" → "Ford-Fulkerson maximum flow from vertex 1 to vertex 6"
**Expected output:** Maximum flow calculation results

### 10. Graph Coloring
**File to create:** `coloring_output.png`

**Example to run:** `social_network.exe`
**Section to capture:** Look for graph coloring in social network analysis
**Expected output:** Color assignment results

## Network Analysis

### 11. Dependency Resolution
**File to create:** `dependency_output.png`

**Example to run:** `dependency_resolver.exe`
**Section to capture:** The complete output showing circular dependency detection
**Expected output:**
```
Error: Circular dependencies detected!
Dependency Resolver Demo
========================

=== Dependency Analysis ===
Total packages: 15
Total dependencies: 17

Circular dependencies detected:
  package-b
  package-c
  package-a

Orphaned packages (not depended upon):
  parser
  web-server
  database

Packages with most dependencies:
  web-server: 3 dependencies
  ssl: 2 dependencies
  logging: 2 dependencies
  database: 2 dependencies
  http-server: 2 dependencies
```

### 12. Social Network Analysis
**File to create:** `social_output.png`

**Example to run:** `social_network.exe`
**Section to capture:** The complete social network analysis output
**Expected output:** Friend recommendations, influence analysis, community detection

## Capture Instructions

### For Screenshots:
1. Run the specified example
2. Wait for complete output
3. Take a clean screenshot of the console output
4. Crop to show only the relevant section
5. Save as PNG format

### File Naming:
- Screenshots: `[algorithm]_output.png`
- Example: `dijkstra_output.png`

### Quality Guidelines:
- Screenshots: Minimum 1920x1080 resolution
- Ensure text is clearly readable
- Use consistent color scheme
- Add brief captions if needed
