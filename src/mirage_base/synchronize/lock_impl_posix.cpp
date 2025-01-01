#ifndef MIRAGE_BUILD_MSVC

#include "mirage_base/synchronize/lock_impl.hpp"

#include <pthread.h>

using namespace mirage::base;

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
  auto* handle = new pthread_mutex_t();
  pthread_mutex_init(handle, nullptr);
  native_handle_ = static_cast<void*>(handle);
}

LockImpl::~LockImpl() {
  auto* handle = static_cast<pthread_mutex_t*>(native_handle_);
  pthread_mutex_destroy(handle);
  delete handle;
}

bool LockImpl::TryAcquire() const {
  auto* handle = static_cast<pthread_mutex_t*>(native_handle_);
  int rv = pthread_mutex_trylock(handle);
  return rv == 0;
}

void LockImpl::AcquireInternal() const {
  pthread_mutex_lock(static_cast<pthread_mutex_t*>(native_handle_));
}

void LockImpl::Release() const {
  pthread_mutex_unlock(static_cast<pthread_mutex_t*>(native_handle_));
}

#endif
