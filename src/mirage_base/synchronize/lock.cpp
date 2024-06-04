#include "mirage_base/synchronize/lock.hpp"

using namespace mirage;

bool Lock::TryAcquire() {
  return lock_.TryAcquire();
}

void Lock::Acquire() {
  lock_.Acquire();
}

void Lock::Release() {
  lock_.Release();
}

LockGuard::LockGuard(Lock& lock) : lock_(lock) {
  lock.Acquire();
}

LockGuard::~LockGuard() {
  lock_.Release();
}
