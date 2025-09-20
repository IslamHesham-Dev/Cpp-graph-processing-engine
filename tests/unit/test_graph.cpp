#include <gtest/gtest.h>
#include "graph_engine/graph.hpp"

using namespace graph_engine;

class GraphTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a test graph
        graph_ = Graph<int, double>(GraphDirection::DIRECTED);

        // Add vertices
        for (int i = 1; i <= 5; ++i)
        {
            graph_.add_vertex(i);
        }

        // Add edges
        graph_.add_edge(1, 2, 1.0);
        graph_.add_edge(1, 3, 2.0);
        graph_.add_edge(2, 4, 3.0);
        graph_.add_edge(3, 4, 1.5);
        graph_.add_edge(4, 5, 2.5);
    }

    Graph<int, double> graph_;
};

TEST_F(GraphTest, BasicOperations)
{
    EXPECT_EQ(graph_.num_vertices(), 5);
    EXPECT_EQ(graph_.num_edges(), 5);
    EXPECT_FALSE(graph_.empty());

    // Test vertex existence
    EXPECT_TRUE(graph_.get_vertices().find(1) != graph_.get_vertices().end());
    EXPECT_TRUE(graph_.get_vertices().find(6) == graph_.get_vertices().end());
}

TEST_F(GraphTest, EdgeOperations)
{
    // Test edge existence
    EXPECT_TRUE(graph_.has_edge(1, 2));
    EXPECT_FALSE(graph_.has_edge(2, 1)); // Directed graph
    EXPECT_FALSE(graph_.has_edge(1, 5));

    // Test edge weights
    EXPECT_DOUBLE_EQ(graph_.get_edge_weight(1, 2), 1.0);
    EXPECT_DOUBLE_EQ(graph_.get_edge_weight(1, 3), 2.0);
    EXPECT_DOUBLE_EQ(graph_.get_edge_weight(1, 5), 0.0); // No edge

    // Test neighbors
    auto neighbors = graph_.get_neighbors(1);
    EXPECT_EQ(neighbors.size(), 2);

    // Test edge removal
    EXPECT_TRUE(graph_.remove_edge(1, 2));
    EXPECT_FALSE(graph_.has_edge(1, 2));
    EXPECT_EQ(graph_.num_edges(), 4);
}

TEST_F(GraphTest, GraphRepresentations)
{
    // Test adjacency list representation
    graph_.convert_to(GraphRepresentation::ADJACENCY_LIST);
    EXPECT_EQ(graph_.get_representation(), GraphRepresentation::ADJACENCY_LIST);
    EXPECT_TRUE(graph_.has_edge(1, 2));

    // Test adjacency matrix representation
    graph_.convert_to(GraphRepresentation::ADJACENCY_MATRIX);
    EXPECT_EQ(graph_.get_representation(), GraphRepresentation::ADJACENCY_MATRIX);
    EXPECT_TRUE(graph_.has_edge(1, 2));

    // Test edge list representation
    graph_.convert_to(GraphRepresentation::EDGE_LIST);
    EXPECT_EQ(graph_.get_representation(), GraphRepresentation::EDGE_LIST);
    EXPECT_TRUE(graph_.has_edge(1, 2));
}

TEST_F(GraphTest, UndirectedGraph)
{
    Graph<int, double> undirected_graph(GraphDirection::UNDIRECTED);

    undirected_graph.add_vertex(1);
    undirected_graph.add_vertex(2);
    undirected_graph.add_edge(1, 2, 1.0);

    EXPECT_TRUE(undirected_graph.has_edge(1, 2));
    EXPECT_TRUE(undirected_graph.has_edge(2, 1)); // Should be true for undirected
    EXPECT_EQ(undirected_graph.num_edges(), 1);   // Only one edge in undirected graph
}

TEST_F(GraphTest, ClearGraph)
{
    EXPECT_FALSE(graph_.empty());
    graph_.clear();
    EXPECT_TRUE(graph_.empty());
    EXPECT_EQ(graph_.num_vertices(), 0);
    EXPECT_EQ(graph_.num_edges(), 0);
}

TEST_F(GraphTest, CopyAndMove)
{
    // Test copy constructor
    Graph<int, double> copied_graph(graph_);
    EXPECT_EQ(copied_graph.num_vertices(), graph_.num_vertices());
    EXPECT_EQ(copied_graph.num_edges(), graph_.num_edges());
    EXPECT_TRUE(copied_graph.has_edge(1, 2));

    // Test move constructor
    Graph<int, double> moved_graph(std::move(copied_graph));
    EXPECT_EQ(moved_graph.num_vertices(), graph_.num_vertices());
    EXPECT_EQ(moved_graph.num_edges(), graph_.num_edges());
    EXPECT_TRUE(moved_graph.has_edge(1, 2));

    // Test assignment operator
    Graph<int, double> assigned_graph;
    assigned_graph = graph_;
    EXPECT_EQ(assigned_graph.num_vertices(), graph_.num_vertices());
    EXPECT_EQ(assigned_graph.num_edges(), graph_.num_edges());
}

TEST_F(GraphTest, StringVertices)
{
    Graph<std::string, double> string_graph(GraphDirection::DIRECTED);

    string_graph.add_vertex("A");
    string_graph.add_vertex("B");
    string_graph.add_vertex("C");

    string_graph.add_edge("A", "B", 1.0);
    string_graph.add_edge("B", "C", 2.0);

    EXPECT_EQ(string_graph.num_vertices(), 3);
    EXPECT_EQ(string_graph.num_edges(), 2);
    EXPECT_TRUE(string_graph.has_edge("A", "B"));
    EXPECT_DOUBLE_EQ(string_graph.get_edge_weight("A", "B"), 1.0);
}

TEST_F(GraphTest, MemoryUsage)
{
    size_type initial_memory = graph_.memory_usage();
    EXPECT_GT(initial_memory, 0);

    // Add more vertices and edges
    graph_.add_vertex(6);
    graph_.add_edge(5, 6, 1.0);

    size_type new_memory = graph_.memory_usage();
    EXPECT_GE(new_memory, initial_memory);
}

TEST_F(GraphTest, ExceptionHandling)
{
    // Test with invalid operations
    Graph<int, double> empty_graph;

    // These should not throw exceptions
    EXPECT_FALSE(empty_graph.has_edge(1, 2));
    EXPECT_DOUBLE_EQ(empty_graph.get_edge_weight(1, 2), 0.0);

    auto neighbors = empty_graph.get_neighbors(1);
    EXPECT_TRUE(neighbors.empty());
}
