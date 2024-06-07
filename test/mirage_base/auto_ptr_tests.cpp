#include <gtest/gtest.h>

#include <thread>

#include "mirage_base/auto_ptr/owned.hpp"
#include "mirage_base/auto_ptr/ref_count.hpp"

using namespace mirage;

namespace {

struct Base {
  bool* base_destructed_{nullptr};

  explicit Base(bool* base_destructed) : base_destructed_(base_destructed) {}

  virtual ~Base() { *base_destructed_ = true; }
};

struct Derive : public Base {
  bool* derive_destructed_{nullptr};

  explicit Derive(bool* base_destructed, bool* derive_destructed)
      : Base(base_destructed), derive_destructed_(derive_destructed) {}

  ~Derive() override { *derive_destructed_ = true; }
};

}  // namespace

TEST(AutoPtrTests, OwnedConstruct) {
  // Default Construct
  Owned<int32_t> owned;
  EXPECT_TRUE(owned.IsNull());

  // List Construct
  owned = Owned<int32_t>::New(0);
  EXPECT_FALSE(owned.IsNull());

  // Raw construct
  int32_t x = 0;
  Owned<int32_t> raw_owned(&x, [](int32_t* ptr) { *ptr += 1; });
  owned = std::move(raw_owned);
  EXPECT_EQ(owned.Get(), &x);
  EXPECT_TRUE(raw_owned.IsNull());  // NOLINT
  raw_owned = nullptr;
  EXPECT_EQ(x, 0);  // Destructor won't be called since raw_owned is moved.

  // Move Construct
  Owned<int32_t> move_owned(std::move(owned));
  EXPECT_EQ(*move_owned, 0);
  EXPECT_TRUE(owned.IsNull());  // NOLINT
  move_owned = nullptr;
  EXPECT_EQ(x, 1);  // Destructor will be called since move_owned is destructed.
}

TEST(AutoPtrTests, OwnedPtrOps) {
  bool flag = false;
  Owned<Base> owned_flag = Owned<Base>::New(&flag);
  EXPECT_FALSE(*(owned_flag->base_destructed_));

  *((*owned_flag).base_destructed_) = true;
  EXPECT_TRUE(*(owned_flag->base_destructed_));
}

TEST(AutoPtrTests, OwnedConvertDeriveToBase) {
  bool base_destructed = false;
  bool derive_destructed = false;

  // Convert from derive to base is always successful.
  Owned<Derive> derive =
      Owned<Derive>::New(&base_destructed, &derive_destructed);
  Owned<Base> base(derive.TryConvert<Base>());
  EXPECT_TRUE(derive.IsNull());
  EXPECT_FALSE(base.IsNull());
  base = nullptr;

  // Even holder is base, derive destructor is still called.
  EXPECT_TRUE(base_destructed);
  EXPECT_TRUE(derive_destructed);
}

TEST(AutoPtrTests, OwnedConvertBaseToDerive) {
  // Can't convert from base to derive when base is the origin type.
  bool base_destructed = false;
  Owned<Base> base = Owned<Base>::New(&base_destructed);
  Owned<Derive> derive_from_base = base.TryConvert<Derive>();
  EXPECT_TRUE(derive_from_base.IsNull());
  EXPECT_FALSE(base.IsNull());
  EXPECT_FALSE(base_destructed);

  // Convert from base to derive when derive is the origin type.
  bool derive_destructed = false;
  Owned<Derive> derive =
      Owned<Derive>::New(&base_destructed, &derive_destructed);
  Owned<Base> base_from_derive = derive.TryConvert<Base>();
  derive = base_from_derive.TryConvert<Derive>();
  EXPECT_FALSE(derive.IsNull());
  EXPECT_TRUE(base_from_derive.IsNull());
  EXPECT_FALSE(base_destructed);
  EXPECT_FALSE(derive_destructed);
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
