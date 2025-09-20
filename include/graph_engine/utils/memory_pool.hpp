#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <mutex>
#include <atomic>
#include <cassert>
#include <type_traits>
#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif

namespace graph_engine
{
    namespace utils
    {

        /**
         * @brief Thread-safe memory pool for efficient allocation/deallocation
         *
         * This memory pool provides O(1) allocation and deallocation for objects
         * of a fixed size, reducing memory fragmentation and improving cache locality.
         *
         * @tparam T Type of objects to store in the pool
         * @tparam BlockSize Number of objects per memory block
         */
        template <typename T, std::size_t BlockSize = 1024>
        class MemoryPool
        {
            static_assert(BlockSize > 0, "Block size must be positive");

        public:
            using value_type = T;
            using pointer = T *;
            using const_pointer = const T *;
            using reference = T &;
            using const_reference = const T &;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;

            /**
             * @brief Construct a new Memory Pool object
             * @param initial_blocks Number of blocks to allocate initially
             */
            explicit MemoryPool(size_type initial_blocks = 1)
                : blocks_allocated_(0), total_objects_(0)
            {
                for (size_type i = 0; i < initial_blocks; ++i)
                {
                    allocate_block();
                }
            }

            /**
             * @brief Destructor - deallocates all memory
             */
            ~MemoryPool()
            {
                for (auto &block : blocks_)
                {
#ifdef _WIN32
                    _aligned_free(block);
#else
                    std::free(block);
#endif
                }
            }

            // Non-copyable, non-movable
            MemoryPool(const MemoryPool &) = delete;
            MemoryPool &operator=(const MemoryPool &) = delete;
            MemoryPool(MemoryPool &&) = delete;
            MemoryPool &operator=(MemoryPool &&) = delete;

            /**
             * @brief Allocate memory for a single object
             * @return Pointer to allocated memory
             */
            pointer allocate()
            {
                std::lock_guard<std::mutex> lock(mutex_);

                if (free_list_.empty())
                {
                    allocate_block();
                }

                pointer ptr = free_list_.top();
                free_list_.pop();
                return ptr;
            }

            /**
             * @brief Deallocate memory for a single object
             * @param ptr Pointer to deallocate
             */
            void deallocate(pointer ptr)
            {
                if (!ptr)
                    return;

                std::lock_guard<std::mutex> lock(mutex_);
                free_list_.push(ptr);
            }

            /**
             * @brief Construct an object in the pool
             * @param args Arguments for object construction
             * @return Pointer to constructed object
             */
            template <typename... Args>
            pointer construct(Args &&...args)
            {
                pointer ptr = allocate();
                try
                {
                    new (ptr) T(std::forward<Args>(args)...);
                }
                catch (...)
                {
                    deallocate(ptr);
                    throw;
                }
                return ptr;
            }

            /**
             * @brief Destroy an object and return memory to pool
             * @param ptr Pointer to object to destroy
             */
            void destroy(pointer ptr)
            {
                if (!ptr)
                    return;

                ptr->~T();
                deallocate(ptr);
            }

            /**
             * @brief Get the number of allocated blocks
             * @return Number of blocks
             */
            size_type blocks_allocated() const
            {
                return blocks_allocated_.load();
            }

            /**
             * @brief Get the total number of objects that can be stored
             * @return Total capacity
             */
            size_type capacity() const
            {
                return blocks_allocated_.load() * BlockSize;
            }

            /**
             * @brief Get the number of objects currently in use
             * @return Number of objects in use
             */
            size_type size() const
            {
                return total_objects_.load() - free_list_.size();
            }

            /**
             * @brief Check if the pool is empty
             * @return true if no objects are in use
             */
            bool empty() const
            {
                return size() == 0;
            }

            /**
             * @brief Get memory usage statistics
             * @return Pair of (used_objects, total_capacity)
             */
            std::pair<size_type, size_type> memory_stats() const
            {
                return {size(), capacity()};
            }

        private:
            std::vector<pointer> blocks_;
            std::stack<pointer> free_list_;
            std::mutex mutex_;
            std::atomic<size_type> blocks_allocated_;
            std::atomic<size_type> total_objects_;

            void allocate_block()
            {
// Use cross-platform aligned allocation
#ifdef _WIN32
                // Windows: use _aligned_malloc
                pointer block = static_cast<pointer>(_aligned_malloc(BlockSize * sizeof(T), alignof(T)));
#else
                // POSIX: use posix_memalign
                void *ptr = nullptr;
                if (posix_memalign(&ptr, alignof(T), BlockSize * sizeof(T)) != 0)
                {
                    throw std::bad_alloc();
                }
                pointer block = static_cast<pointer>(ptr);
#endif

                if (!block)
                {
                    throw std::bad_alloc();
                }

                blocks_.push_back(block);
                blocks_allocated_.fetch_add(1);
                total_objects_.fetch_add(BlockSize);

                // Add all objects in the block to the free list
                for (size_type i = 0; i < BlockSize; ++i)
                {
                    free_list_.push(block + i);
                }
            }
        };

        /**
         * @brief RAII wrapper for memory pool objects
         *
         * This class provides automatic cleanup of objects allocated from a memory pool.
         *
         * @tparam T Type of object
         * @tparam BlockSize Block size for the memory pool
         */
        template <typename T, std::size_t BlockSize = 1024>
        class PooledObject
        {
        public:
            using value_type = T;
            using pointer = T *;
            using reference = T &;

            /**
             * @brief Construct a pooled object
             * @param pool Reference to the memory pool
             * @param args Arguments for object construction
             */
            template <typename... Args>
            PooledObject(MemoryPool<T, BlockSize> &pool, Args &&...args)
                : pool_(pool), ptr_(pool.construct(std::forward<Args>(args)...)) {}

            /**
             * @brief Destructor - automatically destroys and deallocates
             */
            ~PooledObject()
            {
                if (ptr_)
                {
                    pool_.destroy(ptr_);
                }
            }

            // Non-copyable, movable
            PooledObject(const PooledObject &) = delete;
            PooledObject &operator=(const PooledObject &) = delete;

            PooledObject(PooledObject &&other) noexcept
                : pool_(other.pool_), ptr_(other.ptr_)
            {
                other.ptr_ = nullptr;
            }

            PooledObject &operator=(PooledObject &&other) noexcept
            {
                if (this != &other)
                {
                    if (ptr_)
                    {
                        pool_.destroy(ptr_);
                    }
                    pool_ = other.pool_;
                    ptr_ = other.ptr_;
                    other.ptr_ = nullptr;
                }
                return *this;
            }

            /**
             * @brief Get pointer to the object
             * @return Pointer to object
             */
            pointer get() const
            {
                return ptr_;
            }

            /**
             * @brief Dereference operator
             * @return Reference to object
             */
            reference operator*() const
            {
                return *ptr_;
            }

            /**
             * @brief Arrow operator
             * @return Pointer to object
             */
            pointer operator->() const
            {
                return ptr_;
            }

            /**
             * @brief Check if object is valid
             * @return true if object exists
             */
            explicit operator bool() const
            {
                return ptr_ != nullptr;
            }

        private:
            MemoryPool<T, BlockSize> &pool_;
            pointer ptr_;
        };

        /**
         * @brief Global memory pool manager for common types
         *
         * This singleton provides access to pre-configured memory pools for
         * commonly used types in graph algorithms.
         */
        class MemoryPoolManager
        {
        public:
            // Get memory pool for a specific type
            template <typename T>
            static MemoryPool<T> &get_pool()
            {
                static MemoryPool<T> pool;
                return pool;
            }

            // Get memory pool for a specific type with custom block size
            template <typename T, std::size_t BlockSize>
            static MemoryPool<T, BlockSize> &get_pool()
            {
                static MemoryPool<T, BlockSize> pool;
                return pool;
            }

            // Get statistics for all pools
            static std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>> get_all_stats()
            {
                // This would need to be implemented with a registry of pools
                // For now, return empty vector
                return {};
            }
        };

    } // namespace utils
} // namespace graph_engine
