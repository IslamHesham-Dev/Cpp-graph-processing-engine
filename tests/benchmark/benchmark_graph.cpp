#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include "graph_engine/graph.hpp"
#include "graph_engine/algorithms/traversal.hpp"
#include "graph_engine/algorithms/shortest_path.hpp"
#include "graph_engine/algorithms/mst.hpp"

using namespace graph_engine;
using namespace graph_engine::algorithms;

// Helper function to create a random graph
Graph<int, double> create_random_graph(int num_vertices, int num_edges, bool directed = true)
{
    Graph<int, double> graph(directed ? GraphDirection::DIRECTED : GraphDirection::UNDIRECTED);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> vertex_dist(0, num_vertices - 1);
    std::uniform_real_distribution<> weight_dist(0.1, 10.0);

    // Add vertices
    for (int i = 0; i < num_vertices; ++i)
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

    return graph;
}

// Benchmark graph construction
static void BM_GraphConstruction(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    for (auto _ : state)
    {
        auto graph = create_random_graph(num_vertices, num_edges);
        benchmark::DoNotOptimize(graph);
    }

    state.SetComplexityN(num_vertices + num_edges);
}

// Benchmark graph representation conversion
static void BM_GraphConversion(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges);

    for (auto _ : state)
    {
        graph.convert_to(GraphRepresentation::ADJACENCY_LIST);
        graph.convert_to(GraphRepresentation::ADJACENCY_MATRIX);
        graph.convert_to(GraphRepresentation::EDGE_LIST);
        benchmark::DoNotOptimize(graph);
    }

    state.SetComplexityN(num_vertices + num_edges);
}

// Benchmark DFS traversal
static void BM_DFSTraversal(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges);

    for (auto _ : state)
    {
        auto result = dfs(graph, 0);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_vertices + num_edges);
}

// Benchmark BFS traversal
static void BM_BFSTraversal(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges);

    for (auto _ : state)
    {
        auto result = bfs(graph, 0);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_vertices + num_edges);
}

// Benchmark Dijkstra's algorithm
static void BM_Dijkstra(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges);

    for (auto _ : state)
    {
        auto result = dijkstra(graph, 0, num_vertices - 1);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_vertices + num_edges);
}

// Benchmark Bellman-Ford algorithm
static void BM_BellmanFord(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges);

    for (auto _ : state)
    {
        auto result = bellman_ford(graph, 0, num_vertices - 1);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_vertices * num_edges);
}

// Benchmark Floyd-Warshall algorithm
static void BM_FloydWarshall(benchmark::State &state)
{
    int num_vertices = state.range(0);

    auto graph = create_random_graph(num_vertices, num_vertices * 2);

    for (auto _ : state)
    {
        auto result = floyd_warshall(graph);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_vertices * num_vertices * num_vertices);
}

// Benchmark Kruskal's MST algorithm
static void BM_KruskalMST(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges, false); // Undirected

    for (auto _ : state)
    {
        auto result = kruskal_mst(graph);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_edges * std::log(num_edges));
}

// Benchmark Prim's MST algorithm
static void BM_PrimMST(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges, false); // Undirected

    for (auto _ : state)
    {
        auto result = prim_mst(graph, 0);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_edges * std::log(num_vertices));
}

// Benchmark topological sort
static void BM_TopologicalSort(benchmark::State &state)
{
    int num_vertices = state.range(0);
    int num_edges = state.range(1);

    auto graph = create_random_graph(num_vertices, num_edges);

    for (auto _ : state)
    {
        auto result = topological_sort(graph);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(num_vertices + num_edges);
}

// Register benchmarks
BENCHMARK(BM_GraphConstruction)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oN);

BENCHMARK(BM_GraphConversion)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oN);

BENCHMARK(BM_DFSTraversal)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oN);

BENCHMARK(BM_BFSTraversal)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oN);

BENCHMARK(BM_Dijkstra)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oNLogN);

BENCHMARK(BM_BellmanFord)
    ->Args({50, 250})
    ->Args({100, 500})
    ->Args({200, 1000})
    ->Args({500, 2500})
    ->Complexity(benchmark::oN2);

BENCHMARK(BM_FloydWarshall)
    ->Args({50, 100})
    ->Args({100, 200})
    ->Args({200, 400})
    ->Args({300, 600})
    ->Complexity(benchmark::oN3);

BENCHMARK(BM_KruskalMST)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oNLogN);

BENCHMARK(BM_PrimMST)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oNLogN);

BENCHMARK(BM_TopologicalSort)
    ->Args({100, 500})
    ->Args({500, 2500})
    ->Args({1000, 5000})
    ->Args({2000, 10000})
    ->Complexity(benchmark::oN);

BENCHMARK_MAIN();
