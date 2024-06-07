#include <gtest/gtest.h>

#include "mirage_base/auto_ptr/owned.hpp"
#include "mirage_base/container/singly_linked_list.hpp"

using namespace mirage;

TEST(SinglyLinkedListTests, Iterate) {
  using Iter = SinglyLinkedList<int32_t>::Iterator;
  using ConstIter = SinglyLinkedList<int32_t>::ConstIterator;

  EXPECT_TRUE(std::forward_iterator<SinglyLinkedList<int32_t>::Iterator>);
  EXPECT_TRUE(std::forward_iterator<SinglyLinkedList<int32_t>::ConstIterator>);

  SinglyLinkedList<int32_t> list = {0, 1, 2};

  int cnt = 0;
  for (int32_t num : list) {
    EXPECT_EQ(cnt, num);
    ++cnt;
  }
  EXPECT_EQ(cnt, 3);

  cnt = 0;
  for (Iter iter = list.begin(); iter != list.end(); ++iter) {  // NOLINT
    EXPECT_EQ(cnt, *iter);
    ++cnt;
  }
  EXPECT_EQ(cnt, 3);

  cnt = 0;
  const SinglyLinkedList<int32_t>& const_list = list;
  // NOLINTNEXTLINE
  for (ConstIter iter = const_list.begin(); iter != const_list.end(); ++iter) {
    EXPECT_EQ(cnt, *iter);
    ++cnt;
  }
  EXPECT_EQ(cnt, 3);
}

TEST(SinglyLinkedListTests, Destruct) {
  int32_t destruct_cnt = 0;
  auto destructor = [](int32_t* ptr) {
    *ptr += 1;
  };

  {
    SinglyLinkedList<Owned<int32_t>> list;
    list.EmplaceHead(Owned<int32_t>(&destruct_cnt, destructor));
    list.begin().EmplaceAfter(Owned<int32_t>(&destruct_cnt, destructor));
  }
  EXPECT_EQ(destruct_cnt, 2);
}

TEST(SinglyLinkedListTests, Remove) {
  SinglyLinkedList<int32_t> list = {0, 1};
  EXPECT_EQ(list.begin().RemoveAfter(), 1);
  EXPECT_EQ(list.RemoveHead(), 0);
  EXPECT_EQ(list.begin(), list.end());
}

TEST(SinglyLinkedListTests, MoveAndCopy) {
  SinglyLinkedList<int32_t> list = {0, 1};
  SinglyLinkedList<int32_t> move_list(std::move(list));
  SinglyLinkedList<int32_t> copy_list(move_list);

  EXPECT_EQ(list.begin(), list.end());  // NOLINT

  int cnt = 0;
  for (int32_t num : move_list) {
    EXPECT_EQ(cnt, num);
    ++cnt;
  }
  EXPECT_EQ(cnt, 2);

  cnt = 0;
  for (int32_t num : copy_list) {
    EXPECT_EQ(cnt, num);
    ++cnt;
  }
  EXPECT_EQ(cnt, 2);
}
