#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <set>

#include "graph_engine/graph.hpp"
#include "graph_engine/algorithms/traversal.hpp"
#include "graph_engine/algorithms/advanced.hpp"

using namespace graph_engine;
using namespace graph_engine::algorithms;

struct Package
{
    std::string name;
    std::string version;
    std::vector<std::string> dependencies;
    std::string description;

    Package() = default;
    Package(const std::string &n, const std::string &v, const std::vector<std::string> &deps, const std::string &desc)
        : name(n), version(v), dependencies(deps), description(desc) {}
};

class DependencyResolver
{
private:
    Graph<std::string, double> dependency_graph_;
    std::unordered_map<std::string, Package> packages_;

public:
    DependencyResolver() : dependency_graph_(GraphDirection::DIRECTED) {}

    void add_package(const std::string &name, const std::string &version,
                     const std::vector<std::string> &dependencies, const std::string &description)
    {
        packages_[name] = Package(name, version, dependencies, description);
        dependency_graph_.add_vertex(name);

        // Add dependency edges
        for (const auto &dep : dependencies)
        {
            dependency_graph_.add_edge(name, dep, 1.0);
        }
    }

    void remove_package(const std::string &name)
    {
        packages_.erase(name);
        dependency_graph_.remove_edge(name, name); // Remove self-loop if exists

        // Remove all edges involving this package
        auto edges = dependency_graph_.get_edges();
        for (const auto &edge : edges)
        {
            if (edge.from == name || edge.to == name)
            {
                dependency_graph_.remove_edge(edge.from, edge.to);
            }
        }
    }

    std::vector<std::string> resolve_dependencies(const std::string &package_name) const
    {
        // Use topological sort to get installation order
        auto topo_result = topological_sort(dependency_graph_);

        if (!topo_result.is_dag)
        {
            std::cerr << "Error: Circular dependencies detected!\n";
            return {};
        }

        // Find the package and all its dependencies in the sorted order
        std::vector<std::string> installation_order;
        std::unordered_set<std::string> required_packages;

        // Collect all required packages using DFS
        std::function<void(const std::string &)> collect_dependencies = [&](const std::string &pkg)
        {
            if (required_packages.find(pkg) != required_packages.end())
            {
                return; // Already processed
            }

            required_packages.insert(pkg);
            auto neighbors = dependency_graph_.get_neighbors(pkg);
            for (const auto &neighbor : neighbors)
            {
                collect_dependencies(neighbor.first);
            }
        };

        collect_dependencies(package_name);

        // Build installation order based on topological sort
        for (const auto &pkg : topo_result.sorted_order)
        {
            if (required_packages.find(pkg) != required_packages.end())
            {
                installation_order.push_back(pkg);
            }
        }

        return installation_order;
    }

    std::vector<std::string> detect_circular_dependencies() const
    {
        auto topo_result = topological_sort(dependency_graph_);

        if (topo_result.is_dag)
        {
            return {}; // No circular dependencies
        }

        return topo_result.cycle_vertices;
    }

    std::vector<std::string> find_affected_packages(const std::string &package_name) const
    {
        // Find all packages that depend on the given package
        std::vector<std::string> affected;

        for (const auto &edge : dependency_graph_.get_edges())
        {
            if (edge.to == package_name)
            {
                affected.push_back(edge.from);
            }
        }

        return affected;
    }

    std::vector<std::string> find_orphaned_packages() const
    {
        // Find packages that are not depended upon by any other package
        std::vector<std::string> orphaned;

        for (const auto &package : packages_)
        {
            const std::string &name = package.first;
            bool is_depended_upon = false;

            for (const auto &edge : dependency_graph_.get_edges())
            {
                if (edge.to == name)
                {
                    is_depended_upon = true;
                    break;
                }
            }

            if (!is_depended_upon)
            {
                orphaned.push_back(name);
            }
        }

        return orphaned;
    }

    std::vector<std::string> find_strongly_connected_components() const
    {
        auto scc_result = kosaraju_scc(dependency_graph_);

        std::vector<std::string> circular_deps;
        for (const auto &component : scc_result.components)
        {
            if (component.size() > 1)
            {
                // This is a circular dependency
                for (const auto &pkg : component)
                {
                    circular_deps.push_back(pkg);
                }
            }
        }

        return circular_deps;
    }

    void analyze_dependencies() const
    {
        std::cout << "\n=== Dependency Analysis ===\n";
        std::cout << "Total packages: " << packages_.size() << "\n";
        std::cout << "Total dependencies: " << dependency_graph_.num_edges() << "\n";

        // Check for circular dependencies
        auto circular = detect_circular_dependencies();
        if (!circular.empty())
        {
            std::cout << "\nCircular dependencies detected:\n";
            for (const auto &pkg : circular)
            {
                std::cout << "  " << pkg << "\n";
            }
        }
        else
        {
            std::cout << "\nNo circular dependencies found.\n";
        }

        // Find orphaned packages
        auto orphaned = find_orphaned_packages();
        if (!orphaned.empty())
        {
            std::cout << "\nOrphaned packages (not depended upon):\n";
            for (const auto &pkg : orphaned)
            {
                std::cout << "  " << pkg << "\n";
            }
        }

        // Calculate dependency statistics
        std::unordered_map<std::string, int> dependency_counts;
        for (const auto &package : packages_)
        {
            dependency_counts[package.first] = static_cast<int>(package.second.dependencies.size());
        }

        // Find packages with most dependencies
        std::vector<std::pair<std::string, int>> sorted_by_deps(dependency_counts.begin(), dependency_counts.end());
        std::sort(sorted_by_deps.begin(), sorted_by_deps.end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.second > b.second;
                  });

        std::cout << "\nPackages with most dependencies:\n";
        for (int i = 0; i < std::min(5, static_cast<int>(sorted_by_deps.size())); ++i)
        {
            std::cout << "  " << sorted_by_deps[i].first << ": " << sorted_by_deps[i].second << " dependencies\n";
        }
    }

    void print_package_info(const std::string &package_name) const
    {
        auto it = packages_.find(package_name);
        if (it == packages_.end())
        {
            std::cout << "Package not found: " << package_name << "\n";
            return;
        }

        const Package &pkg = it->second;
        std::cout << "\n=== " << pkg.name << " ===\n";
        std::cout << "Version: " << pkg.version << "\n";
        std::cout << "Description: " << pkg.description << "\n";
        std::cout << "Dependencies (" << pkg.dependencies.size() << "):\n";
        for (const auto &dep : pkg.dependencies)
        {
            std::cout << "  " << dep << "\n";
        }

        // Show packages that depend on this one
        auto dependents = find_affected_packages(package_name);
        std::cout << "Dependents (" << dependents.size() << "):\n";
        for (const auto &dependent : dependents)
        {
            std::cout << "  " << dependent << "\n";
        }
    }

    void print_installation_plan(const std::string &package_name) const
    {
        std::cout << "\n=== Installation Plan for " << package_name << " ===\n";

        auto installation_order = resolve_dependencies(package_name);
        if (installation_order.empty())
        {
            std::cout << "Cannot resolve dependencies due to circular dependencies.\n";
            return;
        }

        std::cout << "Installation order:\n";
        for (size_t i = 0; i < installation_order.size(); ++i)
        {
            std::cout << "  " << (i + 1) << ". " << installation_order[i];
            if (installation_order[i] == package_name)
            {
                std::cout << " (target package)";
            }
            std::cout << "\n";
        }

        std::cout << "\nTotal packages to install: " << installation_order.size() << "\n";
    }

    void save_to_file(const std::string &filename) const
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return;
        }

        // Save packages
        file << "# Packages\n";
        for (const auto &package : packages_)
        {
            file << "PACKAGE " << package.second.name << " " << package.second.version;
            for (const auto &dep : package.second.dependencies)
            {
                file << " " << dep;
            }
            file << " \"" << package.second.description << "\"\n";
        }

        file.close();
        std::cout << "Dependency graph saved to " << filename << "\n";
    }

    void load_from_file(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return;
        }

        packages_.clear();
        dependency_graph_.clear();

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "PACKAGE")
            {
                std::string name, version;
                iss >> name >> version;

                std::vector<std::string> dependencies;
                std::string dep;
                while (iss >> dep && dep[0] != '"')
                {
                    dependencies.push_back(dep);
                }

                // Read description (everything after the quote)
                std::string description;
                if (dep[0] == '"')
                {
                    description = dep.substr(1);
                    std::string word;
                    while (iss >> word)
                    {
                        description += " " + word;
                    }
                    // Remove trailing quote if present
                    if (!description.empty() && description.back() == '"')
                    {
                        description.pop_back();
                    }
                }

                add_package(name, version, dependencies, description);
            }
        }

        file.close();
        std::cout << "Dependency graph loaded from " << filename << "\n";
    }
};

void create_sample_packages(DependencyResolver &resolver)
{
    // Add packages with dependencies
    resolver.add_package("web-server", "1.0.0", {"http-server", "ssl", "logging"}, "Web server application");
    resolver.add_package("http-server", "2.1.0", {"networking", "threading"}, "HTTP server library");
    resolver.add_package("ssl", "1.5.0", {"crypto", "networking"}, "SSL/TLS library");
    resolver.add_package("logging", "1.2.0", {"file-io", "threading"}, "Logging framework");
    resolver.add_package("networking", "3.0.0", {"threading"}, "Network communication library");
    resolver.add_package("threading", "2.0.0", {}, "Threading library");
    resolver.add_package("crypto", "1.8.0", {"math"}, "Cryptographic functions");
    resolver.add_package("file-io", "1.1.0", {}, "File I/O operations");
    resolver.add_package("math", "1.0.0", {}, "Mathematical functions");

    // Add some packages with circular dependencies (for demonstration)
    resolver.add_package("package-a", "1.0.0", {"package-b"}, "Package A");
    resolver.add_package("package-b", "1.0.0", {"package-c"}, "Package B");
    resolver.add_package("package-c", "1.0.0", {"package-a"}, "Package C");

    // Add some standalone packages
    resolver.add_package("utility", "1.0.0", {}, "Utility functions");
    resolver.add_package("parser", "1.0.0", {"utility"}, "Text parser");
    resolver.add_package("database", "2.0.0", {"networking", "file-io"}, "Database library");
}

int main()
{
    std::cout << "Dependency Resolver Demo\n";
    std::cout << "========================\n";

    DependencyResolver resolver;

    // Create sample packages
    create_sample_packages(resolver);

    // Analyze dependencies
    resolver.analyze_dependencies();

    // Demonstrate dependency resolution
    std::cout << "\n=== Dependency Resolution Examples ===\n";

    std::string target_package = "web-server";
    resolver.print_installation_plan(target_package);

    // Show package information
    std::cout << "\n=== Package Information ===\n";
    resolver.print_package_info("web-server");
    resolver.print_package_info("ssl");

    // Demonstrate circular dependency detection
    std::cout << "\n=== Circular Dependency Detection ===\n";
    auto circular = resolver.detect_circular_dependencies();
    if (!circular.empty())
    {
        std::cout << "Circular dependencies found:\n";
        for (const auto &pkg : circular)
        {
            std::cout << "  " << pkg << "\n";
        }
    }

    // Show affected packages
    std::cout << "\n=== Affected Packages ===\n";
    std::string affected_package = "threading";
    auto affected = resolver.find_affected_packages(affected_package);
    std::cout << "Packages that depend on " << affected_package << ":\n";
    for (const auto &pkg : affected)
    {
        std::cout << "  " << pkg << "\n";
    }

    // Show orphaned packages
    std::cout << "\n=== Orphaned Packages ===\n";
    auto orphaned = resolver.find_orphaned_packages();
    std::cout << "Packages not depended upon by any other package:\n";
    for (const auto &pkg : orphaned)
    {
        std::cout << "  " << pkg << "\n";
    }

    // Save and load dependency graph
    std::cout << "\n=== File Operations ===\n";
    resolver.save_to_file("dependencies.txt");

    // Create new resolver and load from file
    DependencyResolver loaded_resolver;
    loaded_resolver.load_from_file("dependencies.txt");
    loaded_resolver.analyze_dependencies();

    return 0;
}
