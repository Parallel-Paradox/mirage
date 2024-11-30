#include <gtest/gtest.h>

#include "mirage_base/container/hash_map.hpp"

using namespace mirage;

TEST(HashMapTests, Construct) {
  using KVPair = HashMap<size_t, size_t>::KVPair;
  HashMap<size_t, size_t> map = {KVPair(0, 1), KVPair(1, 2), KVPair(2, 3)};

  // HashMap<size_t, size_t> move_map(std::move(map));
  // EXPECT_TRUE(map.GetSize());
  // EXPECT_EQ(move_map.GetSize(), 3);

  // HashMap<size_t, size_t> copy_map(move_map);
  // EXPECT_EQ(move_map.GetSize(), 3);
  // EXPECT_EQ(copy_map.GetSize(), 3);
}

// TEST(HashMapTests, CommonOperations) {
//   using KVPair = HashMap<size_t, size_t>::KVPair;
//   HashMap<size_t, size_t> map = {KVPair(0, 1), KVPair(1, 2), KVPair(2, 3)};
//   EXPECT_FALSE(map.IsEmpty());
//   EXPECT_EQ(map.GetSize(), 3);

//   // Insert new, find exist
//   Optional<size_t> old = map.Insert(3, 4);
//   EXPECT_EQ(map.GetSize(), 4);
//   EXPECT_FALSE(old.IsValid());
//   EXPECT_EQ(map.Find(3), 4);

//   // Insert exist
//   old = map.Insert(3, 5);
//   EXPECT_EQ(map.GetSize(), 4);
//   EXPECT_TRUE(old.IsValid());
//   if (old.IsValid())
//     EXPECT_EQ(old.Unwrap(), 4);
//   EXPECT_EQ(map[3], 5);

//   // Remove exist, find not exist
//   old = map.Remove(3);
//   EXPECT_EQ(map.GetSize(), 3);
//   EXPECT_TRUE(old.IsValid());
//   if (old.IsValid())
//     EXPECT_EQ(old.Unwrap(), 5);
//   size_t* ptr = map.TryFind(3);
//   EXPECT_EQ(ptr, nullptr);

//   // Remove not exist
//   old = map.Remove(3);
//   EXPECT_EQ(map.GetSize(), 3);
//   EXPECT_FALSE(old.IsValid());

//   // Index operator
//   EXPECT_EQ(map[0], 1);

//   // Clear
//   map.Clear();
//   EXPECT_TRUE(map.IsEmpty());
//   EXPECT_EQ(map.GetSize(), 0);
// }
