#include "graph_engine/algorithms/flow.hpp"

namespace graph_engine
{
    namespace algorithms
    {

        // Explicit template instantiations for common types
        template MaxFlowResult<int, double> ford_fulkerson<int, double>(const Graph<int, double> &, const int &, const int &);
        template MaxFlowResult<int, double> edmonds_karp<int, double>(const Graph<int, double> &, const int &, const int &);
        template MaxFlowResult<int, double> dinic<int, double>(const Graph<int, double> &, const int &, const int &);
        template BipartiteMatchingResult<int> max_bipartite_matching<int, double>(const Graph<int, double> &, const std::unordered_set<int> &, const std::unordered_set<int> &);
        template std::pair<double, double> min_cost_max_flow<int, double>(const Graph<int, double> &, const Graph<int, double> &, const int &, const int &);
        template bool is_valid_flow<int, double>(const Graph<int, double> &, const std::unordered_map<std::pair<int, int>, double> &, const int &, const int &);

        template MaxFlowResult<std::string, double> ford_fulkerson<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &);
        template MaxFlowResult<std::string, double> edmonds_karp<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &);
        template MaxFlowResult<std::string, double> dinic<std::string, double>(const Graph<std::string, double> &, const std::string &, const std::string &);
        template BipartiteMatchingResult<std::string> max_bipartite_matching<std::string, double>(const Graph<std::string, double> &, const std::unordered_set<std::string> &, const std::unordered_set<std::string> &);
        template std::pair<double, double> min_cost_max_flow<std::string, double>(const Graph<std::string, double> &, const Graph<std::string, double> &, const std::string &, const std::string &);
        template bool is_valid_flow<std::string, double>(const Graph<std::string, double> &, const std::unordered_map<std::pair<std::string, std::string>, double> &, const std::string &, const std::string &);

    } // namespace algorithms
} // namespace graph_engine
