#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

#include "utils/union_find.hpp"
#include "utils/memory_pool.hpp"

// Custom hash function for std::pair
namespace std
{
    template <typename T1, typename T2>
    struct hash<std::pair<T1, T2>>
    {
        size_t operator()(const std::pair<T1, T2> &p) const
        {
            size_t h1 = std::hash<T1>{}(p.first);
            size_t h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };
}

namespace graph_engine
{

    /**
     * @brief Exception class for graph-related errors
     */
    class GraphException : public std::exception
    {
    public:
        explicit GraphException(const std::string &message) : message_(message) {}

        const char *what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    /**
     * @brief Edge structure for weighted graphs
     *
     * @tparam VertexType Type of vertex identifiers
     * @tparam WeightType Type of edge weights
     */
    template <typename VertexType, typename WeightType = double>
    struct Edge
    {
        VertexType from;
        VertexType to;
        WeightType weight;

        Edge() = default;
        Edge(VertexType f, VertexType t, WeightType w = WeightType{1})
            : from(f), to(t), weight(w) {}

        bool operator==(const Edge &other) const
        {
            return from == other.from && to == other.to && weight == other.weight;
        }

        bool operator<(const Edge &other) const
        {
            if (weight != other.weight)
                return weight < other.weight;
            if (from != other.from)
                return from < other.from;
            return to < other.to;
        }
    };

    /**
     * @brief Graph representation types
     */
    enum class GraphRepresentation
    {
        ADJACENCY_LIST,   // O(V + E) space, O(1) add edge, O(degree) neighbors
        ADJACENCY_MATRIX, // O(V²) space, O(1) add/remove edge, O(V) neighbors
        EDGE_LIST         // O(E) space, O(1) add edge, O(E) neighbors
    };

    /**
     * @brief Graph direction type
     */
    enum class GraphDirection
    {
        DIRECTED,
        UNDIRECTED
    };

    /**
     * @brief High-performance Graph class with multiple representations
     *
     * This class provides a template-based graph implementation supporting:
     * - Multiple representations (adjacency list, matrix, edge list)
     * - Directed and undirected graphs
     * - Weighted and unweighted edges
     * - Efficient conversion between representations
     * - Memory pool optimization for large graphs
     *
     * @tparam VertexType Type of vertex identifiers
     * @tparam WeightType Type of edge weights
     */
    template <typename VertexType, typename WeightType = double>
    class Graph
    {
    public:
        using vertex_type = VertexType;
        using weight_type = WeightType;
        using edge_type = Edge<VertexType, WeightType>;
        using size_type = std::size_t;

        // Type aliases for different representations
        using AdjacencyList = std::unordered_map<VertexType, std::vector<std::pair<VertexType, WeightType>>>;
        using AdjacencyMatrix = std::vector<std::vector<WeightType>>;
        using EdgeList = std::vector<edge_type>;
        using VertexSet = std::unordered_set<VertexType>;

        /**
         * @brief Construct a new Graph object
         * @param direction Graph direction (directed/undirected)
         * @param representation Initial representation type
         */
        explicit Graph(GraphDirection direction = GraphDirection::DIRECTED,
                       GraphRepresentation representation = GraphRepresentation::ADJACENCY_LIST)
            : direction_(direction), representation_(representation),
              num_vertices_(0), num_edges_(0)
        {
            initialize_representation();
        }

        /**
         * @brief Copy constructor
         */
        Graph(const Graph &other)
            : direction_(other.direction_), representation_(other.representation_),
              num_vertices_(other.num_vertices_), num_edges_(other.num_edges_),
              vertices_(other.vertices_)
        {
            copy_representation(other);
        }

        /**
         * @brief Move constructor
         */
        Graph(Graph &&other) noexcept
            : direction_(other.direction_), representation_(other.representation_),
              num_vertices_(other.num_vertices_), num_edges_(other.num_edges_),
              vertices_(std::move(other.vertices_))
        {
            move_representation(std::move(other));
        }

        /**
         * @brief Copy assignment operator
         */
        Graph &operator=(const Graph &other)
        {
            if (this != &other)
            {
                direction_ = other.direction_;
                representation_ = other.representation_;
                num_vertices_ = other.num_vertices_;
                num_edges_ = other.num_edges_;
                vertices_ = other.vertices_;
                copy_representation(other);
            }
            return *this;
        }

        /**
         * @brief Move assignment operator
         */
        Graph &operator=(Graph &&other) noexcept
        {
            if (this != &other)
            {
                direction_ = other.direction_;
                representation_ = other.representation_;
                num_vertices_ = other.num_vertices_;
                num_edges_ = other.num_edges_;
                vertices_ = std::move(other.vertices_);
                move_representation(std::move(other));
            }
            return *this;
        }

        /**
         * @brief Add a vertex to the graph
         * @param vertex The vertex to add
         */
        void add_vertex(const VertexType &vertex)
        {
            vertices_.insert(vertex);
            num_vertices_ = vertices_.size();

            // Update representations
            if (representation_ == GraphRepresentation::ADJACENCY_LIST)
            {
                adjacency_list_[vertex]; // Create empty adjacency list
            }
            else if (representation_ == GraphRepresentation::ADJACENCY_MATRIX)
            {
                update_matrix_size();
            }
        }

        /**
         * @brief Add an edge to the graph
         * @param from Source vertex
         * @param to Destination vertex
         * @param weight Edge weight
         */
        void add_edge(const VertexType &from, const VertexType &to, WeightType weight = WeightType{1})
        {
            // Add vertices if they don't exist
            add_vertex(from);
            add_vertex(to);

            // Add edge based on current representation
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                add_edge_adjacency_list(from, to, weight);
                break;
            case GraphRepresentation::ADJACENCY_MATRIX:
                add_edge_adjacency_matrix(from, to, weight);
                break;
            case GraphRepresentation::EDGE_LIST:
                add_edge_edge_list(from, to, weight);
                break;
            }

            // Add reverse edge for undirected graphs
            if (direction_ == GraphDirection::UNDIRECTED)
            {
                switch (representation_)
                {
                case GraphRepresentation::ADJACENCY_LIST:
                    add_edge_adjacency_list(to, from, weight);
                    break;
                case GraphRepresentation::ADJACENCY_MATRIX:
                    add_edge_adjacency_matrix(to, from, weight);
                    break;
                case GraphRepresentation::EDGE_LIST:
                    add_edge_edge_list(to, from, weight);
                    break;
                }
            }

            num_edges_++;
        }

        /**
         * @brief Remove an edge from the graph
         * @param from Source vertex
         * @param to Destination vertex
         * @return true if edge was removed, false if edge didn't exist
         */
        bool remove_edge(const VertexType &from, const VertexType &to)
        {
            bool removed = false;

            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                removed = remove_edge_adjacency_list(from, to);
                break;
            case GraphRepresentation::ADJACENCY_MATRIX:
                removed = remove_edge_adjacency_matrix(from, to);
                break;
            case GraphRepresentation::EDGE_LIST:
                removed = remove_edge_edge_list(from, to);
                break;
            }

            if (removed)
            {
                num_edges_--;

                // Remove reverse edge for undirected graphs
                if (direction_ == GraphDirection::UNDIRECTED)
                {
                    switch (representation_)
                    {
                    case GraphRepresentation::ADJACENCY_LIST:
                        remove_edge_adjacency_list(to, from);
                        break;
                    case GraphRepresentation::ADJACENCY_MATRIX:
                        remove_edge_adjacency_matrix(to, from);
                        break;
                    case GraphRepresentation::EDGE_LIST:
                        remove_edge_edge_list(to, from);
                        break;
                    }
                }
            }

            return removed;
        }

        /**
         * @brief Check if an edge exists
         * @param from Source vertex
         * @param to Destination vertex
         * @return true if edge exists
         */
        bool has_edge(const VertexType &from, const VertexType &to) const
        {
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                return has_edge_adjacency_list(from, to);
            case GraphRepresentation::ADJACENCY_MATRIX:
                return has_edge_adjacency_matrix(from, to);
            case GraphRepresentation::EDGE_LIST:
                return has_edge_edge_list(from, to);
            }
            return false;
        }

        /**
         * @brief Get edge weight
         * @param from Source vertex
         * @param to Destination vertex
         * @return Edge weight, or default weight if edge doesn't exist
         */
        WeightType get_edge_weight(const VertexType &from, const VertexType &to) const
        {
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                return get_edge_weight_adjacency_list(from, to);
            case GraphRepresentation::ADJACENCY_MATRIX:
                return get_edge_weight_adjacency_matrix(from, to);
            case GraphRepresentation::EDGE_LIST:
                return get_edge_weight_edge_list(from, to);
            }
            return WeightType{};
        }

        /**
         * @brief Get neighbors of a vertex
         * @param vertex The vertex
         * @return Vector of neighbor vertices with weights
         */
        std::vector<std::pair<VertexType, WeightType>> get_neighbors(const VertexType &vertex) const
        {
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                return get_neighbors_adjacency_list(vertex);
            case GraphRepresentation::ADJACENCY_MATRIX:
                return get_neighbors_adjacency_matrix(vertex);
            case GraphRepresentation::EDGE_LIST:
                return get_neighbors_edge_list(vertex);
            }
            return {};
        }

        /**
         * @brief Get all vertices
         * @return Set of all vertices
         */
        const VertexSet &get_vertices() const
        {
            return vertices_;
        }

        /**
         * @brief Get all edges
         * @return Vector of all edges
         */
        std::vector<edge_type> get_edges() const
        {
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                return get_edges_adjacency_list();
            case GraphRepresentation::ADJACENCY_MATRIX:
                return get_edges_adjacency_matrix();
            case GraphRepresentation::EDGE_LIST:
                return edge_list_;
            }
            return {};
        }

        /**
         * @brief Get number of vertices
         * @return Number of vertices
         */
        size_type num_vertices() const
        {
            return num_vertices_;
        }

        /**
         * @brief Get number of edges
         * @return Number of edges
         */
        size_type num_edges() const
        {
            return num_edges_;
        }

        /**
         * @brief Check if graph is empty
         * @return true if graph has no vertices
         */
        bool empty() const
        {
            return vertices_.empty();
        }

        /**
         * @brief Clear the graph
         */
        void clear()
        {
            vertices_.clear();
            num_vertices_ = 0;
            num_edges_ = 0;
            initialize_representation();
        }

        /**
         * @brief Convert to a different representation
         * @param new_representation Target representation
         */
        void convert_to(GraphRepresentation new_representation)
        {
            if (representation_ == new_representation)
            {
                return;
            }

            // Store current data
            auto current_edges = get_edges();
            auto current_vertices = vertices_;

            // Clear current representation
            clear_representation();

            // Set new representation
            representation_ = new_representation;
            vertices_ = current_vertices;
            num_vertices_ = vertices_.size();

            // Initialize new representation
            initialize_representation();

            // Rebuild graph with new representation
            for (const auto &edge : current_edges)
            {
                add_edge(edge.from, edge.to, edge.weight);
            }
        }

        /**
         * @brief Get current representation type
         * @return Current representation
         */
        GraphRepresentation get_representation() const
        {
            return representation_;
        }

        /**
         * @brief Get graph direction
         * @return Graph direction
         */
        GraphDirection get_direction() const
        {
            return direction_;
        }

        /**
         * @brief Set graph direction
         * @param direction New direction
         */
        void set_direction(GraphDirection direction)
        {
            if (direction_ != direction)
            {
                direction_ = direction;
                // Rebuild graph to handle direction change
                auto current_edges = get_edges();
                clear();
                for (const auto &edge : current_edges)
                {
                    add_edge(edge.from, edge.to, edge.weight);
                }
            }
        }

        /**
         * @brief Load graph from file
         * @param filename File path
         * @param format File format ("adjacency_list", "edge_list", "matrix")
         */
        void load_from_file(const std::string &filename, const std::string &format = "edge_list")
        {
            std::ifstream file(filename);
            if (!file.is_open())
            {
                throw GraphException("Cannot open file: " + filename);
            }

            clear();

            if (format == "edge_list")
            {
                load_edge_list_format(file);
            }
            else if (format == "adjacency_list")
            {
                load_adjacency_list_format(file);
            }
            else if (format == "matrix")
            {
                load_matrix_format(file);
            }
            else
            {
                throw GraphException("Unsupported format: " + format);
            }

            file.close();
        }

        /**
         * @brief Save graph to file
         * @param filename File path
         * @param format File format ("adjacency_list", "edge_list", "matrix")
         */
        void save_to_file(const std::string &filename, const std::string &format = "edge_list") const
        {
            std::ofstream file(filename);
            if (!file.is_open())
            {
                throw GraphException("Cannot create file: " + filename);
            }

            if (format == "edge_list")
            {
                save_edge_list_format(file);
            }
            else if (format == "adjacency_list")
            {
                save_adjacency_list_format(file);
            }
            else if (format == "matrix")
            {
                save_matrix_format(file);
            }
            else
            {
                throw GraphException("Unsupported format: " + format);
            }

            file.close();
        }

        /**
         * @brief Get memory usage statistics
         * @return Memory usage in bytes
         */
        size_type memory_usage() const
        {
            size_type usage = 0;

            // Base structure
            usage += sizeof(*this);
            usage += vertices_.size() * sizeof(VertexType);

            // Representation-specific usage
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                usage += adjacency_list_.size() * sizeof(std::pair<VertexType, std::vector<std::pair<VertexType, WeightType>>>);
                for (const auto &pair : adjacency_list_)
                {
                    usage += pair.second.size() * sizeof(std::pair<VertexType, WeightType>);
                }
                break;
            case GraphRepresentation::ADJACENCY_MATRIX:
                usage += adjacency_matrix_.size() * sizeof(std::vector<WeightType>);
                for (const auto &row : adjacency_matrix_)
                {
                    usage += row.size() * sizeof(WeightType);
                }
                break;
            case GraphRepresentation::EDGE_LIST:
                usage += edge_list_.size() * sizeof(edge_type);
                break;
            }

            return usage;
        }

        /**
         * @brief Print graph statistics
         */
        void print_stats() const
        {
            std::cout << "Graph Statistics:\n";
            std::cout << "  Vertices: " << num_vertices_ << "\n";
            std::cout << "  Edges: " << num_edges_ << "\n";
            std::cout << "  Direction: " << (direction_ == GraphDirection::DIRECTED ? "Directed" : "Undirected") << "\n";
            std::cout << "  Representation: ";
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                std::cout << "Adjacency List\n";
                break;
            case GraphRepresentation::ADJACENCY_MATRIX:
                std::cout << "Adjacency Matrix\n";
                break;
            case GraphRepresentation::EDGE_LIST:
                std::cout << "Edge List\n";
                break;
            }
            std::cout << "  Memory Usage: " << memory_usage() << " bytes\n";
        }

    private:
        GraphDirection direction_;
        GraphRepresentation representation_;
        size_type num_vertices_;
        size_type num_edges_;
        VertexSet vertices_;

        // Representation-specific data
        AdjacencyList adjacency_list_;
        AdjacencyMatrix adjacency_matrix_;
        EdgeList edge_list_;

        // Vertex to index mapping for matrix representation
        std::unordered_map<VertexType, size_type> vertex_to_index_;
        std::unordered_map<size_type, VertexType> index_to_vertex_;

        void initialize_representation()
        {
            switch (representation_)
            {
            case GraphRepresentation::ADJACENCY_LIST:
                adjacency_list_.clear();
                break;
            case GraphRepresentation::ADJACENCY_MATRIX:
                adjacency_matrix_.clear();
                vertex_to_index_.clear();
                index_to_vertex_.clear();
                break;
            case GraphRepresentation::EDGE_LIST:
                edge_list_.clear();
                break;
            }
        }

        void clear_representation()
        {
            adjacency_list_.clear();
            adjacency_matrix_.clear();
            edge_list_.clear();
            vertex_to_index_.clear();
            index_to_vertex_.clear();
        }

        void copy_representation(const Graph &other)
        {
            adjacency_list_ = other.adjacency_list_;
            adjacency_matrix_ = other.adjacency_matrix_;
            edge_list_ = other.edge_list_;
            vertex_to_index_ = other.vertex_to_index_;
            index_to_vertex_ = other.index_to_vertex_;
        }

        void move_representation(Graph &&other)
        {
            adjacency_list_ = std::move(other.adjacency_list_);
            adjacency_matrix_ = std::move(other.adjacency_matrix_);
            edge_list_ = std::move(other.edge_list_);
            vertex_to_index_ = std::move(other.vertex_to_index_);
            index_to_vertex_ = std::move(other.index_to_vertex_);
        }

        // Adjacency list methods
        void add_edge_adjacency_list(const VertexType &from, const VertexType &to, WeightType weight)
        {
            adjacency_list_[from].emplace_back(to, weight);
        }

        bool remove_edge_adjacency_list(const VertexType &from, const VertexType &to)
        {
            auto it = adjacency_list_.find(from);
            if (it == adjacency_list_.end())
                return false;

            auto &neighbors = it->second;
            auto edge_it = std::find_if(neighbors.begin(), neighbors.end(),
                                        [&to](const auto &pair)
                                        { return pair.first == to; });

            if (edge_it != neighbors.end())
            {
                neighbors.erase(edge_it);
                return true;
            }
            return false;
        }

        bool has_edge_adjacency_list(const VertexType &from, const VertexType &to) const
        {
            auto it = adjacency_list_.find(from);
            if (it == adjacency_list_.end())
                return false;

            return std::any_of(it->second.begin(), it->second.end(),
                               [&to](const auto &pair)
                               { return pair.first == to; });
        }

        WeightType get_edge_weight_adjacency_list(const VertexType &from, const VertexType &to) const
        {
            auto it = adjacency_list_.find(from);
            if (it == adjacency_list_.end())
                return WeightType{};

            auto edge_it = std::find_if(it->second.begin(), it->second.end(),
                                        [&to](const auto &pair)
                                        { return pair.first == to; });

            return edge_it != it->second.end() ? edge_it->second : WeightType{};
        }

        std::vector<std::pair<VertexType, WeightType>> get_neighbors_adjacency_list(const VertexType &vertex) const
        {
            auto it = adjacency_list_.find(vertex);
            return it != adjacency_list_.end() ? it->second : std::vector<std::pair<VertexType, WeightType>>{};
        }

        std::vector<edge_type> get_edges_adjacency_list() const
        {
            std::vector<edge_type> edges;
            for (const auto &pair : adjacency_list_)
            {
                for (const auto &neighbor : pair.second)
                {
                    edges.emplace_back(pair.first, neighbor.first, neighbor.second);
                }
            }
            return edges;
        }

        // Adjacency matrix methods
        void update_matrix_size()
        {
            size_type new_size = vertices_.size();
            adjacency_matrix_.resize(new_size, std::vector<WeightType>(new_size, WeightType{}));

            // Update vertex mappings
            size_type index = 0;
            for (const auto &vertex : vertices_)
            {
                if (vertex_to_index_.find(vertex) == vertex_to_index_.end())
                {
                    vertex_to_index_[vertex] = index;
                    index_to_vertex_[index] = vertex;
                    index++;
                }
            }
        }

        void add_edge_adjacency_matrix(const VertexType &from, const VertexType &to, WeightType weight)
        {
            update_matrix_size();
            size_type from_idx = vertex_to_index_[from];
            size_type to_idx = vertex_to_index_[to];
            adjacency_matrix_[from_idx][to_idx] = weight;
        }

        bool remove_edge_adjacency_matrix(const VertexType &from, const VertexType &to)
        {
            auto from_it = vertex_to_index_.find(from);
            auto to_it = vertex_to_index_.find(to);
            if (from_it == vertex_to_index_.end() || to_it == vertex_to_index_.end())
            {
                return false;
            }

            size_type from_idx = from_it->second;
            size_type to_idx = to_it->second;

            if (adjacency_matrix_[from_idx][to_idx] != WeightType{})
            {
                adjacency_matrix_[from_idx][to_idx] = WeightType{};
                return true;
            }
            return false;
        }

        bool has_edge_adjacency_matrix(const VertexType &from, const VertexType &to) const
        {
            auto from_it = vertex_to_index_.find(from);
            auto to_it = vertex_to_index_.find(to);
            if (from_it == vertex_to_index_.end() || to_it == vertex_to_index_.end())
            {
                return false;
            }

            return adjacency_matrix_[from_it->second][to_it->second] != WeightType{};
        }

        WeightType get_edge_weight_adjacency_matrix(const VertexType &from, const VertexType &to) const
        {
            auto from_it = vertex_to_index_.find(from);
            auto to_it = vertex_to_index_.find(to);
            if (from_it == vertex_to_index_.end() || to_it == vertex_to_index_.end())
            {
                return WeightType{};
            }

            return adjacency_matrix_[from_it->second][to_it->second];
        }

        std::vector<std::pair<VertexType, WeightType>> get_neighbors_adjacency_matrix(const VertexType &vertex) const
        {
            auto it = vertex_to_index_.find(vertex);
            if (it == vertex_to_index_.end())
                return {};

            size_type vertex_idx = it->second;
            std::vector<std::pair<VertexType, WeightType>> neighbors;

            for (size_type i = 0; i < adjacency_matrix_[vertex_idx].size(); ++i)
            {
                if (adjacency_matrix_[vertex_idx][i] != WeightType{})
                {
                    neighbors.emplace_back(index_to_vertex_.at(i), adjacency_matrix_[vertex_idx][i]);
                }
            }

            return neighbors;
        }

        std::vector<edge_type> get_edges_adjacency_matrix() const
        {
            std::vector<edge_type> edges;
            for (size_type i = 0; i < adjacency_matrix_.size(); ++i)
            {
                for (size_type j = 0; j < adjacency_matrix_[i].size(); ++j)
                {
                    if (adjacency_matrix_[i][j] != WeightType{})
                    {
                        edges.emplace_back(index_to_vertex_.at(i), index_to_vertex_.at(j), adjacency_matrix_[i][j]);
                    }
                }
            }
            return edges;
        }

        // Edge list methods
        void add_edge_edge_list(const VertexType &from, const VertexType &to, WeightType weight)
        {
            edge_list_.emplace_back(from, to, weight);
        }

        bool remove_edge_edge_list(const VertexType &from, const VertexType &to)
        {
            auto it = std::find_if(edge_list_.begin(), edge_list_.end(),
                                   [&from, &to](const edge_type &edge)
                                   {
                                       return edge.from == from && edge.to == to;
                                   });

            if (it != edge_list_.end())
            {
                edge_list_.erase(it);
                return true;
            }
            return false;
        }

        bool has_edge_edge_list(const VertexType &from, const VertexType &to) const
        {
            return std::any_of(edge_list_.begin(), edge_list_.end(),
                               [&from, &to](const edge_type &edge)
                               {
                                   return edge.from == from && edge.to == to;
                               });
        }

        WeightType get_edge_weight_edge_list(const VertexType &from, const VertexType &to) const
        {
            auto it = std::find_if(edge_list_.begin(), edge_list_.end(),
                                   [&from, &to](const edge_type &edge)
                                   {
                                       return edge.from == from && edge.to == to;
                                   });

            return it != edge_list_.end() ? it->weight : WeightType{};
        }

        std::vector<std::pair<VertexType, WeightType>> get_neighbors_edge_list(const VertexType &vertex) const
        {
            std::vector<std::pair<VertexType, WeightType>> neighbors;
            for (const auto &edge : edge_list_)
            {
                if (edge.from == vertex)
                {
                    neighbors.emplace_back(edge.to, edge.weight);
                }
            }
            return neighbors;
        }

        // File I/O methods
        void load_edge_list_format(std::ifstream &file)
        {
            std::string line;
            while (std::getline(file, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;

                std::istringstream iss(line);
                VertexType from, to;
                WeightType weight = WeightType{1};

                if (iss >> from >> to >> weight)
                {
                    add_edge(from, to, weight);
                }
                else if (iss >> from >> to)
                {
                    add_edge(from, to, weight);
                }
            }
        }

        void load_adjacency_list_format(std::ifstream &file)
        {
            std::string line;
            while (std::getline(file, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;

                std::istringstream iss(line);
                VertexType vertex;
                if (iss >> vertex)
                {
                    add_vertex(vertex);

                    VertexType neighbor;
                    WeightType weight = WeightType{1};
                    while (iss >> neighbor >> weight)
                    {
                        add_edge(vertex, neighbor, weight);
                    }
                }
            }
        }

        void load_matrix_format(std::ifstream &file)
        {
            std::vector<std::vector<WeightType>> matrix;
            std::string line;

            while (std::getline(file, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;

                std::istringstream iss(line);
                std::vector<WeightType> row;
                WeightType value;

                while (iss >> value)
                {
                    row.push_back(value);
                }

                if (!row.empty())
                {
                    matrix.push_back(row);
                }
            }

            // Convert matrix to graph
            // For matrix format, we assume vertices are numeric indices
            // This function only works for numeric vertex types
            if constexpr (std::is_arithmetic_v<VertexType>)
            {
                for (size_type i = 0; i < matrix.size(); ++i)
                {
                    add_vertex(static_cast<VertexType>(i));
                    for (size_type j = 0; j < matrix[i].size(); ++j)
                    {
                        if (matrix[i][j] != WeightType{})
                        {
                            add_edge(static_cast<VertexType>(i), static_cast<VertexType>(j), matrix[i][j]);
                        }
                    }
                }
            }
            else
            {
                throw GraphException("Matrix format loading only supported for numeric vertex types");
            }
        }

        void save_edge_list_format(std::ofstream &file) const
        {
            for (const auto &edge : get_edges())
            {
                file << edge.from << " " << edge.to << " " << edge.weight << "\n";
            }
        }

        void save_adjacency_list_format(std::ofstream &file) const
        {
            for (const auto &vertex : vertices_)
            {
                file << vertex;
                for (const auto &neighbor : get_neighbors(vertex))
                {
                    file << " " << neighbor.first << " " << neighbor.second;
                }
                file << "\n";
            }
        }

        void save_matrix_format(std::ofstream &file) const
        {
            if (representation_ != GraphRepresentation::ADJACENCY_MATRIX)
            {
                // Convert to matrix representation temporarily
                auto temp_graph = *this;
                temp_graph.convert_to(GraphRepresentation::ADJACENCY_MATRIX);

                for (const auto &row : temp_graph.adjacency_matrix_)
                {
                    for (size_type j = 0; j < row.size(); ++j)
                    {
                        if (j > 0)
                            file << " ";
                        file << row[j];
                    }
                    file << "\n";
                }
            }
            else
            {
                for (const auto &row : adjacency_matrix_)
                {
                    for (size_type j = 0; j < row.size(); ++j)
                    {
                        if (j > 0)
                            file << " ";
                        file << row[j];
                    }
                    file << "\n";
                }
            }
        }
    };

} // namespace graph_engine
