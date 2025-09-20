#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>

#include "graph_engine/graph.hpp"
#include "graph_engine/algorithms/traversal.hpp"
#include "graph_engine/algorithms/shortest_path.hpp"
#include "graph_engine/algorithms/advanced.hpp"

using namespace graph_engine;
using namespace graph_engine::algorithms;

struct Person
{
    std::string name;
    int age;
    std::string occupation;
    std::vector<std::string> interests;

    Person() = default;
    Person(const std::string &n, int a, const std::string &occ, const std::vector<std::string> &ints)
        : name(n), age(a), occupation(occ), interests(ints) {}
};

class SocialNetwork
{
private:
    Graph<std::string, double> network_;
    std::unordered_map<std::string, Person> people_;

public:
    SocialNetwork() : network_(GraphDirection::UNDIRECTED) {}

    void add_person(const std::string &name, int age, const std::string &occupation,
                    const std::vector<std::string> &interests)
    {
        people_[name] = Person(name, age, occupation, interests);
        network_.add_vertex(name);
    }

    void add_friendship(const std::string &person1, const std::string &person2, double strength = 1.0)
    {
        if (people_.find(person1) != people_.end() && people_.find(person2) != people_.end())
        {
            network_.add_edge(person1, person2, strength);
        }
    }

    void remove_friendship(const std::string &person1, const std::string &person2)
    {
        network_.remove_edge(person1, person2);
    }

    std::vector<std::string> get_friends(const std::string &person) const
    {
        auto neighbors = network_.get_neighbors(person);
        std::vector<std::string> friends;
        for (const auto &neighbor : neighbors)
        {
            friends.push_back(neighbor.first);
        }
        return friends;
    }

    double get_friendship_strength(const std::string &person1, const std::string &person2) const
    {
        return network_.get_edge_weight(person1, person2);
    }

    std::vector<std::string> find_mutual_friends(const std::string &person1, const std::string &person2) const
    {
        auto friends1 = get_friends(person1);
        auto friends2 = get_friends(person2);

        std::unordered_set<std::string> friends1_set(friends1.begin(), friends1.end());
        std::vector<std::string> mutual;

        for (const auto &friend_name : friends2)
        {
            if (friends1_set.find(friend_name) != friends1_set.end())
            {
                mutual.push_back(friend_name);
            }
        }

        return mutual;
    }

    std::vector<std::string> find_shortest_path(const std::string &from, const std::string &to) const
    {
        auto result = shortest_path_unweighted(network_, from, to);
        return result;
    }

    std::vector<std::vector<std::string>> find_communities() const
    {
        return connected_components(network_);
    }

    std::vector<std::string> find_influential_people(int top_k = 5) const
    {
        std::vector<std::pair<std::string, int>> centrality_scores;

        for (const auto &person : people_)
        {
            const std::string &name = person.first;
            int degree = static_cast<int>(get_friends(name).size());
            centrality_scores.push_back({name, degree});
        }

        // Sort by degree centrality (number of friends)
        std::sort(centrality_scores.begin(), centrality_scores.end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.second > b.second;
                  });

        std::vector<std::string> influential;
        for (int i = 0; i < std::min(top_k, static_cast<int>(centrality_scores.size())); ++i)
        {
            influential.push_back(centrality_scores[i].first);
        }

        return influential;
    }

    std::vector<std::string> recommend_friends(const std::string &person, int max_recommendations = 5) const
    {
        if (people_.find(person) == people_.end())
        {
            return {};
        }

        auto current_friends = get_friends(person);
        std::unordered_set<std::string> friends_set(current_friends.begin(), current_friends.end());

        std::vector<std::pair<std::string, int>> recommendations;

        // Find friends of friends who are not already friends
        for (const auto &friend_name : current_friends)
        {
            auto friends_of_friend = get_friends(friend_name);
            for (const auto &potential_friend : friends_of_friend)
            {
                if (potential_friend != person &&
                    friends_set.find(potential_friend) == friends_set.end())
                {

                    // Count mutual connections
                    int mutual_count = 0;
                    for (const auto &mutual : find_mutual_friends(person, potential_friend))
                    {
                        mutual_count++;
                    }

                    recommendations.push_back({potential_friend, mutual_count});
                }
            }
        }

        // Remove duplicates and sort by mutual connections
        std::sort(recommendations.begin(), recommendations.end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.second > b.second;
                  });

        std::vector<std::string> unique_recommendations;
        std::unordered_set<std::string> seen;

        for (const auto &rec : recommendations)
        {
            if (seen.find(rec.first) == seen.end())
            {
                unique_recommendations.push_back(rec.first);
                seen.insert(rec.first);
                if (unique_recommendations.size() >= max_recommendations)
                {
                    break;
                }
            }
        }

        return unique_recommendations;
    }

    void analyze_network() const
    {
        std::cout << "\n=== Social Network Analysis ===\n";
        std::cout << "Total people: " << people_.size() << "\n";
        std::cout << "Total friendships: " << network_.num_edges() << "\n";

        // Find communities
        auto communities = find_communities();
        std::cout << "Number of communities: " << communities.size() << "\n";

        // Find largest community
        auto largest_community = *std::max_element(communities.begin(), communities.end(),
                                                   [](const auto &a, const auto &b)
                                                   {
                                                       return a.size() < b.size();
                                                   });

        std::cout << "Largest community size: " << largest_community.size() << "\n";

        // Find influential people
        auto influential = find_influential_people(3);
        std::cout << "Most influential people:\n";
        for (const auto &person : influential)
        {
            int degree = static_cast<int>(get_friends(person).size());
            std::cout << "  " << person << " (" << degree << " friends)\n";
        }

        // Calculate average degree
        double total_degree = 0;
        for (const auto &person : people_)
        {
            total_degree += get_friends(person.first).size();
        }
        double avg_degree = total_degree / people_.size();
        std::cout << "Average number of friends: " << avg_degree << "\n";
    }

    void print_person_info(const std::string &name) const
    {
        auto it = people_.find(name);
        if (it == people_.end())
        {
            std::cout << "Person not found: " << name << "\n";
            return;
        }

        const Person &person = it->second;
        std::cout << "\n=== " << person.name << " ===\n";
        std::cout << "Age: " << person.age << "\n";
        std::cout << "Occupation: " << person.occupation << "\n";
        std::cout << "Interests: ";
        for (size_t i = 0; i < person.interests.size(); ++i)
        {
            std::cout << person.interests[i];
            if (i < person.interests.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n";

        auto friends = get_friends(name);
        std::cout << "Friends (" << friends.size() << "): ";
        for (size_t i = 0; i < friends.size(); ++i)
        {
            std::cout << friends[i];
            if (i < friends.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n";
    }

    void save_to_file(const std::string &filename) const
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return;
        }

        // Save people information
        file << "# People\n";
        for (const auto &person : people_)
        {
            file << "PERSON " << person.second.name << " " << person.second.age
                 << " " << person.second.occupation;
            for (const auto &interest : person.second.interests)
            {
                file << " " << interest;
            }
            file << "\n";
        }

        // Save friendships
        file << "\n# Friendships\n";
        for (const auto &edge : network_.get_edges())
        {
            file << "FRIENDSHIP " << edge.from << " " << edge.to << " " << edge.weight << "\n";
        }

        file.close();
        std::cout << "Network saved to " << filename << "\n";
    }

    void load_from_file(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return;
        }

        people_.clear();
        network_.clear();

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "PERSON")
            {
                std::string name, occupation;
                int age;
                iss >> name >> age >> occupation;

                std::vector<std::string> interests;
                std::string interest;
                while (iss >> interest)
                {
                    interests.push_back(interest);
                }

                add_person(name, age, occupation, interests);
            }
            else if (type == "FRIENDSHIP")
            {
                std::string person1, person2;
                double strength;
                iss >> person1 >> person2 >> strength;
                add_friendship(person1, person2, strength);
            }
        }

        file.close();
        std::cout << "Network loaded from " << filename << "\n";
    }
};

void create_sample_network(SocialNetwork &network)
{
    // Add people
    network.add_person("Alice", 25, "Software Engineer", {"programming", "music", "travel"});
    network.add_person("Bob", 30, "Data Scientist", {"machine learning", "hiking", "photography"});
    network.add_person("Charlie", 28, "Designer", {"art", "music", "fashion"});
    network.add_person("Diana", 32, "Manager", {"leadership", "travel", "cooking"});
    network.add_person("Eve", 26, "Writer", {"books", "coffee", "travel"});
    network.add_person("Frank", 35, "Teacher", {"education", "sports", "music"});
    network.add_person("Grace", 29, "Doctor", {"medicine", "fitness", "reading"});
    network.add_person("Henry", 31, "Artist", {"painting", "music", "nature"});
    network.add_person("Ivy", 27, "Researcher", {"science", "books", "hiking"});
    network.add_person("Jack", 33, "Chef", {"cooking", "travel", "wine"});

    // Add friendships based on common interests
    network.add_friendship("Alice", "Bob", 0.8);     // Both like programming/tech
    network.add_friendship("Alice", "Charlie", 0.6); // Both like music
    network.add_friendship("Bob", "Ivy", 0.9);       // Both like science/research
    network.add_friendship("Charlie", "Henry", 0.8); // Both like art/music
    network.add_friendship("Diana", "Eve", 0.7);     // Both like travel
    network.add_friendship("Diana", "Jack", 0.6);    // Both like travel/cooking
    network.add_friendship("Eve", "Grace", 0.5);     // Both like reading
    network.add_friendship("Frank", "Grace", 0.7);   // Both like music
    network.add_friendship("Grace", "Ivy", 0.6);     // Both like reading/books
    network.add_friendship("Henry", "Jack", 0.5);    // Both like art/culture
    network.add_friendship("Ivy", "Bob", 0.9);       // Both like science
    network.add_friendship("Alice", "Eve", 0.4);     // Weak connection
    network.add_friendship("Charlie", "Diana", 0.3); // Weak connection
    network.add_friendship("Frank", "Jack", 0.4);    // Weak connection
}

int main()
{
    std::cout << "Social Network Analysis Demo\n";
    std::cout << "============================\n";

    SocialNetwork network;

    // Create sample network
    create_sample_network(network);

    // Analyze the network
    network.analyze_network();

    // Demonstrate various features
    std::cout << "\n=== Friend Recommendations ===\n";
    std::string person = "Alice";
    auto recommendations = network.recommend_friends(person, 3);
    std::cout << "Friend recommendations for " << person << ":\n";
    for (const auto &rec : recommendations)
    {
        auto mutual = network.find_mutual_friends(person, rec);
        std::cout << "  " << rec << " (" << mutual.size() << " mutual friends)\n";
    }

    std::cout << "\n=== Shortest Path ===\n";
    std::string from = "Alice";
    std::string to = "Jack";
    auto path = network.find_shortest_path(from, to);
    if (!path.empty())
    {
        std::cout << "Shortest path from " << from << " to " << to << ":\n";
        for (size_t i = 0; i < path.size(); ++i)
        {
            std::cout << path[i];
            if (i < path.size() - 1)
                std::cout << " -> ";
        }
        std::cout << "\n";
    }
    else
    {
        std::cout << "No path found from " << from << " to " << to << "\n";
    }

    std::cout << "\n=== Person Information ===\n";
    network.print_person_info("Alice");
    network.print_person_info("Bob");

    std::cout << "\n=== Mutual Friends ===\n";
    auto mutual = network.find_mutual_friends("Alice", "Bob");
    std::cout << "Mutual friends between Alice and Bob: ";
    for (size_t i = 0; i < mutual.size(); ++i)
    {
        std::cout << mutual[i];
        if (i < mutual.size() - 1)
            std::cout << ", ";
    }
    std::cout << "\n";

    // Save and load network
    std::cout << "\n=== File Operations ===\n";
    network.save_to_file("social_network.txt");

    // Create new network and load from file
    SocialNetwork loaded_network;
    loaded_network.load_from_file("social_network.txt");
    loaded_network.analyze_network();

    return 0;
}
