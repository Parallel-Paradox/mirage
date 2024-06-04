#ifndef MIRAGE_BASE_SYNCHRONIZE_LOCK
#define MIRAGE_BASE_SYNCHRONIZE_LOCK

#include "mirage_base/define.hpp"
#include "mirage_base/synchronize/lock_impl.hpp"

namespace mirage {

class MIRAGE_API Lock {
 public:
  Lock() = default;
  ~Lock() = default;

  bool TryAcquire();
  void Acquire();
  void Release();

 private:
  LockImpl lock_;
};

class MIRAGE_API LockGuard {
 public:
  LockGuard() = delete;
  LockGuard(const LockGuard&) = delete;

  explicit LockGuard(Lock& lock);
  ~LockGuard();

 private:
  Lock& lock_;
};

}  // namespace mirage

#endif  // MIRAGE_BASE_SYNCHRONIZE_LOCK
