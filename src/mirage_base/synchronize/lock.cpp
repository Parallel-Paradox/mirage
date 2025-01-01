#include "mirage_base/synchronize/lock.hpp"

using namespace mirage::base;

bool Lock::TryAcquire() const {
  return lock_.TryAcquire();
}

void Lock::Acquire() const {
  lock_.Acquire();
}

void Lock::Release() const {
  lock_.Release();
}

LockGuard::LockGuard(Lock& lock) : lock_(lock) {
  lock.Acquire();
}

LockGuard::~LockGuard() {
  lock_.Release();
}
