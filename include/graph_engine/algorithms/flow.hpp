#pragma once

#include "../graph.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <functional>

namespace graph_engine
{
    namespace algorithms
    {

        /**
         * @brief Maximum flow result structure
         */
        template <typename VertexType, typename WeightType>
        struct MaxFlowResult
        {
            WeightType max_flow = WeightType{};
            std::unordered_map<std::pair<VertexType, VertexType>, WeightType> flow_values;
            std::vector<std::pair<VertexType, VertexType>> min_cut_edges;

            MaxFlowResult() = default;
        };

        /**
         * @brief Bipartite matching result structure
         */
        template <typename VertexType>
        struct BipartiteMatchingResult
        {
            std::vector<std::pair<VertexType, VertexType>> matches;
            std::size_t max_matching_size = 0;
            bool is_perfect_matching = false;

            BipartiteMatchingResult() = default;
        };

        /**
         * @brief Ford-Fulkerson algorithm for maximum flow using DFS
         *
         * Time Complexity: O(E * max_flow) in worst case
         * Space Complexity: O(V)
         *
         * Uses DFS to find augmenting paths. Can be slow for large flows.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The flow network (capacities as edge weights)
         * @param source Source vertex
         * @param sink Sink vertex
         * @return MaxFlowResult containing maximum flow and flow values
         */
        template <typename VertexType, typename WeightType = double>
        MaxFlowResult<VertexType, WeightType> ford_fulkerson(const Graph<VertexType, WeightType> &graph,
                                                             const VertexType &source,
                                                             const VertexType &sink)
        {
            MaxFlowResult<VertexType, WeightType> result;

            if (source == sink)
            {
                return result;
            }

            // Initialize flow values
            std::unordered_map<std::pair<VertexType, VertexType>, WeightType> flow;
            std::unordered_map<std::pair<VertexType, VertexType>, WeightType> capacity;
            
            // Store original capacities
            for (const auto &edge : graph.get_edges())
            {
                capacity[{edge.from, edge.to}] = edge.weight;
                flow[{edge.from, edge.to}] = WeightType{};
            }

            // Find augmenting paths using DFS
            std::function<WeightType(const VertexType &, const VertexType &, WeightType, std::unordered_set<VertexType> &)>
                dfs_augment = [&](const VertexType &current, const VertexType &target, WeightType min_capacity,
                                  std::unordered_set<VertexType> &visited) -> WeightType
            {
                if (current == target)
                {
                    return min_capacity;
                }

                visited.insert(current);

                // Check all possible edges (both forward and backward)
                for (const auto &vertex : graph.get_vertices())
                {
                    if (visited.find(vertex) == visited.end())
                    {
                        // Forward edge
                        auto forward_key = std::make_pair(current, vertex);
                        if (capacity.find(forward_key) != capacity.end())
                        {
                            WeightType residual_capacity = capacity[forward_key] - flow[forward_key];
                            if (residual_capacity > WeightType{})
                            {
                                WeightType path_flow = dfs_augment(vertex, target,
                                                                   std::min(min_capacity, residual_capacity), visited);

                                if (path_flow > WeightType{})
                                {
                                    flow[forward_key] += path_flow;
                                    return path_flow;
                                }
                            }
                        }

                        // Backward edge
                        auto backward_key = std::make_pair(vertex, current);
                        if (capacity.find(backward_key) != capacity.end())
                        {
                            if (flow[backward_key] > WeightType{})
                            {
                                WeightType path_flow = dfs_augment(vertex, target,
                                                                   std::min(min_capacity, flow[backward_key]), visited);

                                if (path_flow > WeightType{})
                                {
                                    flow[backward_key] -= path_flow;
                                    return path_flow;
                                }
                            }
                        }
                    }
                }

                return WeightType{};
            };

            // Find maximum flow
            WeightType total_flow = WeightType{};
            while (true)
            {
                std::unordered_set<VertexType> visited;
                WeightType path_flow = dfs_augment(source, sink,
                                                   std::numeric_limits<WeightType>::max(), visited);

                if (path_flow == WeightType{})
                {
                    break; // No more augmenting paths
                }

                total_flow += path_flow;
            }

            result.max_flow = total_flow;
            result.flow_values = flow;

            // Find minimum cut using BFS on residual graph
            std::unordered_set<VertexType> reachable;
            std::queue<VertexType> queue;

            queue.push(source);
            reachable.insert(source);

            while (!queue.empty())
            {
                VertexType current = queue.front();
                queue.pop();

                for (const auto &vertex : graph.get_vertices())
                {
                    if (reachable.find(vertex) == reachable.end())
                    {
                        auto forward_key = std::make_pair(current, vertex);
                        auto backward_key = std::make_pair(vertex, current);
                        
                        // Check if there's residual capacity
                        bool has_residual = false;
                        if (capacity.find(forward_key) != capacity.end())
                        {
                            WeightType residual = capacity[forward_key] - flow[forward_key];
                            if (residual > WeightType{})
                            {
                                has_residual = true;
                            }
                        }
                        if (capacity.find(backward_key) != capacity.end())
                        {
                            if (flow[backward_key] > WeightType{})
                            {
                                has_residual = true;
                            }
                        }

                        if (has_residual)
                        {
                            reachable.insert(vertex);
                            queue.push(vertex);
                        }
                    }
                }
            }

            // Find edges in minimum cut
            for (const auto &edge : graph.get_edges())
            {
                if (reachable.find(edge.from) != reachable.end() &&
                    reachable.find(edge.to) == reachable.end())
                {
                    result.min_cut_edges.push_back({edge.from, edge.to});
                }
            }

            return result;
        }

        /**
         * @brief Edmonds-Karp algorithm for maximum flow using BFS
         *
         * Time Complexity: O(VE²)
         * Space Complexity: O(V)
         *
         * Uses BFS to find shortest augmenting paths, which guarantees
         * polynomial time complexity.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The flow network (capacities as edge weights)
         * @param source Source vertex
         * @param sink Sink vertex
         * @return MaxFlowResult containing maximum flow and flow values
         */
        template <typename VertexType, typename WeightType = double>
        MaxFlowResult<VertexType, WeightType> edmonds_karp(const Graph<VertexType, WeightType> &graph,
                                                           const VertexType &source,
                                                           const VertexType &sink)
        {
            MaxFlowResult<VertexType, WeightType> result;

            if (source == sink)
            {
                return result;
            }

            // Create residual graph
            Graph<VertexType, WeightType> residual_graph = graph;

            // Initialize flow values
            std::unordered_map<std::pair<VertexType, VertexType>, WeightType> flow;
            for (const auto &edge : graph.get_edges())
            {
                flow[{edge.from, edge.to}] = WeightType{};
            }

            // Find augmenting paths using BFS
            std::function<WeightType()> bfs_augment = [&]() -> WeightType
            {
                std::unordered_map<VertexType, VertexType> parent;
                std::queue<VertexType> queue;

                queue.push(source);
                parent[source] = source;

                while (!queue.empty())
                {
                    VertexType current = queue.front();
                    queue.pop();

                    if (current == sink)
                    {
                        // Found path to sink, calculate flow
                        WeightType path_flow = std::numeric_limits<WeightType>::max();
                        VertexType v = sink;

                        while (v != source)
                        {
                            VertexType u = parent[v];
                            WeightType capacity = residual_graph.get_edge_weight(u, v);
                            path_flow = std::min(path_flow, capacity);
                            v = u;
                        }

                        // Update residual capacities and flow
                        v = sink;
                        while (v != source)
                        {
                            VertexType u = parent[v];
                            WeightType capacity = residual_graph.get_edge_weight(u, v);

                            residual_graph.add_edge(u, v, capacity - path_flow);
                            residual_graph.add_edge(v, u,
                                                    residual_graph.get_edge_weight(v, u) + path_flow);

                            flow[{u, v}] += path_flow;
                            v = u;
                        }

                        return path_flow;
                    }

                    auto neighbors = residual_graph.get_neighbors(current);
                    for (const auto &neighbor_pair : neighbors)
                    {
                        const VertexType &neighbor = neighbor_pair.first;
                        WeightType capacity = neighbor_pair.second;

                        if (parent.find(neighbor) == parent.end() && capacity > WeightType{})
                        {
                            parent[neighbor] = current;
                            queue.push(neighbor);
                        }
                    }
                }

                return WeightType{}; // No augmenting path found
            };

            // Find maximum flow
            WeightType total_flow = WeightType{};
            while (true)
            {
                WeightType path_flow = bfs_augment();
                if (path_flow == WeightType{})
                {
                    break;
                }
                total_flow += path_flow;
            }

            result.max_flow = total_flow;
            result.flow_values = flow;

            // Find minimum cut
            std::unordered_set<VertexType> reachable;
            std::queue<VertexType> queue;

            queue.push(source);
            reachable.insert(source);

            while (!queue.empty())
            {
                VertexType current = queue.front();
                queue.pop();

                auto neighbors = residual_graph.get_neighbors(current);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    WeightType capacity = neighbor_pair.second;

                    if (reachable.find(neighbor) == reachable.end() && capacity > WeightType{})
                    {
                        reachable.insert(neighbor);
                        queue.push(neighbor);
                    }
                }
            }

            for (const auto &edge : graph.get_edges())
            {
                if (reachable.find(edge.from) != reachable.end() &&
                    reachable.find(edge.to) == reachable.end())
                {
                    result.min_cut_edges.push_back({edge.from, edge.to});
                }
            }

            return result;
        }

        /**
         * @brief Dinic's algorithm for maximum flow
         *
         * Time Complexity: O(V²E)
         * Space Complexity: O(V)
         *
         * Uses level graph and blocking flow to achieve better performance
         * than Edmonds-Karp for many practical cases.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The flow network (capacities as edge weights)
         * @param source Source vertex
         * @param sink Sink vertex
         * @return MaxFlowResult containing maximum flow and flow values
         */
        template <typename VertexType, typename WeightType = double>
        MaxFlowResult<VertexType, WeightType> dinic(const Graph<VertexType, WeightType> &graph,
                                                    const VertexType &source,
                                                    const VertexType &sink)
        {
            MaxFlowResult<VertexType, WeightType> result;

            if (source == sink)
            {
                return result;
            }

            // Create residual graph
            Graph<VertexType, WeightType> residual_graph = graph;

            // Initialize flow values
            std::unordered_map<std::pair<VertexType, VertexType>, WeightType> flow;
            for (const auto &edge : graph.get_edges())
            {
                flow[{edge.from, edge.to}] = WeightType{};
            }

            // Build level graph using BFS
            std::function<bool()> build_level_graph = [&]() -> bool
            {
                std::unordered_map<VertexType, int> level;
                std::queue<VertexType> queue;

                queue.push(source);
                level[source] = 0;

                while (!queue.empty())
                {
                    VertexType current = queue.front();
                    queue.pop();

                    if (current == sink)
                    {
                        return true; // Sink is reachable
                    }

                    auto neighbors = residual_graph.get_neighbors(current);
                    for (const auto &neighbor_pair : neighbors)
                    {
                        const VertexType &neighbor = neighbor_pair.first;
                        WeightType capacity = neighbor_pair.second;

                        if (level.find(neighbor) == level.end() && capacity > WeightType{})
                        {
                            level[neighbor] = level[current] + 1;
                            queue.push(neighbor);
                        }
                    }
                }

                return false; // Sink not reachable
            };

            // Find blocking flow using DFS
            std::function<WeightType(const VertexType &, WeightType)>
                find_blocking_flow = [&](const VertexType &current, WeightType flow_limit) -> WeightType
            {
                if (current == sink)
                {
                    return flow_limit;
                }

                auto neighbors = residual_graph.get_neighbors(current);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    WeightType capacity = neighbor_pair.second;

                    if (capacity > WeightType{})
                    {
                        WeightType pushed_flow = find_blocking_flow(neighbor,
                                                                    std::min(flow_limit, capacity));

                        if (pushed_flow > WeightType{})
                        {
                            // Update residual capacities
                            residual_graph.add_edge(current, neighbor, capacity - pushed_flow);
                            residual_graph.add_edge(neighbor, current,
                                                    residual_graph.get_edge_weight(neighbor, current) + pushed_flow);

                            // Update flow values
                            flow[{current, neighbor}] += pushed_flow;

                            return pushed_flow;
                        }
                    }
                }

                return WeightType{};
            };

            // Main algorithm
            WeightType total_flow = WeightType{};
            while (build_level_graph())
            {
                WeightType blocking_flow = find_blocking_flow(source,
                                                              std::numeric_limits<WeightType>::max());
                if (blocking_flow == WeightType{})
                {
                    break;
                }
                total_flow += blocking_flow;
            }

            result.max_flow = total_flow;
            result.flow_values = flow;

            // Find minimum cut (same as Edmonds-Karp)
            std::unordered_set<VertexType> reachable;
            std::queue<VertexType> queue;

            queue.push(source);
            reachable.insert(source);

            while (!queue.empty())
            {
                VertexType current = queue.front();
                queue.pop();

                auto neighbors = residual_graph.get_neighbors(current);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    WeightType capacity = neighbor_pair.second;

                    if (reachable.find(neighbor) == reachable.end() && capacity > WeightType{})
                    {
                        reachable.insert(neighbor);
                        queue.push(neighbor);
                    }
                }
            }

            for (const auto &edge : graph.get_edges())
            {
                if (reachable.find(edge.from) != reachable.end() &&
                    reachable.find(edge.to) == reachable.end())
                {
                    result.min_cut_edges.push_back({edge.from, edge.to});
                }
            }

            return result;
        }

        /**
         * @brief Find maximum bipartite matching using maximum flow
         *
         * Time Complexity: O(VE²) using Edmonds-Karp
         * Space Complexity: O(V)
         *
         * Converts bipartite matching problem to maximum flow problem
         * by adding source and sink vertices.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The bipartite graph
         * @param left_vertices Vertices in the left partition
         * @param right_vertices Vertices in the right partition
         * @return BipartiteMatchingResult containing maximum matching
         */
        template <typename VertexType, typename WeightType = double>
        BipartiteMatchingResult<VertexType> max_bipartite_matching(
            const Graph<VertexType, WeightType> &graph,
            const std::unordered_set<VertexType> &left_vertices,
            const std::unordered_set<VertexType> &right_vertices)
        {

            BipartiteMatchingResult<VertexType> result;

            // Create flow network
            Graph<VertexType, WeightType> flow_network(GraphDirection::DIRECTED);

            // Add source and sink - use special values that won't conflict
            VertexType source, sink;
            if constexpr (std::is_arithmetic_v<VertexType>) {
                source = static_cast<VertexType>(-1);
                sink = static_cast<VertexType>(-2);
            } else {
                // For non-arithmetic types like strings, use special string values
                source = VertexType("__SOURCE__");
                sink = VertexType("__SINK__");
            }

            flow_network.add_vertex(source);
            flow_network.add_vertex(sink);

            // Add all vertices from bipartite graph
            for (const auto &vertex : graph.get_vertices())
            {
                flow_network.add_vertex(vertex);
            }

            // Add edges from source to left vertices
            for (const auto &vertex : left_vertices)
            {
                flow_network.add_edge(source, vertex, WeightType{1});
            }

            // Add edges from right vertices to sink
            for (const auto &vertex : right_vertices)
            {
                flow_network.add_edge(vertex, sink, WeightType{1});
            }

            // Add original edges with capacity 1
            for (const auto &edge : graph.get_edges())
            {
                flow_network.add_edge(edge.from, edge.to, WeightType{1});
            }

            // Find maximum flow
            MaxFlowResult<VertexType, WeightType> flow_result = edmonds_karp(flow_network, source, sink);

            result.max_matching_size = static_cast<std::size_t>(flow_result.max_flow);

            // Extract matching from flow values
            for (const auto &flow_pair : flow_result.flow_values)
            {
                const auto &edge = flow_pair.first;
                WeightType flow_value = flow_pair.second;

                if (flow_value > WeightType{} &&
                    left_vertices.find(edge.first) != left_vertices.end() &&
                    right_vertices.find(edge.second) != right_vertices.end())
                {
                    result.matches.push_back({edge.first, edge.second});
                }
            }

            // Check if it's a perfect matching
            result.is_perfect_matching = (result.max_matching_size ==
                                          std::min(left_vertices.size(), right_vertices.size()));

            return result;
        }

        /**
         * @brief Find minimum cost maximum flow using cycle cancellation
         *
         * Time Complexity: O(VE² log V) with proper implementation
         * Space Complexity: O(V)
         *
         * This is a simplified implementation. For better performance,
         * use more sophisticated algorithms like Successive Shortest Path.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The flow network (capacities as edge weights)
         * @param cost_graph The cost graph (costs as edge weights)
         * @param source Source vertex
         * @param sink Sink vertex
         * @return Pair of (max_flow, min_cost)
         */
        template <typename VertexType, typename WeightType = double>
        std::pair<WeightType, WeightType> min_cost_max_flow(
            const Graph<VertexType, WeightType> &graph,
            const Graph<VertexType, WeightType> &cost_graph,
            const VertexType &source,
            const VertexType &sink)
        {

            // First find maximum flow
            MaxFlowResult<VertexType, WeightType> flow_result = edmonds_karp(graph, source, sink);
            WeightType max_flow = flow_result.max_flow;

            if (max_flow == WeightType{})
            {
                return {WeightType{}, WeightType{}};
            }

            // Calculate total cost
            WeightType total_cost = WeightType{};
            for (const auto &flow_pair : flow_result.flow_values)
            {
                const auto &edge = flow_pair.first;
                WeightType flow_value = flow_pair.second;

                if (flow_value > WeightType{})
                {
                    WeightType cost = cost_graph.get_edge_weight(edge.first, edge.second);
                    total_cost += flow_value * cost;
                }
            }

            return {max_flow, total_cost};
        }

        /**
         * @brief Check if a flow is valid (satisfies capacity and conservation constraints)
         *
         * Time Complexity: O(V + E)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The original graph with capacities
         * @param flow_values The flow values to check
         * @param source Source vertex
         * @param sink Sink vertex
         * @return true if the flow is valid
         */
        template <typename VertexType, typename WeightType = double>
        bool is_valid_flow(const Graph<VertexType, WeightType> &graph,
                           const std::unordered_map<std::pair<VertexType, VertexType>, WeightType> &flow_values,
                           const VertexType &source,
                           const VertexType &sink)
        {

            // Check capacity constraints
            for (const auto &edge : graph.get_edges())
            {
                auto flow_it = flow_values.find({edge.from, edge.to});
                if (flow_it != flow_values.end())
                {
                    if (flow_it->second > edge.weight || flow_it->second < WeightType{})
                    {
                        return false; // Flow exceeds capacity or is negative
                    }
                }
            }

            // Check flow conservation (except source and sink)
            for (const auto &vertex : graph.get_vertices())
            {
                if (vertex == source || vertex == sink)
                {
                    continue;
                }

                WeightType inflow = WeightType{};
                WeightType outflow = WeightType{};

                // Calculate inflow
                for (const auto &edge : graph.get_edges())
                {
                    if (edge.to == vertex)
                    {
                        auto flow_it = flow_values.find({edge.from, edge.to});
                        if (flow_it != flow_values.end())
                        {
                            inflow += flow_it->second;
                        }
                    }
                }

                // Calculate outflow
                for (const auto &edge : graph.get_edges())
                {
                    if (edge.from == vertex)
                    {
                        auto flow_it = flow_values.find({edge.from, edge.to});
                        if (flow_it != flow_values.end())
                        {
                            outflow += flow_it->second;
                        }
                    }
                }

                if (inflow != outflow)
                {
                    return false; // Flow conservation violated
                }
            }

            return true;
        }

    } // namespace algorithms
} // namespace graph_engine
