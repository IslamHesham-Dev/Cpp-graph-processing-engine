#include "graph_engine/graph.hpp"

namespace graph_engine
{

    // Explicit template instantiations for common types
    template class Graph<int, double>;
    template class Graph<int, float>;
    template class Graph<int, int>;
    template class Graph<std::string, double>;
    template class Graph<std::string, float>;
    template class Graph<std::string, int>;

    // Edge template instantiations
    template struct Edge<int, double>;
    template struct Edge<int, float>;
    template struct Edge<int, int>;
    template struct Edge<std::string, double>;
    template struct Edge<std::string, float>;
    template struct Edge<std::string, int>;

} // namespace graph_engine
