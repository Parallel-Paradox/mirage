#include <gtest/gtest.h>

#include "mirage_base/auto_ptr/owned.hpp"
#include "mirage_base/container/array.hpp"

using namespace mirage::base;

namespace {

struct Counter final {
  int32_t* base_destructed{nullptr};

  explicit Counter(int32_t* base_destructed)
      : base_destructed(base_destructed) {}

  ~Counter() { *base_destructed += 1; }
};

}  // namespace

TEST(ArrayTests, Construct) {
  Array<int32_t> array = {0, 1, 2};

  const Array<int32_t> copy_array(array);
  EXPECT_EQ(array, copy_array);

  int32_t* raw_ptr = array.GetRawPtr();
  const Array<int32_t> move_array(std::move(array));
  EXPECT_TRUE(array.IsEmpty());  // NOLINT(*-use-after-move): Allow for test.
  EXPECT_EQ(array.GetRawPtr(), nullptr);
  EXPECT_EQ(raw_ptr, move_array.GetRawPtr());
  EXPECT_EQ(move_array, copy_array);
}

TEST(ArrayTests, DestructAfterMoved) {
  int32_t destruct_cnt = 0;
  {
    Array<Owned<Counter>> src;
    src.Emplace(Owned<Counter>::New(&destruct_cnt));
    src.Emplace(Owned<Counter>::New(&destruct_cnt));
    Array<Owned<Counter>> dst(std::move(src));
    Owned<Counter> pop = dst.Pop();
    EXPECT_EQ(destruct_cnt, 0);
  }
  EXPECT_EQ(destruct_cnt, 2);
}

TEST(ArrayTests, ChangeSizeAndCapacity) {
  Array<int32_t> array = {0, 1, 2};
  EXPECT_EQ(array.GetSize(), 3);
  EXPECT_EQ(array.GetCapacity(), 3);

  array.Push(3);
  EXPECT_EQ(array.GetSize(), 4);
  EXPECT_EQ(array.GetCapacity(), 6);

  array.Reserve(5);
  EXPECT_EQ(array.GetSize(), 4);
  EXPECT_EQ(array.GetCapacity(), 6);

  array.SetCapacity(5);
  EXPECT_EQ(array.GetSize(), 4);
  EXPECT_EQ(array.GetCapacity(), 5);

  array.SetSize(5);
  EXPECT_EQ(array.GetSize(), 5);
  EXPECT_EQ(array[4], 0);
  EXPECT_EQ(array.GetCapacity(), 5);

  array.SetSize(3);
  EXPECT_EQ(array.GetSize(), 3);
  EXPECT_EQ(array.GetCapacity(), 5);
}

TEST(ArrayTests, CompareEquality) {
  const Array<int32_t> array_a = {0, 1, 2};
  const Array<int32_t> array_b = {2, 1, 0};
  EXPECT_EQ(array_a, array_a);
  EXPECT_NE(array_a, array_b);
}

TEST(ArrayTests, IterateArray) {
  EXPECT_TRUE(std::contiguous_iterator<Array<int32_t>::Iterator>);
  EXPECT_TRUE(std::contiguous_iterator<Array<int32_t>::ConstIterator>);
  Array<int32_t> array = {0, 1, 2};
  for (auto iter = array.begin(); iter != array.end(); ++iter) {  // NOLINT
    *iter += 1;
  }
  for (auto& num : array) {
    num += 1;
  }
  const Array compare = {2, 3, 4};
  const auto arr_iter = array.begin();
  const auto cmp_iter = compare.begin();
  for (int32_t i = 0; i < 3; ++i) {
    EXPECT_EQ(arr_iter[i], cmp_iter[i]);
  }
}
