#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "graph_engine/graph.hpp"
#include "graph_engine/algorithms/shortest_path.hpp"
#include "graph_engine/algorithms/traversal.hpp"

using namespace graph_engine;
using namespace graph_engine::algorithms;

struct Location
{
    std::string name;
    double latitude;
    double longitude;
    std::string type; // "city", "landmark", "restaurant", "hotel", etc.

    Location() = default;
    Location(const std::string &n, double lat, double lon, const std::string &t)
        : name(n), latitude(lat), longitude(lon), type(t) {}
};

class RoutePlanner
{
private:
    Graph<std::string, double> road_network_;
    std::unordered_map<std::string, Location> locations_;

    // Calculate distance between two points using Haversine formula
    double calculate_distance(const Location &loc1, const Location &loc2) const
    {
        const double R = 6371.0; // Earth's radius in kilometers
        const double PI = 3.14159265358979323846;

        double lat1_rad = loc1.latitude * PI / 180.0;
        double lat2_rad = loc2.latitude * PI / 180.0;
        double delta_lat = (loc2.latitude - loc1.latitude) * PI / 180.0;
        double delta_lon = (loc2.longitude - loc1.longitude) * PI / 180.0;

        double a = std::sin(delta_lat / 2) * std::sin(delta_lat / 2) +
                   std::cos(lat1_rad) * std::cos(lat2_rad) *
                       std::sin(delta_lon / 2) * std::sin(delta_lon / 2);
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

        return R * c;
    }

public:
    RoutePlanner() : road_network_(GraphDirection::UNDIRECTED) {}

    void add_location(const std::string &name, double latitude, double longitude, const std::string &type)
    {
        locations_[name] = Location(name, latitude, longitude, type);
        road_network_.add_vertex(name);
    }

    void add_road(const std::string &from, const std::string &to, double distance = -1)
    {
        if (locations_.find(from) != locations_.end() && locations_.find(to) != locations_.end())
        {
            if (distance < 0)
            {
                // Calculate distance automatically
                distance = calculate_distance(locations_[from], locations_[to]);
            }
            road_network_.add_edge(from, to, distance);
        }
    }

    void remove_road(const std::string &from, const std::string &to)
    {
        road_network_.remove_edge(from, to);
    }

    std::vector<std::string> find_shortest_route(const std::string &from, const std::string &to) const
    {
        auto result = dijkstra(road_network_, from, to);
        return result.path;
    }

    double get_route_distance(const std::string &from, const std::string &to) const
    {
        auto result = dijkstra(road_network_, from, to);
        return result.total_distance;
    }

    std::vector<std::string> find_route_with_waypoints(const std::string &start,
                                                       const std::vector<std::string> &waypoints,
                                                       const std::string &end) const
    {
        std::vector<std::string> full_route;
        std::string current = start;

        // Add start location
        full_route.push_back(current);

        // Visit each waypoint
        for (const auto &waypoint : waypoints)
        {
            auto segment = find_shortest_route(current, waypoint);
            if (segment.empty())
            {
                std::cerr << "No route found from " << current << " to " << waypoint << std::endl;
                return {};
            }

            // Add segment (excluding the starting point to avoid duplicates)
            for (size_t i = 1; i < segment.size(); ++i)
            {
                full_route.push_back(segment[i]);
            }
            current = waypoint;
        }

        // Go to end location
        auto final_segment = find_shortest_route(current, end);
        if (final_segment.empty())
        {
            std::cerr << "No route found from " << current << " to " << end << std::endl;
            return {};
        }

        // Add final segment (excluding the starting point)
        for (size_t i = 1; i < final_segment.size(); ++i)
        {
            full_route.push_back(final_segment[i]);
        }

        return full_route;
    }

    std::vector<std::string> find_nearby_locations(const std::string &center, double radius_km) const
    {
        std::vector<std::string> nearby;

        auto center_it = locations_.find(center);
        if (center_it == locations_.end())
        {
            return nearby;
        }

        const Location &center_loc = center_it->second;

        for (const auto &location : locations_)
        {
            if (location.first != center)
            {
                double distance = calculate_distance(center_loc, location.second);
                if (distance <= radius_km)
                {
                    nearby.push_back(location.first);
                }
            }
        }

        // Sort by distance
        std::sort(nearby.begin(), nearby.end(),
                  [&](const std::string &a, const std::string &b)
                  {
                      double dist_a = calculate_distance(center_loc, locations_.at(a));
                      double dist_b = calculate_distance(center_loc, locations_.at(b));
                      return dist_a < dist_b;
                  });

        return nearby;
    }

    std::vector<std::string> find_locations_by_type(const std::string &type) const
    {
        std::vector<std::string> locations;

        for (const auto &location : locations_)
        {
            if (location.second.type == type)
            {
                locations.push_back(location.first);
            }
        }

        return locations;
    }

    void print_route_info(const std::vector<std::string> &route) const
    {
        if (route.empty())
        {
            std::cout << "No route found.\n";
            return;
        }

        std::cout << "\n=== Route Information ===\n";
        std::cout << "Route: ";
        for (size_t i = 0; i < route.size(); ++i)
        {
            std::cout << route[i];
            if (i < route.size() - 1)
                std::cout << " -> ";
        }
        std::cout << "\n";

        double total_distance = 0;
        for (size_t i = 0; i < route.size() - 1; ++i)
        {
            double segment_distance = road_network_.get_edge_weight(route[i], route[i + 1]);
            total_distance += segment_distance;
        }

        std::cout << "Total distance: " << std::fixed << std::setprecision(2)
                  << total_distance << " km\n";
        std::cout << "Number of stops: " << route.size() << "\n";

        // Estimate travel time (assuming average speed of 60 km/h)
        double travel_time = total_distance / 60.0;
        int hours = static_cast<int>(travel_time);
        int minutes = static_cast<int>((travel_time - hours) * 60);
        std::cout << "Estimated travel time: " << hours << "h " << minutes << "m\n";

        std::cout << "\nDetailed route:\n";
        for (size_t i = 0; i < route.size() - 1; ++i)
        {
            double segment_distance = road_network_.get_edge_weight(route[i], route[i + 1]);
            auto loc_it = locations_.find(route[i]);
            if (loc_it != locations_.end())
            {
                std::cout << "  " << (i + 1) << ". " << route[i]
                          << " (" << loc_it->second.type << ") -> "
                          << segment_distance << " km\n";
            }
        }

        // Print final destination
        auto final_it = locations_.find(route.back());
        if (final_it != locations_.end())
        {
            std::cout << "  " << route.size() << ". " << route.back()
                      << " (" << final_it->second.type << ")\n";
        }
    }

    void analyze_network() const
    {
        std::cout << "\n=== Road Network Analysis ===\n";
        std::cout << "Total locations: " << locations_.size() << "\n";
        std::cout << "Total roads: " << road_network_.num_edges() << "\n";

        // Count locations by type
        std::unordered_map<std::string, int> type_counts;
        for (const auto &location : locations_)
        {
            type_counts[location.second.type]++;
        }

        std::cout << "\nLocations by type:\n";
        for (const auto &type_count : type_counts)
        {
            std::cout << "  " << type_count.first << ": " << type_count.second << "\n";
        }

        // Find isolated locations
        auto components = connected_components(road_network_);
        std::cout << "\nConnected components: " << components.size() << "\n";

        if (components.size() > 1)
        {
            std::cout << "Isolated locations:\n";
            for (const auto &component : components)
            {
                if (component.size() == 1)
                {
                    std::cout << "  " << component[0] << "\n";
                }
            }
        }
    }

    void save_to_file(const std::string &filename) const
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return;
        }

        // Save locations
        file << "# Locations\n";
        for (const auto &location : locations_)
        {
            file << "LOCATION " << location.second.name << " "
                 << location.second.latitude << " " << location.second.longitude
                 << " " << location.second.type << "\n";
        }

        // Save roads
        file << "\n# Roads\n";
        for (const auto &edge : road_network_.get_edges())
        {
            file << "ROAD " << edge.from << " " << edge.to << " " << edge.weight << "\n";
        }

        file.close();
        std::cout << "Road network saved to " << filename << "\n";
    }

    void load_from_file(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return;
        }

        locations_.clear();
        road_network_.clear();

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "LOCATION")
            {
                std::string name, location_type;
                double latitude, longitude;
                iss >> name >> latitude >> longitude >> location_type;
                add_location(name, latitude, longitude, location_type);
            }
            else if (type == "ROAD")
            {
                std::string from, to;
                double distance;
                iss >> from >> to >> distance;
                add_road(from, to, distance);
            }
        }

        file.close();
        std::cout << "Road network loaded from " << filename << "\n";
    }
};

void create_sample_network(RoutePlanner &planner)
{
    // Add cities and landmarks
    planner.add_location("New York", 40.7128, -74.0060, "city");
    planner.add_location("Boston", 42.3601, -71.0589, "city");
    planner.add_location("Philadelphia", 39.9526, -75.1652, "city");
    planner.add_location("Washington DC", 38.9072, -77.0369, "city");
    planner.add_location("Baltimore", 39.2904, -76.6122, "city");

    // Add landmarks
    planner.add_location("Statue of Liberty", 40.6892, -74.0445, "landmark");
    planner.add_location("Times Square", 40.7580, -73.9855, "landmark");
    planner.add_location("Independence Hall", 39.9489, -75.1500, "landmark");
    planner.add_location("White House", 38.8977, -77.0365, "landmark");
    planner.add_location("Fenway Park", 42.3467, -71.0972, "landmark");

    // Add restaurants
    planner.add_location("Joe's Pizza", 40.7505, -73.9934, "restaurant");
    planner.add_location("Legal Sea Foods", 42.3601, -71.0589, "restaurant");
    planner.add_location("Reading Terminal Market", 39.9546, -75.1594, "restaurant");

    // Add hotels
    planner.add_location("The Plaza", 40.7648, -73.9748, "hotel");
    planner.add_location("Four Seasons Boston", 42.3503, -71.0750, "hotel");
    planner.add_location("Ritz-Carlton DC", 38.9072, -77.0369, "hotel");

    // Add roads between cities
    planner.add_road("New York", "Boston");
    planner.add_road("New York", "Philadelphia");
    planner.add_road("Boston", "Philadelphia");
    planner.add_road("Philadelphia", "Washington DC");
    planner.add_road("Washington DC", "Baltimore");
    planner.add_road("Philadelphia", "Baltimore");

    // Add roads to landmarks
    planner.add_road("New York", "Statue of Liberty");
    planner.add_road("New York", "Times Square");
    planner.add_road("Philadelphia", "Independence Hall");
    planner.add_road("Washington DC", "White House");
    planner.add_road("Boston", "Fenway Park");

    // Add roads to restaurants
    planner.add_road("New York", "Joe's Pizza");
    planner.add_road("Boston", "Legal Sea Foods");
    planner.add_road("Philadelphia", "Reading Terminal Market");

    // Add roads to hotels
    planner.add_road("New York", "The Plaza");
    planner.add_road("Boston", "Four Seasons Boston");
    planner.add_road("Washington DC", "Ritz-Carlton DC");
}

int main()
{
    std::cout << "Route Planner Demo\n";
    std::cout << "==================\n";

    RoutePlanner planner;

    // Create sample road network
    create_sample_network(planner);

    // Analyze the network
    planner.analyze_network();

    // Demonstrate route planning
    std::cout << "\n=== Route Planning Examples ===\n";

    // Simple route
    std::string from = "New York";
    std::string to = "Washington DC";
    auto route = planner.find_shortest_route(from, to);
    std::cout << "Route from " << from << " to " << to << ":\n";
    planner.print_route_info(route);

    // Route with waypoints
    std::cout << "\n=== Route with Waypoints ===\n";
    std::vector<std::string> waypoints = {"Philadelphia", "Independence Hall"};
    auto waypoint_route = planner.find_route_with_waypoints("New York", waypoints, "Washington DC");
    std::cout << "Route from New York via Philadelphia and Independence Hall to Washington DC:\n";
    planner.print_route_info(waypoint_route);

    // Find nearby locations
    std::cout << "\n=== Nearby Locations ===\n";
    std::string center = "New York";
    double radius = 50.0; // 50 km radius
    auto nearby = planner.find_nearby_locations(center, radius);
    std::cout << "Locations within " << radius << " km of " << center << ":\n";
    for (const auto &location : nearby)
    {
        std::cout << "  " << location << "\n";
    }

    // Find locations by type
    std::cout << "\n=== Locations by Type ===\n";
    auto restaurants = planner.find_locations_by_type("restaurant");
    std::cout << "Restaurants:\n";
    for (const auto &restaurant : restaurants)
    {
        std::cout << "  " << restaurant << "\n";
    }

    auto landmarks = planner.find_locations_by_type("landmark");
    std::cout << "Landmarks:\n";
    for (const auto &landmark : landmarks)
    {
        std::cout << "  " << landmark << "\n";
    }

    // Save and load network
    std::cout << "\n=== File Operations ===\n";
    planner.save_to_file("road_network.txt");

    // Create new planner and load from file
    RoutePlanner loaded_planner;
    loaded_planner.load_from_file("road_network.txt");
    loaded_planner.analyze_network();

    return 0;
}
