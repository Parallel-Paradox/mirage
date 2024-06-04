#include "mirage_base/auto_ptr/ref_count.hpp"

using namespace mirage;

size_t RefCountLocal::GetCnt() {
  return cnt_;
}

void RefCountLocal::Increase() {
  cnt_ += 1;
}

bool RefCountLocal::TryIncrease() {
  if (cnt_ == 0) {
    return false;
  }
  cnt_ += 1;
  return true;
}

bool RefCountLocal::TryRelease() {
  if (cnt_ == 0) {
    return true;
  }
  cnt_ -= 1;
  return cnt_ == 0;
}

size_t RefCountAsync::GetCnt() {
  LockGuard lock(lock_);
  return RefCountLocal::GetCnt();
}

void RefCountAsync::Increase() {
  LockGuard lock(lock_);
  RefCountLocal::Increase();
}

bool RefCountAsync::TryIncrease() {
  LockGuard lock(lock_);
  return RefCountLocal::TryIncrease();
}

bool RefCountAsync::TryRelease() {
  LockGuard lock(lock_);
  return RefCountLocal::TryRelease();
}
