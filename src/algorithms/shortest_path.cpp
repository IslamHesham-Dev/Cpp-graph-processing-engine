#include "graph_engine/algorithms/shortest_path.hpp"

namespace graph_engine
{
    namespace algorithms
    {

        // Explicit template instantiations for common types
        template ShortestPathResult<int, double> dijkstra<int, double>(const Graph<int, double> &, const int &, const int &, bool);
        template ShortestPathResult<int, double> bellman_ford<int, double>(const Graph<int, double> &, const int &, const int &, bool);
        template std::unordered_map<std::pair<int, int>, double> floyd_warshall<int, double>(const Graph<int, double> &);
        template ShortestPathResult<int, double> a_star<int, double>(const Graph<int, double> &, const int &, const int &, std::function<double(const int &, const int &)>);
        template ShortestPathResult<int, double> bidirectional_dijkstra<int, double>(const Graph<int, double> &, const int &, const int &);

        template ShortestPathResult<std::string, double> dijkstra<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &, bool);
        template ShortestPathResult<std::string, double> bellman_ford<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &, bool);
        template std::unordered_map<std::pair<std::string, std::string>, double> floyd_warshall<std::string, double>(const Graph<std::string, double> &);
        template ShortestPathResult<std::string, double> a_star<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &, std::function<double(const std::string &, const std::string &)>);
        template ShortestPathResult<std::string, double> bidirectional_dijkstra<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &);

    } // namespace algorithms
} // namespace graph_engine
