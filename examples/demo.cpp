#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

#include "graph_engine/graph.hpp"
#include "graph_engine/algorithms/traversal.hpp"
#include "graph_engine/algorithms/shortest_path.hpp"
#include "graph_engine/algorithms/mst.hpp"
#include "graph_engine/algorithms/flow.hpp"
#include "graph_engine/algorithms/advanced.hpp"

using namespace graph_engine;
using namespace graph_engine::algorithms;

void print_separator(const std::string &title)
{
    std::cout << "\n"
              << std::string(50, '=') << "\n";
    std::cout << " " << title << "\n";
    std::cout << std::string(50, '=') << "\n\n";
}

void demo_basic_graph_operations()
{
    print_separator("Basic Graph Operations");

    // Create a directed graph
    Graph<int, double> graph(GraphDirection::DIRECTED);

    // Add vertices and edges
    for (int i = 1; i <= 6; ++i)
    {
        graph.add_vertex(i);
    }

    graph.add_edge(1, 2, 1.0);
    graph.add_edge(1, 3, 2.0);
    graph.add_edge(2, 4, 3.0);
    graph.add_edge(3, 4, 1.5);
    graph.add_edge(4, 5, 2.5);
    graph.add_edge(5, 6, 1.0);
    graph.add_edge(6, 3, 0.5);

    std::cout << "Graph created with " << graph.num_vertices() << " vertices and "
              << graph.num_edges() << " edges\n";

    // Print graph statistics
    graph.print_stats();

    // Test different representations
    std::cout << "\nConverting to different representations:\n";

    graph.convert_to(GraphRepresentation::ADJACENCY_MATRIX);
    std::cout << "Memory usage (Adjacency Matrix): " << graph.memory_usage() << " bytes\n";

    graph.convert_to(GraphRepresentation::EDGE_LIST);
    std::cout << "Memory usage (Edge List): " << graph.memory_usage() << " bytes\n";

    graph.convert_to(GraphRepresentation::ADJACENCY_LIST);
    std::cout << "Memory usage (Adjacency List): " << graph.memory_usage() << " bytes\n";
}

void demo_traversal_algorithms()
{
    print_separator("Traversal Algorithms");

    Graph<int, double> graph(GraphDirection::DIRECTED);

    // Create a sample graph
    for (int i = 1; i <= 7; ++i)
    {
        graph.add_vertex(i);
    }

    graph.add_edge(1, 2, 1.0);
    graph.add_edge(1, 3, 1.0);
    graph.add_edge(2, 4, 1.0);
    graph.add_edge(2, 5, 1.0);
    graph.add_edge(3, 6, 1.0);
    graph.add_edge(3, 7, 1.0);
    graph.add_edge(4, 1, 1.0); // Creates a cycle

    // DFS
    std::cout << "DFS from vertex 1:\n";
    auto dfs_result = dfs(graph, 1);
    std::cout << "Traversal order: ";
    for (const auto &vertex : dfs_result.traversal_order)
    {
        std::cout << vertex << " ";
    }
    std::cout << "\nHas cycle: " << (dfs_result.has_cycle ? "Yes" : "No") << "\n";

    // BFS
    std::cout << "\nBFS from vertex 1:\n";
    auto bfs_result = bfs(graph, 1);
    std::cout << "Traversal order: ";
    for (const auto &vertex : bfs_result.traversal_order)
    {
        std::cout << vertex << " ";
    }
    std::cout << "\nDistances: ";
    for (const auto &pair : bfs_result.distance)
    {
        std::cout << "(" << pair.first << ":" << pair.second << ") ";
    }
    std::cout << "\n";

    // Topological sort
    std::cout << "\nTopological sort:\n";
    auto topo_result = topological_sort(graph);
    if (topo_result.is_dag)
    {
        std::cout << "Topological order: ";
        for (const auto &vertex : topo_result.sorted_order)
        {
            std::cout << vertex << " ";
        }
        std::cout << "\n";
    }
    else
    {
        std::cout << "Graph is not a DAG (has cycles)\n";
    }
}

void demo_shortest_path_algorithms()
{
    print_separator("Shortest Path Algorithms");

    Graph<int, double> graph(GraphDirection::DIRECTED);

    // Create a weighted graph
    for (int i = 1; i <= 6; ++i)
    {
        graph.add_vertex(i);
    }

    graph.add_edge(1, 2, 4.0);
    graph.add_edge(1, 3, 2.0);
    graph.add_edge(2, 3, 1.0);
    graph.add_edge(2, 4, 5.0);
    graph.add_edge(3, 4, 8.0);
    graph.add_edge(3, 5, 10.0);
    graph.add_edge(4, 5, 2.0);
    graph.add_edge(4, 6, 6.0);
    graph.add_edge(5, 6, 3.0);

    // Dijkstra's algorithm
    std::cout << "Dijkstra's algorithm from vertex 1 to vertex 6:\n";
    auto dijkstra_result = dijkstra(graph, 1, 6, true);
    if (dijkstra_result.path_exists)
    {
        std::cout << "Shortest distance: " << dijkstra_result.total_distance << "\n";
        std::cout << "Path: ";
        for (const auto &vertex : dijkstra_result.path)
        {
            std::cout << vertex << " ";
        }
        std::cout << "\n";
    }
    else
    {
        std::cout << "No path exists\n";
    }

    // All-pairs shortest paths using Floyd-Warshall
    std::cout << "\nAll-pairs shortest distances (Floyd-Warshall):\n";
    auto all_pairs = floyd_warshall(graph);
    for (int i = 1; i <= 6; ++i)
    {
        for (int j = 1; j <= 6; ++j)
        {
            if (i != j)
            {
                auto it = all_pairs.find({i, j});
                if (it != all_pairs.end() && it->second != std::numeric_limits<double>::max())
                {
                    std::cout << "Distance from " << i << " to " << j << ": " << it->second << "\n";
                }
            }
        }
    }
}

void demo_mst_algorithms()
{
    print_separator("Minimum Spanning Tree Algorithms");

    Graph<int, double> graph(GraphDirection::UNDIRECTED);

    // Create a weighted undirected graph
    for (int i = 1; i <= 6; ++i)
    {
        graph.add_vertex(i);
    }

    graph.add_edge(1, 2, 4.0);
    graph.add_edge(1, 3, 2.0);
    graph.add_edge(2, 3, 1.0);
    graph.add_edge(2, 4, 5.0);
    graph.add_edge(3, 4, 8.0);
    graph.add_edge(3, 5, 10.0);
    graph.add_edge(4, 5, 2.0);
    graph.add_edge(4, 6, 6.0);
    graph.add_edge(5, 6, 3.0);

    // Kruskal's MST
    std::cout << "Kruskal's MST:\n";
    auto kruskal_result = kruskal_mst(graph);
    if (kruskal_result.is_connected)
    {
        std::cout << "Total weight: " << kruskal_result.total_weight << "\n";
        std::cout << "MST edges:\n";
        for (const auto &edge : kruskal_result.edges)
        {
            std::cout << "  " << edge.from << " -- " << edge.to << " (weight: " << edge.weight << ")\n";
        }
    }
    else
    {
        std::cout << "Graph is not connected\n";
    }

    // Prim's MST
    std::cout << "\nPrim's MST:\n";
    auto prim_result = prim_mst(graph, 1, true);
    if (prim_result.is_connected)
    {
        std::cout << "Total weight: " << prim_result.total_weight << "\n";
        std::cout << "MST edges:\n";
        for (const auto &edge : prim_result.edges)
        {
            std::cout << "  " << edge.from << " -- " << edge.to << " (weight: " << edge.weight << ")\n";
        }
    }
}

void demo_flow_algorithms()
{
    print_separator("Maximum Flow Algorithms");

    Graph<int, double> graph(GraphDirection::DIRECTED);

    // Create a flow network
    for (int i = 1; i <= 6; ++i)
    {
        graph.add_vertex(i);
    }

    // Add edges with capacities
    graph.add_edge(1, 2, 10.0); // source to intermediate
    graph.add_edge(1, 3, 10.0);
    graph.add_edge(2, 3, 2.0);
    graph.add_edge(2, 4, 4.0);
    graph.add_edge(2, 5, 8.0);
    graph.add_edge(3, 5, 9.0);
    graph.add_edge(4, 6, 10.0); // intermediate to sink
    graph.add_edge(5, 6, 10.0);

    std::cout << "Flow network created with " << graph.num_vertices() << " vertices and "
              << graph.num_edges() << " edges\n";

    // Ford-Fulkerson algorithm
    std::cout << "Ford-Fulkerson maximum flow from vertex 1 to vertex 6:\n";

    try
    {
        auto flow_result = ford_fulkerson(graph, 1, 6);

        std::cout << "Maximum flow: " << flow_result.max_flow << "\n";

        if (flow_result.max_flow > 0)
        {
            std::cout << "Flow values:\n";
            for (const auto &flow_pair : flow_result.flow_values)
            {
                if (flow_pair.second > 0)
                {
                    std::cout << "  " << flow_pair.first.first << " -> " << flow_pair.first.second
                              << ": " << flow_pair.second << "\n";
                }
            }

            std::cout << "Minimum cut edges:\n";
            for (const auto &edge : flow_result.min_cut_edges)
            {
                std::cout << "  " << edge.first << " -> " << edge.second << "\n";
            }
        }
        else
        {
            std::cout << "No flow found - check if source and sink are connected\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Error in Ford-Fulkerson: " << e.what() << "\n";
    }
}

void demo_advanced_algorithms()
{
    print_separator("Advanced Algorithms");

    Graph<int, double> graph(GraphDirection::DIRECTED);

    // Create a graph for SCC analysis
    for (int i = 1; i <= 8; ++i)
    {
        graph.add_vertex(i);
    }

    graph.add_edge(1, 2, 1.0);
    graph.add_edge(2, 3, 1.0);
    graph.add_edge(3, 1, 1.0); // SCC: {1, 2, 3}
    graph.add_edge(2, 4, 1.0);
    graph.add_edge(4, 5, 1.0);
    graph.add_edge(5, 6, 1.0);
    graph.add_edge(6, 4, 1.0); // SCC: {4, 5, 6}
    graph.add_edge(7, 6, 1.0);
    graph.add_edge(7, 8, 1.0);
    graph.add_edge(8, 7, 1.0); // SCC: {7, 8}

    // Strongly Connected Components
    std::cout << "Strongly Connected Components (Kosaraju's algorithm):\n";
    auto scc_result = kosaraju_scc(graph);
    std::cout << "Number of SCCs: " << scc_result.num_components << "\n";
    for (size_t i = 0; i < scc_result.components.size(); ++i)
    {
        std::cout << "SCC " << i << ": ";
        for (const auto &vertex : scc_result.components[i])
        {
            std::cout << vertex << " ";
        }
        std::cout << "\n";
    }

    // Graph coloring
    std::cout << "\nGraph Coloring (Greedy algorithm):\n";
    Graph<int, double> coloring_graph(GraphDirection::UNDIRECTED);
    for (int i = 1; i <= 5; ++i)
    {
        coloring_graph.add_vertex(i);
    }

    // Create a 5-cycle (requires 3 colors)
    coloring_graph.add_edge(1, 2, 1.0);
    coloring_graph.add_edge(2, 3, 1.0);
    coloring_graph.add_edge(3, 4, 1.0);
    coloring_graph.add_edge(4, 5, 1.0);
    coloring_graph.add_edge(5, 1, 1.0);

    auto coloring_result = greedy_coloring(coloring_graph);
    std::cout << "Number of colors used: " << coloring_result.num_colors_used << "\n";
    std::cout << "Vertex colors:\n";
    for (const auto &color_pair : coloring_result.colors)
    {
        std::cout << "  Vertex " << color_pair.first << ": Color " << color_pair.second << "\n";
    }

    // Check if bipartite
    std::cout << "\nBipartite check:\n";
    auto [is_bip, bip_coloring] = is_bipartite(coloring_graph);
    std::cout << "Is bipartite: " << (is_bip ? "Yes" : "No") << "\n";
}

void demo_performance_comparison()
{
    print_separator("Performance Comparison");

    // Create a large random graph
    const int num_vertices = 1000;
    const int num_edges = 5000;

    Graph<int, double> graph(GraphDirection::DIRECTED);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> vertex_dist(1, num_vertices);
    std::uniform_real_distribution<> weight_dist(0.1, 10.0);

    // Add vertices
    for (int i = 1; i <= num_vertices; ++i)
    {
        graph.add_vertex(i);
    }

    // Add random edges
    for (int i = 0; i < num_edges; ++i)
    {
        int from = vertex_dist(gen);
        int to = vertex_dist(gen);
        double weight = weight_dist(gen);

        if (from != to)
        {
            graph.add_edge(from, to, weight);
        }
    }

    std::cout << "Created random graph with " << graph.num_vertices()
              << " vertices and " << graph.num_edges() << " edges\n";

    // Test different representations
    auto start = std::chrono::high_resolution_clock::now();
    graph.convert_to(GraphRepresentation::ADJACENCY_LIST);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Adjacency List memory usage: " << graph.memory_usage() << " bytes\n";
    std::cout << "Conversion time: " << duration.count() << " microseconds\n";

    start = std::chrono::high_resolution_clock::now();
    graph.convert_to(GraphRepresentation::ADJACENCY_MATRIX);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Adjacency Matrix memory usage: " << graph.memory_usage() << " bytes\n";
    std::cout << "Conversion time: " << duration.count() << " microseconds\n";

    start = std::chrono::high_resolution_clock::now();
    graph.convert_to(GraphRepresentation::EDGE_LIST);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Edge List memory usage: " << graph.memory_usage() << " bytes\n";
    std::cout << "Conversion time: " << duration.count() << " microseconds\n";
}

int main()
{
    std::cout << "Graph Engine Demo\n";
    std::cout << "================\n";

    try
    {
        demo_basic_graph_operations();
        demo_traversal_algorithms();
        demo_shortest_path_algorithms();
        demo_mst_algorithms();
        demo_flow_algorithms();
        demo_advanced_algorithms();
        demo_performance_comparison();

        std::cout << "\n"
                  << std::string(50, '=') << "\n";
        std::cout << " Demo completed successfully!\n";
        std::cout << std::string(50, '=') << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
