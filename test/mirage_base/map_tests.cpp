#include <gtest/gtest.h>

#include "mirage_base/container/map.hpp"

using namespace mirage::base;

TEST(MapTests, OrderOfKey) {
  const Map<int32_t, int32_t>::Entry kv_1(1, 1);
  const Map<int32_t, int32_t>::Entry kv_2(2, -1);
  const Map<int32_t, int32_t>::Entry kv_3 = {1, -1};
  EXPECT_LE(kv_1, kv_2);
  EXPECT_EQ(kv_1, kv_3);
  EXPECT_GE(kv_2, kv_3);
  EXPECT_EQ(kv_1, kv_3);
}

TEST(MapTests, Construct) {
  Map<int32_t, int32_t> map;
  MultiMap<int32_t, int32_t> multi_map;

  // todo
  map.Insert(1, 1);
  multi_map.Remove(1);
  map.Remove(1);
}
