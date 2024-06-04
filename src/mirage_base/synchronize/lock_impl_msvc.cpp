#include "mirage_base/synchronize/lock_impl.hpp"

#include <windows.h>

using namespace mirage;

void LockImpl::Acquire() {
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
  InitializeSRWLock(reinterpret_cast<SRWLOCK*>(native_handle_));
}

LockImpl::~LockImpl() {
  delete reinterpret_cast<SRWLOCK*>(native_handle_);
}

bool LockImpl::TryAcquire() {
  return TryAcquireSRWLockExclusive(reinterpret_cast<SRWLOCK*>(native_handle_));
}

void LockImpl::AcquireInternal() {
  AcquireSRWLockExclusive(reinterpret_cast<SRWLOCK*>(native_handle_));
}

void LockImpl::Release() {
  ReleaseSRWLockExclusive(reinterpret_cast<SRWLOCK*>(native_handle_));
}
