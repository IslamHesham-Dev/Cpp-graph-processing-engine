#pragma once

#include "../graph.hpp"
#include "../utils/union_find.hpp"
#include "shortest_path.hpp"
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <functional>

namespace graph_engine
{
    namespace algorithms
    {

        /**
         * @brief MST result structure
         */
        template <typename VertexType, typename WeightType>
        struct MSTResult
        {
            std::vector<Edge<VertexType, WeightType>> edges;
            WeightType total_weight = WeightType{};
            bool is_connected = true;

            MSTResult() = default;
        };

        /**
         * @brief Kruskal's algorithm for Minimum Spanning Tree using Union-Find
         *
         * Time Complexity: O(E log E) = O(E log V) for sparse graphs
         * Space Complexity: O(V)
         *
         * Uses Union-Find data structure for efficient cycle detection.
         * Works for both directed and undirected graphs.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to find MST for
         * @return MSTResult containing MST edges and total weight
         */
        template <typename VertexType, typename WeightType = double>
        MSTResult<VertexType, WeightType> kruskal_mst(const Graph<VertexType, WeightType> &graph)
        {
            MSTResult<VertexType, WeightType> result;

            if (graph.empty())
            {
                return result;
            }

            // Get all edges and sort by weight
            std::vector<Edge<VertexType, WeightType>> edges = graph.get_edges();
            std::sort(edges.begin(), edges.end(),
                      [](const Edge<VertexType, WeightType> &a, const Edge<VertexType, WeightType> &b)
                      {
                          return a.weight < b.weight;
                      });

            // Initialize Union-Find
            utils::UnionFind<VertexType> uf;

            // Add all vertices to Union-Find
            for (const auto &vertex : graph.get_vertices())
            {
                uf.make_set(vertex);
            }

            // Process edges in order of increasing weight
            for (const auto &edge : edges)
            {
                if (!uf.same_set(edge.from, edge.to))
                {
                    // Add edge to MST
                    result.edges.push_back(edge);
                    result.total_weight += edge.weight;
                    uf.union_sets(edge.from, edge.to);

                    // Early termination if we have V-1 edges
                    if (result.edges.size() == graph.num_vertices() - 1)
                    {
                        break;
                    }
                }
            }

            // Check if graph is connected
            result.is_connected = (result.edges.size() == graph.num_vertices() - 1);

            return result;
        }

        /**
         * @brief Prim's algorithm for Minimum Spanning Tree
         *
         * Time Complexity: O(E log V) with binary heap
         * Space Complexity: O(V)
         *
         * Grows the MST by always adding the minimum weight edge
         * that connects a vertex in the MST to a vertex outside.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to find MST for
         * @param start_vertex Starting vertex (optional, will pick first vertex if not provided)
         * @return MSTResult containing MST edges and total weight
         */
        template <typename VertexType, typename WeightType = double>
        MSTResult<VertexType, WeightType> prim_mst(const Graph<VertexType, WeightType> &graph,
                                                   const VertexType &start_vertex = VertexType{},
                                                   bool has_start_vertex = false)
        {
            MSTResult<VertexType, WeightType> result;

            if (graph.empty())
            {
                return result;
            }

            // Choose starting vertex
            VertexType start = start_vertex;
            if (!has_start_vertex)
            {
                start = *graph.get_vertices().begin();
            }

            // Priority queue: (weight, (from, to))
            using PQElement = std::pair<WeightType, std::pair<VertexType, VertexType>>;
            std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;

            std::unordered_set<VertexType> in_mst;
            std::unordered_map<VertexType, WeightType> min_weight;

            // Initialize minimum weights
            WeightType infinity = std::numeric_limits<WeightType>::max();
            for (const auto &vertex : graph.get_vertices())
            {
                min_weight[vertex] = infinity;
            }

            // Start with the first vertex
            in_mst.insert(start);
            min_weight[start] = WeightType{};

            // Add all edges from start vertex to priority queue
            auto neighbors = graph.get_neighbors(start);
            for (const auto &neighbor_pair : neighbors)
            {
                const VertexType &neighbor = neighbor_pair.first;
                WeightType weight = neighbor_pair.second;

                if (in_mst.find(neighbor) == in_mst.end() && weight < min_weight[neighbor])
                {
                    min_weight[neighbor] = weight;
                    pq.push({weight, {start, neighbor}});
                }
            }

            // Process edges
            while (!pq.empty() && in_mst.size() < graph.num_vertices())
            {
                auto [weight, edge] = pq.top();
                pq.pop();

                const VertexType &from = edge.first;
                const VertexType &to = edge.second;

                if (in_mst.find(to) == in_mst.end())
                {
                    // Add edge to MST
                    result.edges.emplace_back(from, to, weight);
                    result.total_weight += weight;
                    in_mst.insert(to);

                    // Update neighbors
                    auto to_neighbors = graph.get_neighbors(to);
                    for (const auto &neighbor_pair : to_neighbors)
                    {
                        const VertexType &neighbor = neighbor_pair.first;
                        WeightType edge_weight = neighbor_pair.second;

                        if (in_mst.find(neighbor) == in_mst.end() && edge_weight < min_weight[neighbor])
                        {
                            min_weight[neighbor] = edge_weight;
                            pq.push({edge_weight, {to, neighbor}});
                        }
                    }
                }
            }

            // Check if graph is connected
            result.is_connected = (in_mst.size() == graph.num_vertices());

            return result;
        }

        /**
         * @brief Boruvka's algorithm for Minimum Spanning Tree
         *
         * Time Complexity: O(E log V)
         * Space Complexity: O(V)
         *
         * Also known as Sollin's algorithm. Processes all components
         * simultaneously, making it suitable for parallel implementation.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to find MST for
         * @return MSTResult containing MST edges and total weight
         */
        template <typename VertexType, typename WeightType = double>
        MSTResult<VertexType, WeightType> boruvka_mst(const Graph<VertexType, WeightType> &graph)
        {
            MSTResult<VertexType, WeightType> result;

            if (graph.empty())
            {
                return result;
            }

            // Initialize Union-Find
            utils::UnionFind<VertexType> uf;
            for (const auto &vertex : graph.get_vertices())
            {
                uf.make_set(vertex);
            }

            // Continue until we have one component
            while (uf.num_sets() > 1)
            {
                // For each component, find minimum weight edge
                std::unordered_map<VertexType, Edge<VertexType, WeightType>> min_edges;

                for (const auto &edge : graph.get_edges())
                {
                    VertexType from_component = uf.find(edge.from);
                    VertexType to_component = uf.find(edge.to);

                    if (from_component != to_component)
                    {
                        // Edge connects different components
                        if (min_edges.find(from_component) == min_edges.end() ||
                            edge.weight < min_edges[from_component].weight)
                        {
                            min_edges[from_component] = edge;
                        }

                        if (min_edges.find(to_component) == min_edges.end() ||
                            edge.weight < min_edges[to_component].weight)
                        {
                            min_edges[to_component] = edge;
                        }
                    }
                }

                // Add minimum edges to MST
                bool added_edge = false;
                for (const auto &pair : min_edges)
                {
                    const auto &edge = pair.second;

                    if (!uf.same_set(edge.from, edge.to))
                    {
                        result.edges.push_back(edge);
                        result.total_weight += edge.weight;
                        uf.union_sets(edge.from, edge.to);
                        added_edge = true;
                    }
                }

                // If no edges were added, graph is not connected
                if (!added_edge)
                {
                    result.is_connected = false;
                    break;
                }
            }

            result.is_connected = (uf.num_sets() == 1);

            return result;
        }

        /**
         * @brief Find all possible MSTs (if multiple exist with same weight)
         *
         * Time Complexity: O(E² log V) in worst case
         * Space Complexity: O(E)
         *
         * Uses Kruskal's algorithm with tie-breaking to find all MSTs.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to find MSTs for
         * @return Vector of MSTResult, each representing a different MST
         */
        template <typename VertexType, typename WeightType = double>
        std::vector<MSTResult<VertexType, WeightType>> find_all_msts(const Graph<VertexType, WeightType> &graph)
        {
            std::vector<MSTResult<VertexType, WeightType>> all_msts;

            if (graph.empty())
            {
                return all_msts;
            }

            // Get all edges and group by weight
            std::vector<Edge<VertexType, WeightType>> edges = graph.get_edges();
            std::sort(edges.begin(), edges.end(),
                      [](const Edge<VertexType, WeightType> &a, const Edge<VertexType, WeightType> &b)
                      {
                          return a.weight < b.weight;
                      });

            // Find minimum weight
            WeightType min_weight = edges.empty() ? WeightType{} : edges[0].weight;

            // Group edges by weight
            std::unordered_map<WeightType, std::vector<Edge<VertexType, WeightType>>> edges_by_weight;
            for (const auto &edge : edges)
            {
                edges_by_weight[edge.weight].push_back(edge);
            }

            // Use recursive backtracking to find all MSTs
            std::function<void(size_t, utils::UnionFind<VertexType>, std::vector<Edge<VertexType, WeightType>>, WeightType)>
                find_msts_recursive = [&](size_t edge_index,
                                          utils::UnionFind<VertexType> uf,
                                          std::vector<Edge<VertexType, WeightType>> current_edges,
                                          WeightType current_weight)
            {
                if (current_edges.size() == graph.num_vertices() - 1)
                {
                    // Found a complete MST
                    MSTResult<VertexType, WeightType> mst;
                    mst.edges = current_edges;
                    mst.total_weight = current_weight;
                    mst.is_connected = true;
                    all_msts.push_back(mst);
                    return;
                }

                if (edge_index >= edges.size())
                {
                    return;
                }

                // Try including current edge
                const auto &edge = edges[edge_index];
                if (!uf.same_set(edge.from, edge.to))
                {
                    utils::UnionFind<VertexType> new_uf = uf;
                    new_uf.union_sets(edge.from, edge.to);

                    std::vector<Edge<VertexType, WeightType>> new_edges = current_edges;
                    new_edges.push_back(edge);

                    find_msts_recursive(edge_index + 1, new_uf, new_edges, current_weight + edge.weight);
                }

                // Try excluding current edge
                find_msts_recursive(edge_index + 1, uf, current_edges, current_weight);
            };

            // Initialize Union-Find
            utils::UnionFind<VertexType> uf;
            for (const auto &vertex : graph.get_vertices())
            {
                uf.make_set(vertex);
            }

            find_msts_recursive(0, uf, {}, WeightType{});

            return all_msts;
        }

        /**
         * @brief Find second-best MST
         *
         * Time Complexity: O(V² + E log V)
         * Space Complexity: O(V)
         *
         * Finds the MST with the second-smallest total weight.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to find second-best MST for
         * @return MSTResult containing second-best MST
         */
        template <typename VertexType, typename WeightType = double>
        MSTResult<VertexType, WeightType> second_best_mst(const Graph<VertexType, WeightType> &graph)
        {
            MSTResult<VertexType, WeightType> result;

            if (graph.empty())
            {
                return result;
            }

            // Find the best MST
            MSTResult<VertexType, WeightType> best_mst = kruskal_mst(graph);

            if (!best_mst.is_connected)
            {
                return result; // No MST exists
            }

            WeightType second_best_weight = std::numeric_limits<WeightType>::max();

            // Try removing each edge from the best MST and find the best replacement
            for (size_t i = 0; i < best_mst.edges.size(); ++i)
            {
                // Create graph without the i-th edge
                Graph<VertexType, WeightType> temp_graph = graph;
                temp_graph.remove_edge(best_mst.edges[i].from, best_mst.edges[i].to);

                // Find MST in the modified graph
                MSTResult<VertexType, WeightType> temp_mst = kruskal_mst(temp_graph);

                if (temp_mst.is_connected && temp_mst.total_weight < second_best_weight)
                {
                    second_best_weight = temp_mst.total_weight;
                    result = temp_mst;
                }
            }

            return result;
        }

        /**
         * @brief Check if a given set of edges forms a valid MST
         *
         * Time Complexity: O(E log V)
         * Space Complexity: O(V)
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The original graph
         * @param mst_edges The edges to check
         * @return true if the edges form a valid MST
         */
        template <typename VertexType, typename WeightType = double>
        bool is_valid_mst(const Graph<VertexType, WeightType> &graph,
                          const std::vector<Edge<VertexType, WeightType>> &mst_edges)
        {
            if (mst_edges.size() != graph.num_vertices() - 1)
            {
                return false; // Wrong number of edges
            }

            // Check if all edges exist in the original graph
            for (const auto &edge : mst_edges)
            {
                if (!graph.has_edge(edge.from, edge.to) ||
                    graph.get_edge_weight(edge.from, edge.to) != edge.weight)
                {
                    return false;
                }
            }

            // Check if the edges form a connected graph
            utils::UnionFind<VertexType> uf;
            for (const auto &vertex : graph.get_vertices())
            {
                uf.make_set(vertex);
            }

            for (const auto &edge : mst_edges)
            {
                uf.union_sets(edge.from, edge.to);
            }

            if (uf.num_sets() != 1)
            {
                return false; // Not connected
            }

            // Check if it's actually minimum (compare with known MST)
            MSTResult<VertexType, WeightType> actual_mst = kruskal_mst(graph);
            WeightType mst_weight = WeightType{};
            for (const auto &edge : mst_edges)
            {
                mst_weight += edge.weight;
            }

            return mst_weight == actual_mst.total_weight;
        }

        /**
         * @brief Find MST for a specific subset of vertices (Steiner Tree approximation)
         *
         * Time Complexity: O(V² log V)
         * Space Complexity: O(V)
         *
         * This is a simple approximation for the Steiner Tree problem.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to find Steiner Tree for
         * @param required_vertices Set of vertices that must be included
         * @return MSTResult containing Steiner Tree approximation
         */
        template <typename VertexType, typename WeightType = double>
        MSTResult<VertexType, WeightType> steiner_tree_approximation(
            const Graph<VertexType, WeightType> &graph,
            const std::unordered_set<VertexType> &required_vertices)
        {

            MSTResult<VertexType, WeightType> result;

            if (required_vertices.empty())
            {
                return result;
            }

            // Create complete graph on required vertices using shortest paths
            Graph<VertexType, WeightType> complete_graph(GraphDirection::UNDIRECTED);

            for (const auto &vertex : required_vertices)
            {
                complete_graph.add_vertex(vertex);
            }

            // Add edges between all pairs of required vertices
            for (const auto &v1 : required_vertices)
            {
                for (const auto &v2 : required_vertices)
                {
                    if (v1 != v2)
                    {
                        // Find shortest path between v1 and v2
                        auto shortest_path = dijkstra(graph, v1, v2, true);
                        if (shortest_path.path_exists)
                        {
                            complete_graph.add_edge(v1, v2, shortest_path.total_distance);
                        }
                    }
                }
            }

            // Find MST in the complete graph
            result = kruskal_mst(complete_graph);

            return result;
        }

    } // namespace algorithms
} // namespace graph_engine
