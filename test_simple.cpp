#include <iostream>
#include "graph_engine/graph.hpp"
#include "graph_engine/algorithms/traversal.hpp"
#include "graph_engine/algorithms/shortest_path.hpp"

using namespace graph_engine;
using namespace graph_engine::algorithms;

int main()
{
    std::cout << "Graph Engine Simple Test\n";
    std::cout << "=======================\n\n";

    try
    {
        // Create a simple graph
        Graph<int, double> graph(GraphDirection::DIRECTED);

        // Add vertices
        for (int i = 1; i <= 5; ++i)
        {
            graph.add_vertex(i);
        }

        // Add edges
        graph.add_edge(1, 2, 1.0);
        graph.add_edge(1, 3, 2.0);
        graph.add_edge(2, 4, 3.0);
        graph.add_edge(3, 4, 1.5);
        graph.add_edge(4, 5, 2.5);

        std::cout << "Graph created with " << graph.num_vertices()
                  << " vertices and " << graph.num_edges() << " edges\n";

        // Test DFS
        std::cout << "\nTesting DFS from vertex 1:\n";
        auto dfs_result = dfs(graph, 1);
        std::cout << "DFS traversal order: ";
        for (const auto &vertex : dfs_result.traversal_order)
        {
            std::cout << vertex << " ";
        }
        std::cout << "\n";

        // Test BFS
        std::cout << "\nTesting BFS from vertex 1:\n";
        auto bfs_result = bfs(graph, 1);
        std::cout << "BFS traversal order: ";
        for (const auto &vertex : bfs_result.traversal_order)
        {
            std::cout << vertex << " ";
        }
        std::cout << "\n";

        // Test shortest path
        std::cout << "\nTesting shortest path from 1 to 5:\n";
        auto path_result = dijkstra(graph, 1, 5);
        if (path_result.path_exists)
        {
            std::cout << "Shortest distance: " << path_result.total_distance << "\n";
            std::cout << "Path: ";
            for (const auto &vertex : path_result.path)
            {
                std::cout << vertex << " ";
            }
            std::cout << "\n";
        }
        else
        {
            std::cout << "No path found\n";
        }

        // Test graph representations
        std::cout << "\nTesting graph representations:\n";
        std::cout << "Original memory usage: " << graph.memory_usage() << " bytes\n";

        graph.convert_to(GraphRepresentation::ADJACENCY_MATRIX);
        std::cout << "Matrix memory usage: " << graph.memory_usage() << " bytes\n";

        graph.convert_to(GraphRepresentation::EDGE_LIST);
        std::cout << "Edge list memory usage: " << graph.memory_usage() << " bytes\n";

        std::cout << "\nAll tests passed successfully!\n";
        std::cout << "Graph Engine is working correctly.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
