#include <gtest/gtest.h>

#include "mirage_base/util/optional.hpp"

using namespace mirage;

TEST(UtilTests, UnwrapOptional) {
  auto num = Optional<int32_t>::None();
  EXPECT_FALSE(num.IsValid());
  // num.Unwrap(); // Panic if try to unwrap invalid Optional.

  num = Optional<int32_t>::New(1);
  EXPECT_TRUE(num.IsValid());
  EXPECT_EQ(num.Unwrap(), 1);
  EXPECT_FALSE(num.IsValid());
}

TEST(UtilTests, MoveOptional) {
  auto num = Optional<int32_t>::New(1);
  EXPECT_TRUE(num.IsValid());
  Optional<int> move_num(std::move(num));
  EXPECT_FALSE(num.IsValid());  // NOLINT(*-use-after-move): Allow for test.
  EXPECT_TRUE(move_num.IsValid());
  EXPECT_EQ(move_num.Unwrap(), 1);
}
