#include "graph_engine/algorithms/traversal.hpp"

namespace graph_engine
{
    namespace algorithms
    {

        // Explicit template instantiations for common types
        template DFSResult<int> dfs<int, double>(const Graph<int, double> &, const int &, std::function<void(const int &)>);
        template DFSResult<int> dfs_all<int, double>(const Graph<int, double> &, std::function<void(const int &)>);
        template BFSResult<int> bfs<int, double>(const Graph<int, double> &, const int &, std::function<void(const int &)>);
        template BFSResult<int> bfs_all<int, double>(const Graph<int, double> &, std::function<void(const int &)>);
        template bool has_cycle<int, double>(const Graph<int, double> &);
        template TopologicalResult<int> topological_sort<int, double>(const Graph<int, double> &);
        template std::vector<int> shortest_path_unweighted<int, double>(const Graph<int, double> &, const int &, const int &);
        template std::vector<std::vector<int>> connected_components<int, double>(const Graph<int, double> &);

        template DFSResult<std::string> dfs<std::string, double>(const Graph<std::string, double> &, const std::string &, std::function<void(const std::string &)>);
        template DFSResult<std::string> dfs_all<std::string, double>(const Graph<std::string, double> &, std::function<void(const std::string &)>);
        template BFSResult<std::string> bfs<std::string, double>(const Graph<std::string, double> &, const std::string &, std::function<void(const std::string &)>);
        template BFSResult<std::string> bfs_all<std::string, double>(const Graph<std::string, double> &, std::function<void(const std::string &)>);
        template bool has_cycle<std::string, double>(const Graph<std::string, double> &);
        template TopologicalResult<std::string> topological_sort<std::string, double>(const Graph<std::string, double> &);
        template std::vector<std::string> shortest_path_unweighted<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &);
        template std::vector<std::vector<std::string>> connected_components<std::string, double>(const Graph<std::string, double> &);

    } // namespace algorithms
} // namespace graph_engine
