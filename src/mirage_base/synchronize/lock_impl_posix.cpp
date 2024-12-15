#ifndef MIRAGE_BUILD_MSVC

#include "mirage_base/synchronize/lock_impl.hpp"

#include <pthread.h>

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
  auto* handle = new pthread_mutex_t();
  pthread_mutex_init(handle, nullptr);
  native_handle_ = reinterpret_cast<void*>(handle);
}

LockImpl::~LockImpl() {
  auto* handle = reinterpret_cast<pthread_mutex_t*>(native_handle_);
  pthread_mutex_destroy(handle);
  delete handle;
}

bool LockImpl::TryAcquire() {
  auto* handle = reinterpret_cast<pthread_mutex_t*>(native_handle_);
  int rv = pthread_mutex_trylock(handle);
  return rv == 0;
}

void LockImpl::AcquireInternal() {
  pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(native_handle_));
}

void LockImpl::Release() {
  pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t*>(native_handle_));
}

#endif
