#pragma once

#include "../graph.hpp"
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>

namespace graph_engine
{
    namespace algorithms
    {

        /**
         * @brief Strongly Connected Components result structure
         */
        template <typename VertexType>
        struct SCCResult
        {
            std::vector<std::vector<VertexType>> components;
            std::unordered_map<VertexType, std::size_t> component_id;
            std::size_t num_components = 0;

            SCCResult() = default;
        };

        /**
         * @brief Graph coloring result structure
         */
        template <typename VertexType>
        struct ColoringResult
        {
            std::unordered_map<VertexType, int> colors;
            int num_colors_used = 0;
            bool is_valid_coloring = true;

            ColoringResult() = default;
        };

        /**
         * @brief Kosaraju's algorithm for finding Strongly Connected Components
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * Uses two DFS passes: first on the original graph to get finish times,
         * then on the transpose graph in reverse finish time order.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The directed graph to analyze
         * @return SCCResult containing all strongly connected components
         */
        template <typename VertexType, typename WeightType = double>
        SCCResult<VertexType> kosaraju_scc(const Graph<VertexType, WeightType> &graph)
        {
            SCCResult<VertexType> result;

            if (graph.empty())
            {
                return result;
            }

            // First DFS pass: compute finish times
            std::stack<VertexType> finish_stack;
            std::unordered_set<VertexType> visited;

            std::function<void(const VertexType &)> dfs_visit = [&](const VertexType &vertex)
            {
                visited.insert(vertex);

                auto neighbors = graph.get_neighbors(vertex);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    if (visited.find(neighbor) == visited.end())
                    {
                        dfs_visit(neighbor);
                    }
                }

                finish_stack.push(vertex);
            };

            // Perform DFS on all unvisited vertices
            for (const auto &vertex : graph.get_vertices())
            {
                if (visited.find(vertex) == visited.end())
                {
                    dfs_visit(vertex);
                }
            }

            // Create transpose graph
            Graph<VertexType, WeightType> transpose_graph(GraphDirection::DIRECTED);

            // Add all vertices
            for (const auto &vertex : graph.get_vertices())
            {
                transpose_graph.add_vertex(vertex);
            }

            // Add reversed edges
            for (const auto &edge : graph.get_edges())
            {
                transpose_graph.add_edge(edge.to, edge.from, edge.weight);
            }

            // Second DFS pass: process vertices in reverse finish time order
            visited.clear();

            std::function<void(const VertexType &, std::vector<VertexType> &)> dfs_visit_transpose =
                [&](const VertexType &vertex, std::vector<VertexType> &component)
            {
                visited.insert(vertex);
                component.push_back(vertex);

                auto neighbors = transpose_graph.get_neighbors(vertex);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    if (visited.find(neighbor) == visited.end())
                    {
                        dfs_visit_transpose(neighbor, component);
                    }
                }
            };

            // Process vertices in reverse finish time order
            while (!finish_stack.empty())
            {
                VertexType vertex = finish_stack.top();
                finish_stack.pop();

                if (visited.find(vertex) == visited.end())
                {
                    std::vector<VertexType> component;
                    dfs_visit_transpose(vertex, component);
                    result.components.push_back(component);

                    // Assign component IDs
                    for (const auto &v : component)
                    {
                        result.component_id[v] = result.num_components;
                    }
                    result.num_components++;
                }
            }

            return result;
        }

        /**
         * @brief Tarjan's algorithm for finding Strongly Connected Components
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * Uses a single DFS pass with low-link values to identify SCCs.
         * More memory efficient than Kosaraju's algorithm.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The directed graph to analyze
         * @return SCCResult containing all strongly connected components
         */
        template <typename VertexType, typename WeightType = double>
        SCCResult<VertexType> tarjan_scc(const Graph<VertexType, WeightType> &graph)
        {
            SCCResult<VertexType> result;

            if (graph.empty())
            {
                return result;
            }

            std::unordered_map<VertexType, int> indices;
            std::unordered_map<VertexType, int> low_links;
            std::stack<VertexType> stack;
            std::unordered_set<VertexType> on_stack;
            int index = 0;

            std::function<void(const VertexType &)> strongconnect = [&](const VertexType &vertex)
            {
                indices[vertex] = index;
                low_links[vertex] = index;
                index++;
                stack.push(vertex);
                on_stack.insert(vertex);

                auto neighbors = graph.get_neighbors(vertex);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;

                    if (indices.find(neighbor) == indices.end())
                    {
                        // Neighbor has not been visited yet
                        strongconnect(neighbor);
                        low_links[vertex] = std::min(low_links[vertex], low_links[neighbor]);
                    }
                    else if (on_stack.find(neighbor) != on_stack.end())
                    {
                        // Neighbor is in stack and hence in the current SCC
                        low_links[vertex] = std::min(low_links[vertex], indices[neighbor]);
                    }
                }

                // If vertex is a root node, pop the stack and create an SCC
                if (low_links[vertex] == indices[vertex])
                {
                    std::vector<VertexType> component;
                    VertexType w;

                    do
                    {
                        w = stack.top();
                        stack.pop();
                        on_stack.erase(w);
                        component.push_back(w);
                        result.component_id[w] = result.num_components;
                    } while (w != vertex);

                    result.components.push_back(component);
                    result.num_components++;
                }
            };

            // Perform Tarjan's algorithm on all unvisited vertices
            for (const auto &vertex : graph.get_vertices())
            {
                if (indices.find(vertex) == indices.end())
                {
                    strongconnect(vertex);
                }
            }

            return result;
        }

        /**
         * @brief Greedy graph coloring algorithm
         *
         * Time Complexity: O(V² + VE) in worst case
         * Space Complexity: O(V)
         *
         * Uses a greedy approach to color vertices. Not optimal but simple and fast.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to color
         * @param max_colors Maximum number of colors to use (0 for unlimited)
         * @return ColoringResult containing vertex colors and validity
         */
        template <typename VertexType, typename WeightType = double>
        ColoringResult<VertexType> greedy_coloring(const Graph<VertexType, WeightType> &graph,
                                                   int max_colors = 0)
        {
            ColoringResult<VertexType> result;

            if (graph.empty())
            {
                return result;
            }

            // Initialize all vertices with no color
            for (const auto &vertex : graph.get_vertices())
            {
                result.colors[vertex] = -1; // -1 means no color assigned
            }

            // Color each vertex
            for (const auto &vertex : graph.get_vertices())
            {
                // Find colors used by neighbors
                std::unordered_set<int> used_colors;
                auto neighbors = graph.get_neighbors(vertex);

                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    int neighbor_color = result.colors[neighbor];
                    if (neighbor_color != -1)
                    {
                        used_colors.insert(neighbor_color);
                    }
                }

                // Find the smallest available color
                int color = 0;
                while (used_colors.find(color) != used_colors.end())
                {
                    color++;
                }

                // Check if we exceed max_colors limit
                if (max_colors > 0 && color >= max_colors)
                {
                    result.is_valid_coloring = false;
                    return result;
                }

                result.colors[vertex] = color;
                result.num_colors_used = std::max(result.num_colors_used, color + 1);
            }

            return result;
        }

        /**
         * @brief Welsh-Powell graph coloring algorithm
         *
         * Time Complexity: O(V² log V)
         * Space Complexity: O(V)
         *
         * Orders vertices by degree and uses greedy coloring. Often produces
         * better results than simple greedy coloring.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to color
         * @param max_colors Maximum number of colors to use (0 for unlimited)
         * @return ColoringResult containing vertex colors and validity
         */
        template <typename VertexType, typename WeightType = double>
        ColoringResult<VertexType> welsh_powell_coloring(const Graph<VertexType, WeightType> &graph,
                                                         int max_colors = 0)
        {
            ColoringResult<VertexType> result;

            if (graph.empty())
            {
                return result;
            }

            // Calculate degrees for all vertices
            std::vector<std::pair<VertexType, int>> vertices_with_degrees;
            for (const auto &vertex : graph.get_vertices())
            {
                int degree = static_cast<int>(graph.get_neighbors(vertex).size());
                vertices_with_degrees.push_back({vertex, degree});
            }

            // Sort vertices by degree in descending order
            std::sort(vertices_with_degrees.begin(), vertices_with_degrees.end(),
                      [](const auto &a, const auto &b)
                      {
                          return a.second > b.second;
                      });

            // Initialize all vertices with no color
            for (const auto &vertex : graph.get_vertices())
            {
                result.colors[vertex] = -1;
            }

            // Color vertices in order of decreasing degree
            for (const auto &vertex_degree : vertices_with_degrees)
            {
                const VertexType &vertex = vertex_degree.first;

                if (result.colors[vertex] != -1)
                {
                    continue; // Already colored
                }

                // Find colors used by neighbors
                std::unordered_set<int> used_colors;
                auto neighbors = graph.get_neighbors(vertex);

                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    int neighbor_color = result.colors[neighbor];
                    if (neighbor_color != -1)
                    {
                        used_colors.insert(neighbor_color);
                    }
                }

                // Find the smallest available color
                int color = 0;
                while (used_colors.find(color) != used_colors.end())
                {
                    color++;
                }

                // Check if we exceed max_colors limit
                if (max_colors > 0 && color >= max_colors)
                {
                    result.is_valid_coloring = false;
                    return result;
                }

                result.colors[vertex] = color;
                result.num_colors_used = std::max(result.num_colors_used, color + 1);
            }

            return result;
        }

        /**
         * @brief Check if a graph is bipartite using BFS
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * A graph is bipartite if it can be colored with 2 colors such that
         * no two adjacent vertices have the same color.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to check
         * @return Pair of (is_bipartite, coloring_result)
         */
        template <typename VertexType, typename WeightType = double>
        std::pair<bool, ColoringResult<VertexType>> is_bipartite(const Graph<VertexType, WeightType> &graph)
        {
            ColoringResult<VertexType> result;

            if (graph.empty())
            {
                return {true, result};
            }

            // Initialize all vertices with no color
            for (const auto &vertex : graph.get_vertices())
            {
                result.colors[vertex] = -1;
            }

            // Use BFS to check bipartiteness
            std::queue<VertexType> queue;

            for (const auto &vertex : graph.get_vertices())
            {
                if (result.colors[vertex] == -1)
                {
                    // Start BFS from this uncolored vertex
                    result.colors[vertex] = 0;
                    queue.push(vertex);

                    while (!queue.empty())
                    {
                        VertexType current = queue.front();
                        queue.pop();

                        auto neighbors = graph.get_neighbors(current);
                        for (const auto &neighbor_pair : neighbors)
                        {
                            const VertexType &neighbor = neighbor_pair.first;

                            if (result.colors[neighbor] == -1)
                            {
                                // Color neighbor with opposite color
                                result.colors[neighbor] = 1 - result.colors[current];
                                queue.push(neighbor);
                            }
                            else if (result.colors[neighbor] == result.colors[current])
                            {
                                // Same color as current vertex - not bipartite
                                result.is_valid_coloring = false;
                                return {false, result};
                            }
                        }
                    }
                }
            }

            result.is_valid_coloring = true;
            result.num_colors_used = 2;
            return {true, result};
        }

        /**
         * @brief Find the chromatic number of a graph (minimum number of colors needed)
         *
         * Time Complexity: O(V^V) in worst case (exponential)
         * Space Complexity: O(V)
         *
         * This is a simplified implementation. For large graphs, use more
         * sophisticated algorithms or heuristics.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to analyze
         * @return The chromatic number
         */
        template <typename VertexType, typename WeightType = double>
        int chromatic_number(const Graph<VertexType, WeightType> &graph)
        {
            if (graph.empty())
            {
                return 0;
            }

            // Check if graph is bipartite
            auto [is_bip, _] = is_bipartite(graph);
            if (is_bip)
            {
                return 2;
            }

            // Try different numbers of colors
            for (int num_colors = 1; num_colors <= static_cast<int>(graph.num_vertices()); ++num_colors)
            {
                auto coloring_result = greedy_coloring(graph, num_colors);
                if (coloring_result.is_valid_coloring)
                {
                    return num_colors;
                }
            }

            return static_cast<int>(graph.num_vertices()); // Fallback
        }

        /**
         * @brief Find all articulation points (cut vertices) in a graph
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * An articulation point is a vertex whose removal increases the number
         * of connected components in the graph.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to analyze
         * @return Set of articulation points
         */
        template <typename VertexType, typename WeightType = double>
        std::unordered_set<VertexType> find_articulation_points(const Graph<VertexType, WeightType> &graph)
        {
            std::unordered_set<VertexType> articulation_points;

            if (graph.empty())
            {
                return articulation_points;
            }

            std::unordered_map<VertexType, int> discovery_time;
            std::unordered_map<VertexType, int> low_time;
            std::unordered_map<VertexType, VertexType> parent;
            std::unordered_set<VertexType> visited;
            int time = 0;

            std::function<void(const VertexType &)> dfs_articulation = [&](const VertexType &vertex)
            {
                visited.insert(vertex);
                discovery_time[vertex] = low_time[vertex] = ++time;
                int children = 0;

                auto neighbors = graph.get_neighbors(vertex);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;

                    if (visited.find(neighbor) == visited.end())
                    {
                        children++;
                        parent[neighbor] = vertex;
                        dfs_articulation(neighbor);

                        // Check if the subtree rooted with neighbor has a connection
                        // to one of the ancestors of vertex
                        low_time[vertex] = std::min(low_time[vertex], low_time[neighbor]);

                        // vertex is an articulation point if:
                        // 1. It is the root and has more than one child
                        // 2. It is not the root and low_time of one of its children is more than discovery_time of vertex
                        if (parent.find(vertex) == parent.end() && children > 1)
                        {
                            articulation_points.insert(vertex);
                        }
                        else if (parent.find(vertex) != parent.end() &&
                                 low_time[neighbor] >= discovery_time[vertex])
                        {
                            articulation_points.insert(vertex);
                        }
                    }
                    else if (neighbor != parent[vertex])
                    {
                        // Update low_time value of vertex for parent function calls
                        low_time[vertex] = std::min(low_time[vertex], discovery_time[neighbor]);
                    }
                }
            };

            // Find articulation points in all connected components
            for (const auto &vertex : graph.get_vertices())
            {
                if (visited.find(vertex) == visited.end())
                {
                    dfs_articulation(vertex);
                }
            }

            return articulation_points;
        }

        /**
         * @brief Find all bridges (cut edges) in a graph
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * A bridge is an edge whose removal increases the number of
         * connected components in the graph.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to analyze
         * @return Vector of bridges
         */
        template <typename VertexType, typename WeightType = double>
        std::vector<Edge<VertexType, WeightType>> find_bridges(const Graph<VertexType, WeightType> &graph)
        {
            std::vector<Edge<VertexType, WeightType>> bridges;

            if (graph.empty())
            {
                return bridges;
            }

            std::unordered_map<VertexType, int> discovery_time;
            std::unordered_map<VertexType, int> low_time;
            std::unordered_map<VertexType, VertexType> parent;
            std::unordered_set<VertexType> visited;
            int time = 0;

            std::function<void(const VertexType &)> dfs_bridge = [&](const VertexType &vertex)
            {
                visited.insert(vertex);
                discovery_time[vertex] = low_time[vertex] = ++time;

                auto neighbors = graph.get_neighbors(vertex);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    WeightType weight = neighbor_pair.second;

                    if (visited.find(neighbor) == visited.end())
                    {
                        parent[neighbor] = vertex;
                        dfs_bridge(neighbor);

                        // Check if the subtree rooted with neighbor has a connection
                        // to one of the ancestors of vertex
                        low_time[vertex] = std::min(low_time[vertex], low_time[neighbor]);

                        // If the lowest vertex reachable from subtree under neighbor
                        // is below vertex in DFS tree, then vertex-neighbor is a bridge
                        if (low_time[neighbor] > discovery_time[vertex])
                        {
                            bridges.emplace_back(vertex, neighbor, weight);
                        }
                    }
                    else if (neighbor != parent[vertex])
                    {
                        // Update low_time value of vertex for parent function calls
                        low_time[vertex] = std::min(low_time[vertex], discovery_time[neighbor]);
                    }
                }
            };

            // Find bridges in all connected components
            for (const auto &vertex : graph.get_vertices())
            {
                if (visited.find(vertex) == visited.end())
                {
                    dfs_bridge(vertex);
                }
            }

            return bridges;
        }

    } // namespace algorithms
} // namespace graph_engine
