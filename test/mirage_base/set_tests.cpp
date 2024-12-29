#include <gtest/gtest.h>

#include "mirage_base/container/set.hpp"

using namespace mirage;

TEST(SetTests, Construct) {
  const Set<int32_t> set;
  const MultiSet<int32_t> multi_set;
  EXPECT_TRUE(set.IsEmpty());
  EXPECT_TRUE(multi_set.IsEmpty());
}

TEST(SetTests, IterateSet) {
  EXPECT_TRUE(std::bidirectional_iterator<Set<int32_t>::ConstIterator>);
  const Set set = {0, 1, 2};
}
