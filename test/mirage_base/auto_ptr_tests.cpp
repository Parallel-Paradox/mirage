#include <gtest/gtest.h>

#include <thread>

#include "mirage_base/auto_ptr/owned.hpp"
#include "mirage_base/auto_ptr/ref_count.hpp"

using namespace mirage::base;

namespace {

struct Base {
  int32_t* base_destructed{nullptr};

  explicit Base(int32_t* base_destructed) : base_destructed(base_destructed) {}

  virtual ~Base() { *base_destructed += 1; }
};

struct Derive final : Base {
  int32_t* derive_destructed{nullptr};

  explicit Derive(int32_t* base_destructed, int32_t* derive_destructed)
      : Base(base_destructed), derive_destructed(derive_destructed) {}

  ~Derive() override { *derive_destructed += 1; }
};

}  // namespace

TEST(AutoPtrTests, OwnedConstruct) {
  int32_t is_destructed = 0;

  // Default Construct
  Owned<Base> owned;
  EXPECT_TRUE(owned.IsNull());

  // List Construct
  owned = Owned<Base>::New(&is_destructed);
  EXPECT_FALSE(owned.IsNull());

  // Raw construct and set with move
  const auto base = new Base(&is_destructed);
  auto raw_owned = Owned(base);
  owned = std::move(raw_owned);
  EXPECT_EQ(owned.Get(), base);
  EXPECT_TRUE(raw_owned.IsNull());

  EXPECT_EQ(is_destructed, 1);
  raw_owned = nullptr;  // NOLINT: Test nullptr setter
  EXPECT_EQ(is_destructed, 1);

  // Move Construct
  auto move_owned = Owned(std::move(owned));
  EXPECT_EQ(move_owned->base_destructed, &is_destructed);
  EXPECT_TRUE(owned.IsNull());

  EXPECT_EQ(is_destructed, 1);
  move_owned = nullptr;  // NOLINT: Test nullptr setter
  EXPECT_EQ(is_destructed, 2);
}

TEST(AutoPtrTests, OwnedPtrOps) {
  int32_t cnt = 0;
  const auto owned_flag = Owned<Base>::New(&cnt);
  EXPECT_EQ(*owned_flag->base_destructed, 0);

  *owned_flag->base_destructed = true;
  EXPECT_EQ(*owned_flag->base_destructed, 1);
}

TEST(AutoPtrTests, OwnedConvertDeriveToBase) {
  int32_t base_destructed = 0;
  int32_t derive_destructed = 0;

  // Convert from derive to base is always successful.
  auto derive = Owned<Derive>::New(&base_destructed, &derive_destructed);
  Owned<Base> base = derive.Convert<Base>();
  EXPECT_TRUE(derive.IsNull());
  EXPECT_FALSE(base.IsNull());
  base.Reset();

  // Even holder is base, derive destructor is still called.
  EXPECT_EQ(base_destructed, 1);
  EXPECT_EQ(derive_destructed, 1);
}

TEST(AutoPtrTests, OwnedConvertBaseToDerive) {
  // Can't convert from base to derive when base is the origin type.
  int32_t base_destructed = 0;
  auto base = Owned<Base>::New(&base_destructed);
  const Owned<Derive> derive_from_base = base.TryConvert<Derive>();
  EXPECT_TRUE(derive_from_base.IsNull());
  EXPECT_FALSE(base.IsNull());
  EXPECT_EQ(base_destructed, 0);

  // Convert from base to derive when derive is the origin type.
  int32_t derive_destructed = 0;
  auto derive = Owned<Derive>::New(&base_destructed, &derive_destructed);
  Owned<Base> base_from_derive = derive.TryConvert<Base>();
  derive = base_from_derive.TryConvert<Derive>();
  EXPECT_FALSE(derive.IsNull());
  EXPECT_TRUE(base_from_derive.IsNull());
  EXPECT_EQ(base_destructed, 0);
  EXPECT_EQ(derive_destructed, 0);
}

TEST(AutoPtrTests, RefCountOps) {
  EXPECT_TRUE(AsRefCount<RefCountLocal>);
  EXPECT_TRUE(AsRefCount<RefCountAsync>);

  auto checker = [](RefCount* count) {
    EXPECT_EQ(count->GetCnt(), 0);

    // Can't increase when cnt is 0
    bool increase = count->TryIncrease();
    EXPECT_FALSE(increase);
    EXPECT_EQ(count->GetCnt(), 0);

    // Release if cnt is already 0, cnt won't change
    bool release = count->TryRelease();
    EXPECT_TRUE(release);
    EXPECT_EQ(count->GetCnt(), 0);

    // Force to increase when cnt is 0
    count->Increase();
    EXPECT_EQ(count->GetCnt(), 1);

    // Allowed to increase when cnt > 0
    increase = count->TryIncrease();
    EXPECT_TRUE(increase);
    EXPECT_EQ(count->GetCnt(), 2);

    // Decrease cnt, only release if cnt = 0 after decreased
    release = count->TryRelease();
    EXPECT_FALSE(release);
    EXPECT_EQ(count->GetCnt(), 1);
    release = count->TryRelease();
    EXPECT_TRUE(release);
    EXPECT_EQ(count->GetCnt(), 0);
  };

  RefCountLocal count_local;
  checker(&count_local);

  RefCountAsync count_async;
  checker(&count_async);
}

TEST(AutoPtrTests, CountAsync) {
  RefCountAsync count_async;
  auto async_operation = [&count_async] {
    for (int32_t i = 0; i < 10000; ++i) {
      count_async.Increase();
      count_async.TryIncrease();
      count_async.TryRelease();
    }
  };
  std::thread async_thread(async_operation);
  async_operation();
  async_thread.join();
  EXPECT_EQ(count_async.GetCnt(), 20000);
}
