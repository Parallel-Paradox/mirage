#include <gtest/gtest.h>

#include "mirage_base/util/hash.hpp"
#include "mirage_base/util/optional.hpp"

using namespace mirage;

namespace {

struct Empty {};

struct HashOnly {};

struct EqHash {
  bool operator==(const EqHash&) const { return true; }
};

}  // namespace

template <>
struct mirage::Hash<HashOnly> {
  size_t operator()(const HashOnly&) const { return 0; }
};

template <>
struct mirage::Hash<EqHash> {
  size_t operator()(const EqHash&) const { return 0; }
};

TEST(UtilTests, HashConcept) {
  EXPECT_FALSE(HashKeyType<Empty>);
  EXPECT_FALSE(HashKeyType<HashOnly>);
  EXPECT_TRUE(HashKeyType<EqHash>);

  EXPECT_TRUE(HashKeyType<size_t>);
  EXPECT_EQ(mirage::Hash<size_t>()(13), 13);
}

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
  Optional move_num(std::move(num));
  EXPECT_FALSE(num.IsValid());  // NOLINT(*-use-after-move): Allow for test.
  EXPECT_TRUE(move_num.IsValid());
  EXPECT_EQ(move_num.Unwrap(), 1);
}
