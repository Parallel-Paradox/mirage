#ifndef MIRAGE_BASE_SYNCHRONIZE_LOCK_IMPL
#define MIRAGE_BASE_SYNCHRONIZE_LOCK_IMPL

#include "mirage_base/define.hpp"

namespace mirage {

class MIRAGE_API LockImpl {
 public:
  using NativeHandle = void*;

  LockImpl();
  LockImpl(const LockImpl&) = delete;
  ~LockImpl();

  [[nodiscard]] bool TryAcquire() const;
  void Acquire() const;
  void Release() const;

 private:
  void AcquireInternal() const;

  NativeHandle native_handle_;
};

}  // namespace mirage

#endif  // MIRAGE_BASE_SYNCHRONIZE_LOCK_IMPL
