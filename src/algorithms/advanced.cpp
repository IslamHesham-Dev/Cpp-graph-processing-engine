#include "graph_engine/algorithms/advanced.hpp"

namespace graph_engine
{
    namespace algorithms
    {

        // Explicit template instantiations for common types
        template SCCResult<int> kosaraju_scc<int, double>(const Graph<int, double> &);
        template SCCResult<int> tarjan_scc<int, double>(const Graph<int, double> &);
        template ColoringResult<int> greedy_coloring<int, double>(const Graph<int, double> &, int);
        template ColoringResult<int> welsh_powell_coloring<int, double>(const Graph<int, double> &, int);
        template std::pair<bool, ColoringResult<int>> is_bipartite<int, double>(const Graph<int, double> &);
        template int chromatic_number<int, double>(const Graph<int, double> &);
        template std::unordered_set<int> find_articulation_points<int, double>(const Graph<int, double> &);
        template std::vector<Edge<int, double>> find_bridges<int, double>(const Graph<int, double> &);

        template SCCResult<std::string> kosaraju_scc<std::string, double>(const Graph<std::string, double> &);
        template SCCResult<std::string> tarjan_scc<std::string, double>(const Graph<std::string, double> &);
        template ColoringResult<std::string> greedy_coloring<std::string, double>(const Graph<std::string, double> &, int);
        template ColoringResult<std::string> welsh_powell_coloring<std::string, double>(const Graph<std::string, double> &, int);
        template std::pair<bool, ColoringResult<std::string>> is_bipartite<std::string, double>(const Graph<std::string, double> &);
        template int chromatic_number<std::string, double>(const Graph<std::string, double> &);
        template std::unordered_set<std::string> find_articulation_points<std::string, double>(const Graph<std::string, double> &);
        template std::vector<Edge<std::string, double>> find_bridges<std::string, double>(const Graph<std::string, double> &);

    } // namespace algorithms
} // namespace graph_engine
