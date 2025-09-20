#include "graph_engine/algorithms/mst.hpp"

namespace graph_engine
{
    namespace algorithms
    {

        // Explicit template instantiations for common types
        template MSTResult<int, double> kruskal_mst<int, double>(const Graph<int, double> &);
        template MSTResult<int, double> prim_mst<int, double>(const Graph<int, double> &, const int &, bool);
        template MSTResult<int, double> boruvka_mst<int, double>(const Graph<int, double> &);
        template std::vector<MSTResult<int, double>> find_all_msts<int, double>(const Graph<int, double> &);
        template MSTResult<int, double> second_best_mst<int, double>(const Graph<int, double> &);
        template bool is_valid_mst<int, double>(const Graph<int, double> &, const std::vector<Edge<int, double>> &);
        template MSTResult<int, double> steiner_tree_approximation<int, double>(const Graph<int, double> &, const std::unordered_set<int> &);

        template MSTResult<std::string, double> kruskal_mst<std::string, double>(const Graph<std::string, double> &);
        template MSTResult<std::string, double> prim_mst<std::string, double>(const Graph<std::string, double> &, const std::string &, bool);
        template MSTResult<std::string, double> boruvka_mst<std::string, double>(const Graph<std::string, double> &);
        template std::vector<MSTResult<std::string, double>> find_all_msts<std::string, double>(const Graph<std::string, double> &);
        template MSTResult<std::string, double> second_best_mst<std::string, double>(const Graph<std::string, double> &);
        template bool is_valid_mst<std::string, double>(const Graph<std::string, double> &, const std::vector<Edge<std::string, double>> &);
        template MSTResult<std::string, double> steiner_tree_approximation<std::string, double>(const Graph<std::string, double> &, const std::unordered_set<std::string> &);

    } // namespace algorithms
} // namespace graph_engine
