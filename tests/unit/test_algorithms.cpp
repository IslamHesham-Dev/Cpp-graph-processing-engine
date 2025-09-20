#include <gtest/gtest.h>
#include "graph_engine/graph.hpp"
#include "graph_engine/algorithms/traversal.hpp"
#include "graph_engine/algorithms/shortest_path.hpp"
#include "graph_engine/algorithms/mst.hpp"
#include "graph_engine/algorithms/flow.hpp"
#include "graph_engine/algorithms/advanced.hpp"

using namespace graph_engine;
using namespace graph_engine::algorithms;

class AlgorithmTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a test graph
        graph_ = Graph<int, double>(GraphDirection::DIRECTED);

        // Add vertices
        for (int i = 1; i <= 6; ++i)
        {
            graph_.add_vertex(i);
        }

        // Add edges
        graph_.add_edge(1, 2, 1.0);
        graph_.add_edge(1, 3, 2.0);
        graph_.add_edge(2, 4, 3.0);
        graph_.add_edge(3, 4, 1.5);
        graph_.add_edge(4, 5, 2.5);
        graph_.add_edge(5, 6, 1.0);
    }

    Graph<int, double> graph_;
};

TEST_F(AlgorithmTest, DFSTraversal)
{
    auto dfs_result = dfs(graph_, 1);

    EXPECT_FALSE(dfs_result.traversal_order.empty());
    EXPECT_EQ(dfs_result.traversal_order[0], 1); // Should start with vertex 1
    EXPECT_TRUE(dfs_result.visited.find(1) != dfs_result.visited.end());

    // Check that all reachable vertices are visited
    EXPECT_TRUE(dfs_result.visited.find(2) != dfs_result.visited.end());
    EXPECT_TRUE(dfs_result.visited.find(4) != dfs_result.visited.end());
}

TEST_F(AlgorithmTest, BFSTraversal)
{
    auto bfs_result = bfs(graph_, 1);

    EXPECT_FALSE(bfs_result.traversal_order.empty());
    EXPECT_EQ(bfs_result.traversal_order[0], 1); // Should start with vertex 1

    // Check distances
    EXPECT_EQ(bfs_result.distance[1], 0);
    EXPECT_EQ(bfs_result.distance[2], 1);
    EXPECT_EQ(bfs_result.distance[3], 1);
    EXPECT_EQ(bfs_result.distance[4], 2);
}

TEST_F(AlgorithmTest, CycleDetection)
{
    // Test graph without cycles
    EXPECT_FALSE(has_cycle(graph_));

    // Add a cycle
    graph_.add_edge(6, 1, 1.0);
    EXPECT_TRUE(has_cycle(graph_));
}

TEST_F(AlgorithmTest, TopologicalSort)
{
    // Test DAG
    auto topo_result = topological_sort(graph_);
    EXPECT_TRUE(topo_result.is_dag);
    EXPECT_FALSE(topo_result.sorted_order.empty());

    // Add a cycle
    graph_.add_edge(6, 1, 1.0);
    auto topo_result_with_cycle = topological_sort(graph_);
    EXPECT_FALSE(topo_result_with_cycle.is_dag);
}

TEST_F(AlgorithmTest, ShortestPathUnweighted)
{
    auto path = shortest_path_unweighted(graph_, 1, 6);

    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path[0], 1);     // Should start with source
    EXPECT_EQ(path.back(), 6); // Should end with destination

    // Test unreachable vertex
    graph_.add_vertex(7);
    auto unreachable_path = shortest_path_unweighted(graph_, 1, 7);
    EXPECT_TRUE(unreachable_path.empty());
}

TEST_F(AlgorithmTest, DijkstraAlgorithm)
{
    auto dijkstra_result = dijkstra(graph_, 1, 6);

    EXPECT_TRUE(dijkstra_result.path_exists);
    EXPECT_GT(dijkstra_result.total_distance, 0);
    EXPECT_FALSE(dijkstra_result.path.empty());
    EXPECT_EQ(dijkstra_result.path[0], 1);
    EXPECT_EQ(dijkstra_result.path.back(), 6);
}

TEST_F(AlgorithmTest, BellmanFordAlgorithm)
{
    auto bellman_result = bellman_ford(graph_, 1, 6);

    EXPECT_TRUE(bellman_result.path_exists);
    EXPECT_GT(bellman_result.total_distance, 0);
    EXPECT_FALSE(bellman_result.path.empty());
}

TEST_F(AlgorithmTest, FloydWarshallAlgorithm)
{
    auto all_pairs = floyd_warshall(graph_);

    // Check that we have distances for all pairs
    EXPECT_GT(all_pairs.size(), 0);

    // Check specific distance
    auto it = all_pairs.find({1, 6});
    EXPECT_TRUE(it != all_pairs.end());
    EXPECT_GT(it->second, 0);
}

TEST_F(AlgorithmTest, KruskalMST)
{
    // Create undirected graph for MST
    Graph<int, double> mst_graph(GraphDirection::UNDIRECTED);

    for (int i = 1; i <= 4; ++i)
    {
        mst_graph.add_vertex(i);
    }

    mst_graph.add_edge(1, 2, 1.0);
    mst_graph.add_edge(1, 3, 2.0);
    mst_graph.add_edge(2, 3, 3.0);
    mst_graph.add_edge(2, 4, 4.0);
    mst_graph.add_edge(3, 4, 5.0);

    auto mst_result = kruskal_mst(mst_graph);

    EXPECT_TRUE(mst_result.is_connected);
    EXPECT_EQ(mst_result.edges.size(), 3); // V-1 edges for MST
    EXPECT_GT(mst_result.total_weight, 0);
}

TEST_F(AlgorithmTest, PrimMST)
{
    // Create undirected graph for MST
    Graph<int, double> mst_graph(GraphDirection::UNDIRECTED);

    for (int i = 1; i <= 4; ++i)
    {
        mst_graph.add_vertex(i);
    }

    mst_graph.add_edge(1, 2, 1.0);
    mst_graph.add_edge(1, 3, 2.0);
    mst_graph.add_edge(2, 3, 3.0);
    mst_graph.add_edge(2, 4, 4.0);
    mst_graph.add_edge(3, 4, 5.0);

    auto mst_result = prim_mst(mst_graph, 1);

    EXPECT_TRUE(mst_result.is_connected);
    EXPECT_EQ(mst_result.edges.size(), 3); // V-1 edges for MST
    EXPECT_GT(mst_result.total_weight, 0);
}

TEST_F(AlgorithmTest, FordFulkersonMaxFlow)
{
    // Create flow network
    Graph<int, double> flow_graph(GraphDirection::DIRECTED);

    for (int i = 1; i <= 4; ++i)
    {
        flow_graph.add_vertex(i);
    }

    flow_graph.add_edge(1, 2, 10.0); // source to intermediate
    flow_graph.add_edge(1, 3, 10.0);
    flow_graph.add_edge(2, 4, 10.0); // intermediate to sink
    flow_graph.add_edge(3, 4, 10.0);

    auto flow_result = ford_fulkerson(flow_graph, 1, 4);

    EXPECT_GT(flow_result.max_flow, 0);
    EXPECT_FALSE(flow_result.flow_values.empty());
}

TEST_F(AlgorithmTest, EdmondsKarpMaxFlow)
{
    // Create flow network
    Graph<int, double> flow_graph(GraphDirection::DIRECTED);

    for (int i = 1; i <= 4; ++i)
    {
        flow_graph.add_vertex(i);
    }

    flow_graph.add_edge(1, 2, 10.0);
    flow_graph.add_edge(1, 3, 10.0);
    flow_graph.add_edge(2, 4, 10.0);
    flow_graph.add_edge(3, 4, 10.0);

    auto flow_result = edmonds_karp(flow_graph, 1, 4);

    EXPECT_GT(flow_result.max_flow, 0);
    EXPECT_FALSE(flow_result.flow_values.empty());
}

TEST_F(AlgorithmTest, KosarajuSCC)
{
    // Create graph with SCCs
    Graph<int, double> scc_graph(GraphDirection::DIRECTED);

    for (int i = 1; i <= 6; ++i)
    {
        scc_graph.add_vertex(i);
    }

    // Create two SCCs: {1,2,3} and {4,5,6}
    scc_graph.add_edge(1, 2, 1.0);
    scc_graph.add_edge(2, 3, 1.0);
    scc_graph.add_edge(3, 1, 1.0);

    scc_graph.add_edge(4, 5, 1.0);
    scc_graph.add_edge(5, 6, 1.0);
    scc_graph.add_edge(6, 4, 1.0);

    auto scc_result = kosaraju_scc(scc_graph);

    EXPECT_EQ(scc_result.num_components, 2);
    EXPECT_EQ(scc_result.components.size(), 2);
}

TEST_F(AlgorithmTest, GraphColoring)
{
    // Create a simple graph
    Graph<int, double> coloring_graph(GraphDirection::UNDIRECTED);

    for (int i = 1; i <= 3; ++i)
    {
        coloring_graph.add_vertex(i);
    }

    // Create a triangle (requires 3 colors)
    coloring_graph.add_edge(1, 2, 1.0);
    coloring_graph.add_edge(2, 3, 1.0);
    coloring_graph.add_edge(3, 1, 1.0);

    auto coloring_result = greedy_coloring(coloring_graph);

    EXPECT_TRUE(coloring_result.is_valid_coloring);
    EXPECT_EQ(coloring_result.num_colors_used, 3);
}

TEST_F(AlgorithmTest, BipartiteCheck)
{
    // Create a bipartite graph
    Graph<int, double> bipartite_graph(GraphDirection::UNDIRECTED);

    for (int i = 1; i <= 4; ++i)
    {
        bipartite_graph.add_vertex(i);
    }

    // Create bipartite graph: {1,3} and {2,4}
    bipartite_graph.add_edge(1, 2, 1.0);
    bipartite_graph.add_edge(1, 4, 1.0);
    bipartite_graph.add_edge(3, 2, 1.0);
    bipartite_graph.add_edge(3, 4, 1.0);

    auto [is_bip, coloring] = is_bipartite(bipartite_graph);

    EXPECT_TRUE(is_bip);
    EXPECT_EQ(coloring.num_colors_used, 2);
}

TEST_F(AlgorithmTest, ConnectedComponents)
{
    // Create disconnected graph
    Graph<int, double> disconnected_graph(GraphDirection::UNDIRECTED);

    for (int i = 1; i <= 6; ++i)
    {
        disconnected_graph.add_vertex(i);
    }

    // Create two components: {1,2,3} and {4,5,6}
    disconnected_graph.add_edge(1, 2, 1.0);
    disconnected_graph.add_edge(2, 3, 1.0);
    disconnected_graph.add_edge(4, 5, 1.0);
    disconnected_graph.add_edge(5, 6, 1.0);

    auto components = connected_components(disconnected_graph);

    EXPECT_EQ(components.size(), 2);

    // Check component sizes
    std::sort(components.begin(), components.end(),
              [](const auto &a, const auto &b)
              {
                  return a.size() < b.size();
              });

    EXPECT_EQ(components[0].size(), 3);
    EXPECT_EQ(components[1].size(), 3);
}

TEST_F(AlgorithmTest, EmptyGraph)
{
    Graph<int, double> empty_graph;

    // Test algorithms on empty graph
    auto dfs_result = dfs(empty_graph, 1);
    EXPECT_TRUE(dfs_result.traversal_order.empty());

    auto bfs_result = bfs(empty_graph, 1);
    EXPECT_TRUE(bfs_result.traversal_order.empty());

    EXPECT_FALSE(has_cycle(empty_graph));

    auto topo_result = topological_sort(empty_graph);
    EXPECT_TRUE(topo_result.is_dag);
    EXPECT_TRUE(topo_result.sorted_order.empty());

    auto path = shortest_path_unweighted(empty_graph, 1, 2);
    EXPECT_TRUE(path.empty());

    auto dijkstra_result = dijkstra(empty_graph, 1, 2);
    EXPECT_FALSE(dijkstra_result.path_exists);

    auto mst_result = kruskal_mst(empty_graph);
    EXPECT_FALSE(mst_result.is_connected);
    EXPECT_TRUE(mst_result.edges.empty());
}
