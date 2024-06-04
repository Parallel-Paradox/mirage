#ifndef MIRAGE_BASE_AUTO_PTR_OWNED
#define MIRAGE_BASE_AUTO_PTR_OWNED

#include "mirage_base/auto_ptr/destructor.hpp"
#include "mirage_base/define.hpp"

namespace mirage {

template <typename T>
class Owned {
 public:
  Owned() = default;
  Owned(const Owned&) = delete;

  explicit Owned(T* raw_ptr)
      : raw_ptr_(raw_ptr), destructor_(Destructor::Default<T>()) {
    MIRAGE_DCHECK(raw_ptr != nullptr);
  }

  Owned(T* raw_ptr, std::function<void(T*)> destructor)
      : raw_ptr_(raw_ptr), destructor_(Destructor::New<T>(destructor)) {
    MIRAGE_DCHECK(raw_ptr != nullptr);
    MIRAGE_DCHECK(destructor != nullptr);
  }

  template <typename... Args>
  static Owned New(Args&&... args) {
    return Owned(new T(std::forward<Args>(args)...));
  }

  Owned(Owned&& other) noexcept
      : raw_ptr_(other.raw_ptr_), destructor_(other.destructor_) {
    other.raw_ptr_ = nullptr;
    other.destructor_ = Destructor::Default<T>();
  }

  Owned& operator=(Owned&& other) noexcept {
    if (this != &other) {
      this->~Owned();
      new (this) Owned(std::move(other));
    }
    return *this;
  }

  Owned& operator=(std::nullptr_t) {
    this->~Owned();
    return *this;
  }

  template <typename T1>
  friend class Owned;

  template <typename T1>
  Owned<T1> TryConvert() {
    T1* raw_ptr = dynamic_cast<T1*>(raw_ptr_);
    if (raw_ptr == nullptr) {
      return Owned<T1>();
    }
    Owned<T1> new_owned = Owned<T1>(raw_ptr, destructor_);
    raw_ptr_ = nullptr;
    destructor_ = Destructor::Default<T>();
    return new_owned;
  }

  ~Owned() {
    destructor_(raw_ptr_);
    raw_ptr_ = nullptr;
    destructor_ = Destructor::Default<T>();
  }

  T* operator->() const { return raw_ptr_; }

  T& operator*() const { return *raw_ptr_; }

  T* Get() const { return raw_ptr_; }

  bool IsNull() const { return raw_ptr_ == nullptr; }

 private:
  T* raw_ptr_{nullptr};
  Destructor destructor_{Destructor::Default<T>()};
};

}  // namespace mirage

#endif  // MIRAGE_BASE_AUTO_PTR_OWNED
