#pragma once

#include "../graph.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include <functional>

namespace graph_engine
{
    namespace algorithms
    {

        /**
         * @brief Shortest path result structure
         */
        template <typename VertexType, typename WeightType>
        struct ShortestPathResult
        {
            std::unordered_map<VertexType, WeightType> distances;
            std::unordered_map<VertexType, VertexType> predecessors;
            std::vector<VertexType> path;
            bool path_exists = false;
            WeightType total_distance = WeightType{};

            ShortestPathResult() = default;
        };

        /**
         * @brief Dijkstra's algorithm for shortest paths in non-negative weighted graphs
         *
         * Time Complexity: O((V + E) log V) with binary heap
         * Space Complexity: O(V)
         *
         * Uses a priority queue (min-heap) to efficiently find the next vertex
         * with minimum distance. Only works with non-negative edge weights.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to search
         * @param start_vertex Starting vertex
         * @param end_vertex Target vertex (optional, if not provided, computes distances to all vertices)
         * @return ShortestPathResult containing distances and path information
         */
        template <typename VertexType, typename WeightType = double>
        ShortestPathResult<VertexType, WeightType> dijkstra(const Graph<VertexType, WeightType> &graph,
                                                            const VertexType &start_vertex,
                                                            const VertexType &end_vertex = VertexType{},
                                                            bool has_end_vertex = false)
        {
            ShortestPathResult<VertexType, WeightType> result;

            // Priority queue: (distance, vertex)
            using PQElement = std::pair<WeightType, VertexType>;
            std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;

            // Initialize distances
            WeightType infinity = std::numeric_limits<WeightType>::max();
            for (const auto &vertex : graph.get_vertices())
            {
                result.distances[vertex] = infinity;
            }
            result.distances[start_vertex] = WeightType{};

            // Track visited vertices
            std::unordered_set<VertexType> visited;

            // Start with source vertex
            pq.push({WeightType{}, start_vertex});

            while (!pq.empty())
            {
                auto [current_distance, current_vertex] = pq.top();
                pq.pop();

                // Skip if already visited
                if (visited.find(current_vertex) != visited.end())
                {
                    continue;
                }

                visited.insert(current_vertex);

                // Early termination if target found
                if (has_end_vertex && current_vertex == end_vertex)
                {
                    break;
                }

                // Relax edges
                auto neighbors = graph.get_neighbors(current_vertex);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    WeightType edge_weight = neighbor_pair.second;

                    if (visited.find(neighbor) == visited.end())
                    {
                        WeightType new_distance = current_distance + edge_weight;

                        if (new_distance < result.distances[neighbor])
                        {
                            result.distances[neighbor] = new_distance;
                            result.predecessors[neighbor] = current_vertex;
                            pq.push({new_distance, neighbor});
                        }
                    }
                }
            }

            // Build path if target was specified and reachable
            if (has_end_vertex && result.distances[end_vertex] != infinity)
            {
                result.path_exists = true;
                result.total_distance = result.distances[end_vertex];

                // Reconstruct path
                VertexType current = end_vertex;
                while (current != start_vertex)
                {
                    result.path.push_back(current);
                    auto pred_it = result.predecessors.find(current);
                    if (pred_it != result.predecessors.end())
                    {
                        current = pred_it->second;
                    }
                    else
                    {
                        result.path.clear();
                        result.path_exists = false;
                        break;
                    }
                }
                result.path.push_back(start_vertex);
                std::reverse(result.path.begin(), result.path.end());
            }

            return result;
        }

        /**
         * @brief Bellman-Ford algorithm for shortest paths with negative weights
         *
         * Time Complexity: O(VE)
         * Space Complexity: O(V)
         *
         * Can handle negative edge weights and detects negative cycles.
         * Slower than Dijkstra but more general.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to search
         * @param start_vertex Starting vertex
         * @param end_vertex Target vertex (optional)
         * @return ShortestPathResult containing distances and path information
         */
        template <typename VertexType, typename WeightType = double>
        ShortestPathResult<VertexType, WeightType> bellman_ford(const Graph<VertexType, WeightType> &graph,
                                                                const VertexType &start_vertex,
                                                                const VertexType &end_vertex = VertexType{},
                                                                bool has_end_vertex = false)
        {
            ShortestPathResult<VertexType, WeightType> result;

            // Initialize distances
            WeightType infinity = std::numeric_limits<WeightType>::max();
            for (const auto &vertex : graph.get_vertices())
            {
                result.distances[vertex] = infinity;
            }
            result.distances[start_vertex] = WeightType{};

            // Relax edges V-1 times
            for (size_t i = 0; i < graph.num_vertices() - 1; ++i)
            {
                bool relaxed = false;

                for (const auto &edge : graph.get_edges())
                {
                    if (result.distances[edge.from] != infinity)
                    {
                        WeightType new_distance = result.distances[edge.from] + edge.weight;

                        if (new_distance < result.distances[edge.to])
                        {
                            result.distances[edge.to] = new_distance;
                            result.predecessors[edge.to] = edge.from;
                            relaxed = true;
                        }
                    }
                }

                // Early termination if no relaxation occurred
                if (!relaxed)
                {
                    break;
                }
            }

            // Check for negative cycles
            bool has_negative_cycle = false;
            for (const auto &edge : graph.get_edges())
            {
                if (result.distances[edge.from] != infinity)
                {
                    WeightType new_distance = result.distances[edge.from] + edge.weight;

                    if (new_distance < result.distances[edge.to])
                    {
                        has_negative_cycle = true;
                        break;
                    }
                }
            }

            if (has_negative_cycle)
            {
                // Clear results if negative cycle detected
                result.distances.clear();
                result.predecessors.clear();
                return result;
            }

            // Build path if target was specified and reachable
            if (has_end_vertex && result.distances[end_vertex] != infinity)
            {
                result.path_exists = true;
                result.total_distance = result.distances[end_vertex];

                // Reconstruct path
                VertexType current = end_vertex;
                while (current != start_vertex)
                {
                    result.path.push_back(current);
                    auto pred_it = result.predecessors.find(current);
                    if (pred_it != result.predecessors.end())
                    {
                        current = pred_it->second;
                    }
                    else
                    {
                        result.path.clear();
                        result.path_exists = false;
                        break;
                    }
                }
                result.path.push_back(start_vertex);
                std::reverse(result.path.begin(), result.path.end());
            }

            return result;
        }

        /**
         * @brief Floyd-Warshall algorithm for all-pairs shortest paths
         *
         * Time Complexity: O(V³)
         * Space Complexity: O(V²)
         *
         * Computes shortest paths between all pairs of vertices.
         * Can handle negative weights but not negative cycles.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to analyze
         * @return Map of (source, target) -> distance
         */
        template <typename VertexType, typename WeightType = double>
        std::unordered_map<std::pair<VertexType, VertexType>, WeightType> floyd_warshall(
            const Graph<VertexType, WeightType> &graph)
        {

            std::unordered_map<std::pair<VertexType, VertexType>, WeightType> distances;
            WeightType infinity = std::numeric_limits<WeightType>::max();

            // Initialize distances
            std::vector<VertexType> vertices(graph.get_vertices().begin(), graph.get_vertices().end());

            for (const auto &u : vertices)
            {
                for (const auto &v : vertices)
                {
                    if (u == v)
                    {
                        distances[{u, v}] = WeightType{};
                    }
                    else
                    {
                        distances[{u, v}] = infinity;
                    }
                }
            }

            // Set initial edge weights
            for (const auto &edge : graph.get_edges())
            {
                distances[{edge.from, edge.to}] = edge.weight;
            }

            // Floyd-Warshall algorithm
            for (const auto &k : vertices)
            {
                for (const auto &i : vertices)
                {
                    for (const auto &j : vertices)
                    {
                        if (distances[{i, k}] != infinity && distances[{k, j}] != infinity)
                        {
                            WeightType new_distance = distances[{i, k}] + distances[{k, j}];
                            if (new_distance < distances[{i, j}])
                            {
                                distances[{i, j}] = new_distance;
                            }
                        }
                    }
                }
            }

            return distances;
        }

        /**
         * @brief A* algorithm for shortest path with heuristic
         *
         * Time Complexity: O(b^d) where b is branching factor and d is depth
         * Space Complexity: O(b^d)
         *
         * Uses a heuristic function to guide the search towards the goal.
         * Optimal if heuristic is admissible (never overestimates).
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to search
         * @param start_vertex Starting vertex
         * @param end_vertex Target vertex
         * @param heuristic Heuristic function: (current, goal) -> estimated distance
         * @return ShortestPathResult containing path information
         */
        template <typename VertexType, typename WeightType = double>
        ShortestPathResult<VertexType, WeightType> a_star(const Graph<VertexType, WeightType> &graph,
                                                          const VertexType &start_vertex,
                                                          const VertexType &end_vertex,
                                                          std::function<WeightType(const VertexType &, const VertexType &)> heuristic)
        {
            ShortestPathResult<VertexType, WeightType> result;

            // Priority queue: (f_score, vertex)
            using PQElement = std::pair<WeightType, VertexType>;
            std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> open_set;

            // Track g_scores (actual distance from start)
            std::unordered_map<VertexType, WeightType> g_score;
            std::unordered_map<VertexType, WeightType> f_score;

            WeightType infinity = std::numeric_limits<WeightType>::max();

            // Initialize scores
            for (const auto &vertex : graph.get_vertices())
            {
                g_score[vertex] = infinity;
                f_score[vertex] = infinity;
            }

            g_score[start_vertex] = WeightType{};
            f_score[start_vertex] = heuristic(start_vertex, end_vertex);

            open_set.push({f_score[start_vertex], start_vertex});

            std::unordered_set<VertexType> closed_set;

            while (!open_set.empty())
            {
                auto [current_f, current] = open_set.top();
                open_set.pop();

                if (current == end_vertex)
                {
                    // Reconstruct path
                    result.path_exists = true;
                    result.total_distance = g_score[end_vertex];

                    VertexType path_vertex = end_vertex;
                    while (path_vertex != start_vertex)
                    {
                        result.path.push_back(path_vertex);
                        auto pred_it = result.predecessors.find(path_vertex);
                        if (pred_it != result.predecessors.end())
                        {
                            path_vertex = pred_it->second;
                        }
                        else
                        {
                            result.path.clear();
                            result.path_exists = false;
                            break;
                        }
                    }
                    result.path.push_back(start_vertex);
                    std::reverse(result.path.begin(), result.path.end());

                    break;
                }

                closed_set.insert(current);

                // Process neighbors
                auto neighbors = graph.get_neighbors(current);
                for (const auto &neighbor_pair : neighbors)
                {
                    const VertexType &neighbor = neighbor_pair.first;
                    WeightType edge_weight = neighbor_pair.second;

                    if (closed_set.find(neighbor) != closed_set.end())
                    {
                        continue;
                    }

                    WeightType tentative_g_score = g_score[current] + edge_weight;

                    if (tentative_g_score < g_score[neighbor])
                    {
                        result.predecessors[neighbor] = current;
                        g_score[neighbor] = tentative_g_score;
                        f_score[neighbor] = g_score[neighbor] + heuristic(neighbor, end_vertex);

                        open_set.push({f_score[neighbor], neighbor});
                    }
                }
            }

            return result;
        }

        /**
         * @brief Bidirectional Dijkstra for faster shortest path search
         *
         * Time Complexity: O((V + E) log V) but typically faster than unidirectional
         * Space Complexity: O(V)
         *
         * Searches from both start and end vertices simultaneously.
         * Can be significantly faster for large graphs.
         *
         * @tparam VertexType Type of vertex identifiers
         * @tparam WeightType Type of edge weights
         * @param graph The graph to search
         * @param start_vertex Starting vertex
         * @param end_vertex Target vertex
         * @return ShortestPathResult containing path information
         */
        template <typename VertexType, typename WeightType = double>
        ShortestPathResult<VertexType, WeightType> bidirectional_dijkstra(
            const Graph<VertexType, WeightType> &graph,
            const VertexType &start_vertex,
            const VertexType &end_vertex)
        {

            ShortestPathResult<VertexType, WeightType> result;

            // Forward search
            std::unordered_map<VertexType, WeightType> dist_forward;
            std::unordered_map<VertexType, VertexType> pred_forward;
            std::priority_queue<std::pair<WeightType, VertexType>,
                                std::vector<std::pair<WeightType, VertexType>>,
                                std::greater<std::pair<WeightType, VertexType>>>
                pq_forward;

            // Backward search
            std::unordered_map<VertexType, WeightType> dist_backward;
            std::unordered_map<VertexType, VertexType> pred_backward;
            std::priority_queue<std::pair<WeightType, VertexType>,
                                std::vector<std::pair<WeightType, VertexType>>,
                                std::greater<std::pair<WeightType, VertexType>>>
                pq_backward;

            WeightType infinity = std::numeric_limits<WeightType>::max();

            // Initialize forward search
            for (const auto &vertex : graph.get_vertices())
            {
                dist_forward[vertex] = infinity;
                dist_backward[vertex] = infinity;
            }

            dist_forward[start_vertex] = WeightType{};
            dist_backward[end_vertex] = WeightType{};

            pq_forward.push({WeightType{}, start_vertex});
            pq_backward.push({WeightType{}, end_vertex});

            std::unordered_set<VertexType> visited_forward, visited_backward;
            WeightType best_distance = infinity;
            VertexType meeting_vertex;

            while (!pq_forward.empty() && !pq_backward.empty())
            {
                // Forward step
                if (!pq_forward.empty())
                {
                    auto [dist, current] = pq_forward.top();
                    pq_forward.pop();

                    if (visited_forward.find(current) == visited_forward.end())
                    {
                        visited_forward.insert(current);

                        if (visited_backward.find(current) != visited_backward.end())
                        {
                            // Found meeting point
                            WeightType total_dist = dist_forward[current] + dist_backward[current];
                            if (total_dist < best_distance)
                            {
                                best_distance = total_dist;
                                meeting_vertex = current;
                            }
                        }

                        auto neighbors = graph.get_neighbors(current);
                        for (const auto &neighbor_pair : neighbors)
                        {
                            const VertexType &neighbor = neighbor_pair.first;
                            WeightType edge_weight = neighbor_pair.second;

                            if (visited_forward.find(neighbor) == visited_forward.end())
                            {
                                WeightType new_dist = dist_forward[current] + edge_weight;
                                if (new_dist < dist_forward[neighbor])
                                {
                                    dist_forward[neighbor] = new_dist;
                                    pred_forward[neighbor] = current;
                                    pq_forward.push({new_dist, neighbor});
                                }
                            }
                        }
                    }
                }

                // Backward step
                if (!pq_backward.empty())
                {
                    auto [dist, current] = pq_backward.top();
                    pq_backward.pop();

                    if (visited_backward.find(current) == visited_backward.end())
                    {
                        visited_backward.insert(current);

                        if (visited_forward.find(current) != visited_forward.end())
                        {
                            // Found meeting point
                            WeightType total_dist = dist_forward[current] + dist_backward[current];
                            if (total_dist < best_distance)
                            {
                                best_distance = total_dist;
                                meeting_vertex = current;
                            }
                        }

                        // For backward search, we need to find incoming edges
                        // This requires iterating through all edges (inefficient for large graphs)
                        for (const auto &edge : graph.get_edges())
                        {
                            if (edge.to == current)
                            {
                                const VertexType &neighbor = edge.from;
                                WeightType edge_weight = edge.weight;

                                if (visited_backward.find(neighbor) == visited_backward.end())
                                {
                                    WeightType new_dist = dist_backward[current] + edge_weight;
                                    if (new_dist < dist_backward[neighbor])
                                    {
                                        dist_backward[neighbor] = new_dist;
                                        pred_backward[neighbor] = current;
                                        pq_backward.push({new_dist, neighbor});
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (best_distance != infinity)
            {
                result.path_exists = true;
                result.total_distance = best_distance;

                // Reconstruct path from start to meeting point
                std::vector<VertexType> forward_path;
                VertexType current = meeting_vertex;
                while (current != start_vertex)
                {
                    forward_path.push_back(current);
                    auto pred_it = pred_forward.find(current);
                    if (pred_it != pred_forward.end())
                    {
                        current = pred_it->second;
                    }
                    else
                    {
                        result.path.clear();
                        result.path_exists = false;
                        return result;
                    }
                }
                forward_path.push_back(start_vertex);
                std::reverse(forward_path.begin(), forward_path.end());

                // Reconstruct path from meeting point to end
                std::vector<VertexType> backward_path;
                current = meeting_vertex;
                while (current != end_vertex)
                {
                    auto pred_it = pred_backward.find(current);
                    if (pred_it != pred_backward.end())
                    {
                        current = pred_it->second;
                        backward_path.push_back(current);
                    }
                    else
                    {
                        result.path.clear();
                        result.path_exists = false;
                        return result;
                    }
                }

                // Combine paths
                result.path = forward_path;
                result.path.insert(result.path.end(), backward_path.begin(), backward_path.end());
            }

            return result;
        }

    } // namespace algorithms
} // namespace graph_engine
