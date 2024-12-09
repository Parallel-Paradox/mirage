#ifndef MIRAGE_BASE_UTIL_OPTIONAL
#define MIRAGE_BASE_UTIL_OPTIONAL

#include <concepts>

#include "mirage_base/define.hpp"
#include "mirage_base/util/aligned_memory.hpp"

namespace mirage {

template <std::move_constructible T>
class Optional {
 public:
  Optional(const Optional&) = delete;

  Optional(Optional&& other) noexcept : is_valid_(other.is_valid_) {
    if (other.is_valid_) {
      new (obj.GetPtr()) T(std::move(other.Unwrap()));
    }
  }

  Optional& operator=(Optional&& other) noexcept {
    if (this != &other) {
      this->~Optional();
      new (this) Optional(std::move(other));
    }
    return *this;
  }

  ~Optional() {
    if (is_valid_) {
      obj.GetPtr()->~T();
    }
    is_valid_ = false;
  }

  template <typename... Args>
  static Optional New(Args&&... args) {
    Optional result;
    result.is_valid_ = true;
    new (result.obj.GetPtr()) T(std::forward<Args>(args)...);
    return std::move(result);
  }

  static Optional None() { return Optional(); }

  bool IsValid() const { return is_valid_; }

  T Unwrap() {
    MIRAGE_CHECK(is_valid_);
    is_valid_ = false;
    return std::move(obj.GetRef());
  }

 private:
  Optional() = default;

  bool is_valid_{false};
  AlignedMemory<T> obj;
};

}  // namespace mirage

#endif  // MIRAGE_BASE_UTIL_OPTIONAL
