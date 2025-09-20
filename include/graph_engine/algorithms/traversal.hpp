#pragma once

#include "../graph.hpp"
#include <vector>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <functional>

namespace graph_engine
{
    namespace algorithms
    {

        /**
         * @brief DFS traversal result structure
         */
        template <typename VertexType>
        struct DFSResult
        {
            std::vector<VertexType> traversal_order;
            std::unordered_map<VertexType, VertexType> parent_map;
            std::unordered_map<VertexType, int> discovery_time;
            std::unordered_map<VertexType, int> finish_time;
            std::unordered_set<VertexType> visited;
            bool has_cycle = false;
            std::vector<VertexType> cycle_path;

            DFSResult() = default;
        };

        /**
         * @brief BFS traversal result structure
         */
        template <typename VertexType>
        struct BFSResult
        {
            std::vector<VertexType> traversal_order;
            std::unordered_map<VertexType, VertexType> parent_map;
            std::unordered_map<VertexType, int> distance;
            std::unordered_set<VertexType> visited;

            BFSResult() = default;
        };

        /**
         * @brief Topological sort result
         */
        template <typename VertexType>
        struct TopologicalResult
        {
            std::vector<VertexType> sorted_order;
            bool is_dag = true;
            std::vector<VertexType> cycle_vertices;

            TopologicalResult() = default;
        };

        /**
         * @brief Depth-First Search (DFS) implementation
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to traverse
         * @param start_vertex Starting vertex for DFS
         * @param visitor Optional visitor function called for each vertex
         * @return DFSResult containing traversal information
         */
        template <typename VertexType, typename WeightType = double>
        DFSResult<VertexType> dfs(const Graph<VertexType, WeightType> &graph,
                                  const VertexType &start_vertex,
                                  std::function<void(const VertexType &)> visitor = nullptr)
        {
            DFSResult<VertexType> result;
            std::stack<VertexType> stack;
            std::unordered_set<VertexType> in_stack;
            int time = 0;

            // Initialize with start vertex
            stack.push(start_vertex);
            in_stack.insert(start_vertex);
            result.visited.insert(start_vertex);
            result.discovery_time[start_vertex] = time++;

            while (!stack.empty())
            {
                VertexType current = stack.top();
                stack.pop();
                in_stack.erase(current);

                if (visitor)
                {
                    visitor(current);
                }

                result.traversal_order.push_back(current);
                result.finish_time[current] = time++;

                // Get neighbors and add to stack
                auto neighbors = graph.get_neighbors(current);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;

                    if (result.visited.find(neighbor) == result.visited.end())
                    {
                        // Unvisited vertex
                        result.visited.insert(neighbor);
                        result.parent_map[neighbor] = current;
                        result.discovery_time[neighbor] = time++;
                        stack.push(neighbor);
                        in_stack.insert(neighbor);
                    }
                    else if (in_stack.find(neighbor) != in_stack.end())
                    {
                        // Back edge found - cycle detected
                        result.has_cycle = true;
                        if (result.cycle_path.empty())
                        {
                            // Build cycle path
                            result.cycle_path.push_back(neighbor);
                            VertexType path_vertex = current;
                            while (path_vertex != neighbor && !result.cycle_path.empty())
                            {
                                result.cycle_path.push_back(path_vertex);
                                auto parent_it = result.parent_map.find(path_vertex);
                                if (parent_it != result.parent_map.end())
                                {
                                    path_vertex = parent_it->second;
                                }
                                else
                                {
                                    result.cycle_path.clear();
                                    break;
                                }
                            }
                            if (!result.cycle_path.empty())
                            {
                                result.cycle_path.push_back(neighbor);
                                std::reverse(result.cycle_path.begin(), result.cycle_path.end());
                            }
                        }
                    }
                }
            }

            return result;
        }

        /**
         * @brief DFS for all vertices (handles disconnected components)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to traverse
         * @param visitor Optional visitor function called for each vertex
         * @return DFSResult containing traversal information for all components
         */
        template <typename VertexType, typename WeightType = double>
        DFSResult<VertexType> dfs_all(const Graph<VertexType, WeightType> &graph,
                                      std::function<void(const VertexType &)> visitor = nullptr)
        {
            DFSResult<VertexType> result;
            std::unordered_set<VertexType> all_visited;
            int time = 0;

            for (const auto &vertex : graph.get_vertices())
            {
                if (all_visited.find(vertex) == all_visited.end())
                {
                    // Start DFS from this unvisited vertex
                    std::stack<VertexType> stack;
                    std::unordered_set<VertexType> in_stack;

                    stack.push(vertex);
                    in_stack.insert(vertex);
                    result.visited.insert(vertex);
                    all_visited.insert(vertex);
                    result.discovery_time[vertex] = time++;

                    while (!stack.empty())
                    {
                        VertexType current = stack.top();
                        stack.pop();
                        in_stack.erase(current);

                        if (visitor)
                        {
                            visitor(current);
                        }

                        result.traversal_order.push_back(current);
                        result.finish_time[current] = time++;

                        auto neighbors = graph.get_neighbors(current);
                        for (const auto &neighbor_pair : neighbors)
                        {
                            const VertexType &neighbor = neighbor_pair.first;

                            if (all_visited.find(neighbor) == all_visited.end())
                            {
                                result.visited.insert(neighbor);
                                all_visited.insert(neighbor);
                                result.parent_map[neighbor] = current;
                                result.discovery_time[neighbor] = time++;
                                stack.push(neighbor);
                                in_stack.insert(neighbor);
                            }
                            else if (in_stack.find(neighbor) != in_stack.end())
                            {
                                result.has_cycle = true;
                                if (result.cycle_path.empty())
                                {
                                    result.cycle_path.push_back(neighbor);
                                    VertexType path_vertex = current;
                                    while (path_vertex != neighbor && !result.cycle_path.empty())
                                    {
                                        result.cycle_path.push_back(path_vertex);
                                        auto parent_it = result.parent_map.find(path_vertex);
                                        if (parent_it != result.parent_map.end())
                                        {
                                            path_vertex = parent_it->second;
                                        }
                                        else
                                        {
                                            result.cycle_path.clear();
                                            break;
                                        }
                                    }
                                    if (!result.cycle_path.empty())
                                    {
                                        result.cycle_path.push_back(neighbor);
                                        std::reverse(result.cycle_path.begin(), result.cycle_path.end());
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return result;
        }

        /**
         * @brief Breadth-First Search (BFS) implementation
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to traverse
         * @param start_vertex Starting vertex for BFS
         * @param visitor Optional visitor function called for each vertex
         * @return BFSResult containing traversal information
         */
        template <typename VertexType, typename WeightType = double>
        BFSResult<VertexType> bfs(const Graph<VertexType, WeightType> &graph,
                                  const VertexType &start_vertex,
                                  std::function<void(const VertexType &)> visitor = nullptr)
        {
            BFSResult<VertexType> result;
            std::queue<VertexType> queue;

            // Initialize with start vertex
            queue.push(start_vertex);
            result.visited.insert(start_vertex);
            result.distance[start_vertex] = 0;

            while (!queue.empty())
            {
                VertexType current = queue.front();
                queue.pop();

                if (visitor)
                {
                    visitor(current);
                }

                result.traversal_order.push_back(current);

                // Process neighbors
                auto neighbors = graph.get_neighbors(current);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;

                    if (result.visited.find(neighbor) == result.visited.end())
                    {
                        result.visited.insert(neighbor);
                        result.parent_map[neighbor] = current;
                        result.distance[neighbor] = result.distance[current] + 1;
                        queue.push(neighbor);
                    }
                }
            }

            return result;
        }

        /**
         * @brief BFS for all vertices (handles disconnected components)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to traverse
         * @param visitor Optional visitor function called for each vertex
         * @return BFSResult containing traversal information for all components
         */
        template <typename VertexType, typename WeightType = double>
        BFSResult<VertexType> bfs_all(const Graph<VertexType, WeightType> &graph,
                                      std::function<void(const VertexType &)> visitor = nullptr)
        {
            BFSResult<VertexType> result;
            std::unordered_set<VertexType> all_visited;

            for (const auto &vertex : graph.get_vertices())
            {
                if (all_visited.find(vertex) == all_visited.end())
                {
                    std::queue<VertexType> queue;

                    queue.push(vertex);
                    result.visited.insert(vertex);
                    all_visited.insert(vertex);
                    result.distance[vertex] = 0;

                    while (!queue.empty())
                    {
                        VertexType current = queue.front();
                        queue.pop();

                        if (visitor)
                        {
                            visitor(current);
                        }

                        result.traversal_order.push_back(current);

                        auto neighbors = graph.get_neighbors(current);
                        for (const auto &neighbor_pair : neighbors)
                        {
                            const VertexType &neighbor = neighbor_pair.first;

                            if (all_visited.find(neighbor) == all_visited.end())
                            {
                                result.visited.insert(neighbor);
                                all_visited.insert(neighbor);
                                result.parent_map[neighbor] = current;
                                result.distance[neighbor] = result.distance[current] + 1;
                                queue.push(neighbor);
                            }
                        }
                    }
                }
            }

            return result;
        }

        /**
         * @brief Check if graph has cycles using DFS
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to check
         * @return true if graph has cycles
         */
        template <typename VertexType, typename WeightType = double>
        bool has_cycle(const Graph<VertexType, WeightType> &graph)
        {
            if (graph.get_direction() == GraphDirection::UNDIRECTED)
            {
                // For undirected graphs, use Union-Find or simple DFS
                return dfs_all(graph).has_cycle;
            }
            else
            {
                // For directed graphs, use DFS with three colors
                std::unordered_map<VertexType, int> color; // 0: white, 1: gray, 2: black

                std::function<bool(const VertexType &)> dfs_visit = [&](const VertexType &vertex) -> bool
                {
                    color[vertex] = 1; // gray

                    auto neighbors = graph.get_neighbors(vertex);
                    for (const auto &neighbor_pair : neighbors)
                    {
                        const VertexType &neighbor = neighbor_pair.first;

                        if (color[neighbor] == 1)
                        { // gray - back edge
                            return true;
                        }
                        else if (color[neighbor] == 0 && dfs_visit(neighbor))
                        { // white
                            return true;
                        }
                    }

                    color[vertex] = 2; // black
                    return false;
                };

                for (const auto &vertex : graph.get_vertices())
                {
                    if (color[vertex] == 0)
                    { // white
                        if (dfs_visit(vertex))
                        {
                            return true;
                        }
                    }
                }

                return false;
            }
        }

        /**
         * @brief Topological sort using DFS (Kahn's algorithm alternative)
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to sort (must be DAG)
         * @return TopologicalResult containing sorted order or cycle information
         */
        template <typename VertexType, typename WeightType = double>
        TopologicalResult<VertexType> topological_sort(const Graph<VertexType, WeightType> &graph)
        {
            TopologicalResult<VertexType> result;

            if (graph.get_direction() != GraphDirection::DIRECTED)
            {
                result.is_dag = false;
                return result;
            }

            std::unordered_map<VertexType, int> in_degree;
            std::queue<VertexType> zero_in_degree;

            // Calculate in-degrees
            for (const auto &vertex : graph.get_vertices())
            {
                in_degree[vertex] = 0;
            }

            for (const auto &edge : graph.get_edges())
            {
                in_degree[edge.to]++;
            }

            // Find vertices with zero in-degree
            for (const auto &pair : in_degree)
            {
                if (pair.second == 0)
                {
                    zero_in_degree.push(pair.first);
                }
            }

            // Process vertices
            while (!zero_in_degree.empty())
            {
                VertexType current = zero_in_degree.front();
                zero_in_degree.pop();
                result.sorted_order.push_back(current);

                // Reduce in-degree of neighbors
                auto neighbors = graph.get_neighbors(current);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    in_degree[neighbor]--;

                    if (in_degree[neighbor] == 0)
                    {
                        zero_in_degree.push(neighbor);
                    }
                }
            }

            // Check if all vertices were processed
            if (result.sorted_order.size() != graph.num_vertices())
            {
                result.is_dag = false;
                // Find remaining vertices (part of cycle)
                for (const auto &pair : in_degree)
                {
                    if (pair.second > 0)
                    {
                        result.cycle_vertices.push_back(pair.first);
                    }
                }
            }

            return result;
        }

        /**
         * @brief Find shortest path in unweighted graph using BFS
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to search
         * @param start_vertex Starting vertex
         * @param end_vertex Target vertex
         * @return Vector of vertices representing the shortest path, empty if no path exists
         */
        template <typename VertexType, typename WeightType = double>
        std::vector<VertexType> shortest_path_unweighted(const Graph<VertexType, WeightType> &graph,
                                                         const VertexType &start_vertex,
                                                         const VertexType &end_vertex)
        {
            if (start_vertex == end_vertex)
            {
                return {start_vertex};
            }

            BFSResult<VertexType> bfs_result = bfs(graph, start_vertex);

            if (bfs_result.visited.find(end_vertex) == bfs_result.visited.end())
            {
                return {}; // No path exists
            }

            // Reconstruct path
            std::vector<VertexType> path;
            VertexType current = end_vertex;

            while (current != start_vertex)
            {
                path.push_back(current);
                auto parent_it = bfs_result.parent_map.find(current);
                if (parent_it != bfs_result.parent_map.end())
                {
                    current = parent_it->second;
                }
                else
                {
                    return {}; // Should not happen
                }
            }

            path.push_back(start_vertex);
            std::reverse(path.begin(), path.end());

            return path;
        }

        /**
         * @brief Find all connected components in undirected graph
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to analyze
         * @return Vector of vectors, each containing vertices in a connected component
         */
        template <typename VertexType, typename WeightType = double>
        std::vector<std::vector<VertexType>> connected_components(const Graph<VertexType, WeightType> &graph)
        {
            std::vector<std::vector<VertexType>> components;
            std::unordered_set<VertexType> visited;

            for (const auto &vertex : graph.get_vertices())
            {
                if (visited.find(vertex) == visited.end())
                {
                    // Start BFS from this unvisited vertex
                    std::vector<VertexType> component;
                    std::queue<VertexType> queue;

                    queue.push(vertex);
                    visited.insert(vertex);

                    while (!queue.empty())
                    {
                        VertexType current = queue.front();
                        queue.pop();
                        component.push_back(current);

                        auto neighbors = graph.get_neighbors(current);
                        for (const auto &neighbor_pair : neighbors)
                        {
                            const VertexType &neighbor = neighbor_pair.first;

                            if (visited.find(neighbor) == visited.end())
                            {
                                visited.insert(neighbor);
                                queue.push(neighbor);
                            }
                        }
                    }

                    components.push_back(component);
                }
            }

            return components;
        }

    } // namespace algorithms
} // namespace graph_engine
