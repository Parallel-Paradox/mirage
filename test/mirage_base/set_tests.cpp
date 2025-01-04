#include <gtest/gtest.h>

#include "mirage_base/container/set.hpp"

using namespace mirage::base;

TEST(SetTests, Construct) {
  const Set<int32_t> set;
  const MultiSet<int32_t> multi_set;
  EXPECT_TRUE(set.IsEmpty());
  EXPECT_TRUE(multi_set.IsEmpty());
}

TEST(SetTests, Insert) {
  Set<int32_t> set;
  EXPECT_TRUE(set.IsEmpty());
  const auto val_none = set.Insert(0);
  set.Insert(1);
  auto val_some = set.Insert(0);
  EXPECT_EQ(set.GetSize(), 2);
  EXPECT_FALSE(val_none.IsValid());
  EXPECT_EQ(val_some.Unwrap(), 0);
  EXPECT_EQ(set.Count(0), 1);
  EXPECT_EQ(set.Count(1), 1);
  EXPECT_EQ(set.Count(2), 0);

  MultiSet<int32_t> multi_set;
  EXPECT_TRUE(multi_set.IsEmpty());
  multi_set.Insert(0);
  multi_set.Insert(1);
  multi_set.Insert(0);
  EXPECT_EQ(multi_set.GetSize(), 3);
  EXPECT_EQ(multi_set.Count(0), 2);
  EXPECT_EQ(multi_set.Count(1), 1);
  EXPECT_EQ(multi_set.Count(2), 0);
}

TEST(SetTests, Remove) {
  Set<int32_t> set = {0, 1, 0, 2};
  EXPECT_EQ(set.GetSize(), 3);
  int32_t removed = set.Remove(0).Unwrap();
  EXPECT_EQ(set.Count(0), 0);
  EXPECT_EQ(removed, 0);
  EXPECT_EQ(set.GetSize(), 2);
  EXPECT_FALSE(set.Remove(-1).IsValid());

  MultiSet<int32_t> multi_set = {0, 1, 0, 2};
  EXPECT_EQ(multi_set.GetSize(), 4);
  removed = multi_set.Remove(0).Unwrap();
  EXPECT_EQ(multi_set.Count(0), 1);
  EXPECT_EQ(removed, 0);
  EXPECT_EQ(multi_set.GetSize(), 3);
  EXPECT_FALSE(multi_set.Remove(-1).IsValid());
}

TEST(SetTests, Iterate) {
  EXPECT_TRUE(std::bidirectional_iterator<Set<int32_t>::ConstIterator>);
  const Set<int32_t> set = {0, 3, 2, 1, 5, 4};
  const Array<int32_t> expected = {0, 1, 2, 3, 4, 5};
  Array<int32_t> actual;
  for (int32_t num : set) {
    actual.Push(num);
  }
  EXPECT_EQ(expected, actual);
}

TEST(SetTests, RemoveBoundary) {
  Set<int32_t> set = {0};
  const int32_t removed = set.Remove(0).Unwrap();
  const auto remove_again = set.Remove(0);
  EXPECT_EQ(set.GetSize(), 0);
  EXPECT_EQ(removed, 0);
  EXPECT_FALSE(remove_again.IsValid());
}
