#include <gtest/gtest.h>
#include "graph_engine/utils/union_find.hpp"
#include "graph_engine/utils/memory_pool.hpp"

using namespace graph_engine::utils;

class UnionFindTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        uf_ = UnionFind<int>();
    }

    UnionFind<int> uf_;
};

TEST_F(UnionFindTest, BasicOperations)
{
    // Add elements
    uf_.make_set(1);
    uf_.make_set(2);
    uf_.make_set(3);

    EXPECT_EQ(uf_.size(), 3);
    EXPECT_EQ(uf_.num_sets(), 3);

    // Test find
    EXPECT_EQ(uf_.find(1), 1);
    EXPECT_EQ(uf_.find(2), 2);
    EXPECT_EQ(uf_.find(3), 3);

    // Test union
    EXPECT_TRUE(uf_.union_sets(1, 2));
    EXPECT_EQ(uf_.num_sets(), 2);
    EXPECT_TRUE(uf_.same_set(1, 2));
    EXPECT_FALSE(uf_.same_set(1, 3));

    // Union again (should return false)
    EXPECT_FALSE(uf_.union_sets(1, 2));

    // Union with third element
    EXPECT_TRUE(uf_.union_sets(2, 3));
    EXPECT_EQ(uf_.num_sets(), 1);
    EXPECT_TRUE(uf_.same_set(1, 3));
}

TEST_F(UnionFindTest, PathCompression)
{
    // Create a chain: 1 -> 2 -> 3 -> 4
    uf_.make_set(1);
    uf_.make_set(2);
    uf_.make_set(3);
    uf_.make_set(4);

    uf_.union_sets(1, 2);
    uf_.union_sets(2, 3);
    uf_.union_sets(3, 4);

    // All should be in the same set
    EXPECT_TRUE(uf_.same_set(1, 4));
    EXPECT_TRUE(uf_.same_set(2, 4));
    EXPECT_TRUE(uf_.same_set(3, 4));
    EXPECT_EQ(uf_.num_sets(), 1);
}

TEST_F(UnionFindTest, GetSetMembers)
{
    uf_.make_set(1);
    uf_.make_set(2);
    uf_.make_set(3);
    uf_.make_set(4);

    uf_.union_sets(1, 2);
    uf_.union_sets(3, 4);

    auto set1 = uf_.get_set_members(1);
    auto set3 = uf_.get_set_members(3);

    EXPECT_EQ(set1.size(), 2);
    EXPECT_EQ(set3.size(), 2);

    // Check that sets are disjoint
    std::unordered_set<int> set1_set(set1.begin(), set1.end());
    std::unordered_set<int> set3_set(set3.begin(), set3.end());

    for (int elem : set1_set)
    {
        EXPECT_TRUE(set3_set.find(elem) == set3_set.end());
    }
}

TEST_F(UnionFindTest, Clear)
{
    uf_.make_set(1);
    uf_.make_set(2);
    uf_.union_sets(1, 2);

    EXPECT_EQ(uf_.size(), 2);
    EXPECT_EQ(uf_.num_sets(), 1);

    uf_.clear();

    EXPECT_EQ(uf_.size(), 0);
    EXPECT_EQ(uf_.num_sets(), 0);
}

TEST_F(UnionFindTest, StringElements)
{
    UnionFind<std::string> string_uf;

    string_uf.make_set("A");
    string_uf.make_set("B");
    string_uf.make_set("C");

    EXPECT_EQ(string_uf.size(), 3);
    EXPECT_EQ(string_uf.num_sets(), 3);

    string_uf.union_sets("A", "B");
    EXPECT_TRUE(string_uf.same_set("A", "B"));
    EXPECT_FALSE(string_uf.same_set("A", "C"));
}

TEST_F(UnionFindTest, UnionFindInt)
{
    UnionFindInt<int> int_uf(10);

    EXPECT_EQ(int_uf.num_sets(), 11); // 0 to 10

    int_uf.union_sets(1, 2);
    int_uf.union_sets(2, 3);

    EXPECT_TRUE(int_uf.same_set(1, 3));
    EXPECT_EQ(int_uf.num_sets(), 9); // 11 - 2 unions

    int_uf.clear();
    EXPECT_EQ(int_uf.num_sets(), 11);
}

class MemoryPoolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pool_ = std::make_unique<MemoryPool<int>>();
    }

    std::unique_ptr<MemoryPool<int>> pool_;
};

TEST_F(MemoryPoolTest, BasicAllocation)
{
    EXPECT_EQ(pool_->size(), 0);
    EXPECT_EQ(pool_->capacity(), 1024); // Default block size

    // Allocate some objects
    auto ptr1 = pool_->allocate();
    auto ptr2 = pool_->allocate();

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr1, ptr2);

    EXPECT_EQ(pool_->size(), 2);

    // Deallocate
    pool_->deallocate(ptr1);
    pool_->deallocate(ptr2);

    EXPECT_EQ(pool_->size(), 0);
}

TEST_F(MemoryPoolTest, ConstructAndDestroy)
{
    // Construct objects
    auto ptr1 = pool_->construct(42);
    auto ptr2 = pool_->construct(100);

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_EQ(*ptr1, 42);
    EXPECT_EQ(*ptr2, 100);

    EXPECT_EQ(pool_->size(), 2);

    // Destroy objects
    pool_->destroy(ptr1);
    pool_->destroy(ptr2);

    EXPECT_EQ(pool_->size(), 0);
}

TEST_F(MemoryPoolTest, PooledObject)
{
    // Test RAII wrapper
    {
        auto obj1 = PooledObject<int>(*pool_, 42);
        auto obj2 = PooledObject<int>(*pool_, 100);

        EXPECT_TRUE(obj1);
        EXPECT_TRUE(obj2);
        EXPECT_EQ(*obj1, 42);
        EXPECT_EQ(*obj2, 100);

        EXPECT_EQ(pool_->size(), 2);
    }

    // Objects should be automatically destroyed
    EXPECT_EQ(pool_->size(), 0);
}

TEST_F(MemoryPoolTest, MemoryStats)
{
    auto stats = pool_->memory_stats();
    EXPECT_EQ(stats.first, 0);     // used
    EXPECT_EQ(stats.second, 1024); // total capacity

    auto ptr = pool_->allocate();
    stats = pool_->memory_stats();
    EXPECT_EQ(stats.first, 1);
    EXPECT_EQ(stats.second, 1024);

    pool_->deallocate(ptr);
    stats = pool_->memory_stats();
    EXPECT_EQ(stats.first, 0);
    EXPECT_EQ(stats.second, 1024);
}

TEST_F(MemoryPoolTest, CustomBlockSize)
{
    MemoryPool<int, 4> small_pool; // Block size of 4

    EXPECT_EQ(small_pool.capacity(), 4);

    // Allocate more than block size
    std::vector<int *> ptrs;
    for (int i = 0; i < 6; ++i)
    {
        ptrs.push_back(small_pool.allocate());
    }

    EXPECT_EQ(small_pool.size(), 6);
    EXPECT_EQ(small_pool.capacity(), 8); // Should have allocated 2 blocks

    // Deallocate all
    for (auto ptr : ptrs)
    {
        small_pool.deallocate(ptr);
    }

    EXPECT_EQ(small_pool.size(), 0);
}

TEST_F(MemoryPoolTest, StringPool)
{
    MemoryPool<std::string> string_pool;

    auto ptr1 = string_pool.construct("Hello");
    auto ptr2 = string_pool.construct("World");

    EXPECT_EQ(*ptr1, "Hello");
    EXPECT_EQ(*ptr2, "World");

    string_pool.destroy(ptr1);
    string_pool.destroy(ptr2);

    EXPECT_EQ(string_pool.size(), 0);
}

TEST_F(MemoryPoolTest, ExceptionSafety)
{
    // Test that exceptions don't leak memory
    try
    {
        auto ptr = pool_->construct(42);
        throw std::runtime_error("Test exception");
    }
    catch (const std::exception &)
    {
        // Exception caught, memory should be cleaned up
    }

    // Pool should still be functional
    auto ptr = pool_->allocate();
    EXPECT_NE(ptr, nullptr);
    pool_->deallocate(ptr);
}

TEST_F(MemoryPoolTest, MemoryPoolManager)
{
    // Test global memory pool manager
    auto &pool1 = MemoryPoolManager::get_pool<int>();
    auto &pool2 = MemoryPoolManager::get_pool<int>();

    // Should return the same instance
    EXPECT_EQ(&pool1, &pool2);

    // Test with custom block size
    auto &custom_pool = MemoryPoolManager::get_pool<int, 512>();
    EXPECT_EQ(custom_pool.capacity(), 512);
}
