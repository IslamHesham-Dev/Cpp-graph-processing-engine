#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <cassert>

namespace graph_engine
{
    namespace utils
    {

        /**
         * @brief Union-Find data structure with path compression and union by rank
         *
         * This implementation provides amortized O(α(n)) time complexity for both
         * find and union operations, where α is the inverse Ackermann function.
         *
         * @tparam T The type of elements to store
         */
        template <typename T>
        class UnionFind
        {
        public:
            using value_type = T;
            using size_type = std::size_t;

            /**
             * @brief Construct a new Union Find object
             * @param hash_func Hash function for the element type
             */
            explicit UnionFind(std::function<size_type(const T &)> hash_func = std::hash<T>{})
                : hash_func_(std::move(hash_func)) {}

            /**
             * @brief Add an element to the disjoint set
             * @param element The element to add
             */
            void make_set(const T &element)
            {
                size_type id = get_id(element);
                if (parent_.size() <= id)
                {
                    parent_.resize(id + 1, id);
                    rank_.resize(id + 1, 0);
                }
            }

            /**
             * @brief Find the representative of the set containing the element
             * @param element The element to find
             * @return The representative element
             */
            T find(const T &element)
            {
                size_type id = get_id(element);
                assert(id < parent_.size() && "Element not in any set");

                // Path compression
                if (parent_[id] != id)
                {
                    parent_[id] = find_by_id(parent_[id]);
                }
                return id_to_element_[parent_[id]];
            }

            /**
             * @brief Union two sets containing the given elements
             * @param a First element
             * @param b Second element
             * @return true if the sets were merged, false if they were already in the same set
             */
            bool union_sets(const T &a, const T &b)
            {
                size_type root_a = find_by_id(get_id(a));
                size_type root_b = find_by_id(get_id(b));

                if (root_a == root_b)
                {
                    return false; // Already in the same set
                }

                // Union by rank
                if (rank_[root_a] < rank_[root_b])
                {
                    std::swap(root_a, root_b);
                }

                parent_[root_b] = root_a;
                if (rank_[root_a] == rank_[root_b])
                {
                    rank_[root_a]++;
                }

                return true;
            }

            /**
             * @brief Check if two elements are in the same set
             * @param a First element
             * @param b Second element
             * @return true if elements are in the same set
             */
            bool same_set(const T &a, const T &b)
            {
                return find(a) == find(b);
            }

            /**
             * @brief Get the number of disjoint sets
             * @return Number of sets
             */
            size_type num_sets() const
            {
                size_type count = 0;
                for (size_type i = 0; i < parent_.size(); ++i)
                {
                    if (parent_[i] == i)
                    {
                        count++;
                    }
                }
                return count;
            }

            /**
             * @brief Get all elements in the same set as the given element
             * @param element The element to find set members for
             * @return Vector of elements in the same set
             */
            std::vector<T> get_set_members(const T &element)
            {
                T representative = find(element);
                std::vector<T> members;

                for (const auto &pair : element_to_id_)
                {
                    if (find(pair.first) == representative)
                    {
                        members.push_back(pair.first);
                    }
                }

                return members;
            }

            /**
             * @brief Clear all sets
             */
            void clear()
            {
                parent_.clear();
                rank_.clear();
                element_to_id_.clear();
                id_to_element_.clear();
            }

            /**
             * @brief Get the number of elements in the structure
             * @return Number of elements
             */
            size_type size() const
            {
                return element_to_id_.size();
            }

            /**
             * @brief Check if the structure is empty
             * @return true if empty
             */
            bool empty() const
            {
                return element_to_id_.empty();
            }

        private:
            std::vector<size_type> parent_;
            std::vector<size_type> rank_;
            std::unordered_map<T, size_type> element_to_id_;
            std::unordered_map<size_type, T> id_to_element_;
            std::function<size_type(const T &)> hash_func_;
            size_type next_id_ = 0;

            size_type get_id(const T &element)
            {
                auto it = element_to_id_.find(element);
                if (it == element_to_id_.end())
                {
                    size_type id = next_id_++;
                    element_to_id_[element] = id;
                    id_to_element_[id] = element;

                    // Ensure vectors are large enough
                    if (parent_.size() <= id)
                    {
                        parent_.resize(id + 1, id);
                        rank_.resize(id + 1, 0);
                    }

                    return id;
                }
                return it->second;
            }

            size_type find_by_id(size_type id)
            {
                if (parent_[id] != id)
                {
                    parent_[id] = find_by_id(parent_[id]);
                }
                return parent_[id];
            }
        };

        /**
         * @brief Specialized Union-Find for integer types with direct indexing
         *
         * This version provides O(1) access for integer types by using direct
         * array indexing instead of hash maps.
         *
         * @tparam T Integer type (int, size_t, etc.)
         */
        template <typename T>
        class UnionFindInt
        {
            static_assert(std::is_integral_v<T>, "UnionFindInt requires integer type");

        public:
            using value_type = T;
            using size_type = std::size_t;

            /**
             * @brief Construct a new Union Find Int object
             * @param max_element Maximum element value (for pre-allocation)
             */
            explicit UnionFindInt(T max_element = 1000000)
                : max_element_(max_element)
            {
                parent_.resize(max_element + 1);
                rank_.resize(max_element + 1, 0);

                // Initialize each element as its own parent
                for (T i = 0; i <= max_element; ++i)
                {
                    parent_[i] = i;
                }
            }

            /**
             * @brief Find the representative of the set containing the element
             * @param element The element to find
             * @return The representative element
             */
            T find(T element)
            {
                assert(element <= max_element_ && "Element exceeds maximum");

                // Path compression
                if (parent_[element] != element)
                {
                    parent_[element] = find(parent_[element]);
                }
                return parent_[element];
            }

            /**
             * @brief Union two sets containing the given elements
             * @param a First element
             * @param b Second element
             * @return true if the sets were merged, false if they were already in the same set
             */
            bool union_sets(T a, T b)
            {
                assert(a <= max_element_ && b <= max_element_ && "Element exceeds maximum");

                T root_a = find(a);
                T root_b = find(b);

                if (root_a == root_b)
                {
                    return false; // Already in the same set
                }

                // Union by rank
                if (rank_[root_a] < rank_[root_b])
                {
                    std::swap(root_a, root_b);
                }

                parent_[root_b] = root_a;
                if (rank_[root_a] == rank_[root_b])
                {
                    rank_[root_a]++;
                }

                return true;
            }

            /**
             * @brief Check if two elements are in the same set
             * @param a First element
             * @param b Second element
             * @return true if elements are in the same set
             */
            bool same_set(T a, T b)
            {
                return find(a) == find(b);
            }

            /**
             * @brief Get the number of disjoint sets
             * @return Number of sets
             */
            size_type num_sets() const
            {
                size_type count = 0;
                for (T i = 0; i <= max_element_; ++i)
                {
                    if (parent_[i] == i)
                    {
                        count++;
                    }
                }
                return count;
            }

            /**
             * @brief Clear all sets (reset to initial state)
             */
            void clear()
            {
                for (T i = 0; i <= max_element_; ++i)
                {
                    parent_[i] = i;
                    rank_[i] = 0;
                }
            }

        private:
            std::vector<T> parent_;
            std::vector<T> rank_;
            T max_element_;
        };

    } // namespace utils
} // namespace graph_engine
