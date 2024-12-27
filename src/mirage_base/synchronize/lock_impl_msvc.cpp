#ifdef MIRAGE_BUILD_MSVC

#include "mirage_base/synchronize/lock_impl.hpp"

#include <windows.h>

using namespace mirage;

void LockImpl::Acquire() const {
  // Try the lock first to acquire it cheaply if it's not contended. Try() is
  // cheap on platforms with futex-type locks, as it doesn't call into the
  // kernel.
  if (TryAcquire()) {
    return;
  }
  AcquireInternal();
}

LockImpl::LockImpl() {
  native_handle_ = new SRWLOCK();
  InitializeSRWLock(static_cast<SRWLOCK*>(native_handle_));
}

LockImpl::~LockImpl() {
  delete static_cast<SRWLOCK*>(native_handle_);
}

bool LockImpl::TryAcquire() const {
  return TryAcquireSRWLockExclusive(static_cast<SRWLOCK*>(native_handle_));
}

void LockImpl::AcquireInternal() const {
  AcquireSRWLockExclusive(static_cast<SRWLOCK*>(native_handle_));
}

void LockImpl::Release() const {
  ReleaseSRWLockExclusive(static_cast<SRWLOCK*>(native_handle_));
}

#endif
