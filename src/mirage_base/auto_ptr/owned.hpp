#ifndef MIRAGE_BASE_AUTO_PTR_OWNED
#define MIRAGE_BASE_AUTO_PTR_OWNED

#include <utility>

namespace mirage::base {

template <typename T>
class Owned {
 public:
  Owned() = default;
  Owned(const Owned&) = delete;

  ~Owned() { Reset(); }

  explicit Owned(T* raw_ptr) : raw_ptr_(raw_ptr) {}

  // NOLINTNEXTLINE: Signed by nullptr
  Owned(std::nullptr_t) : raw_ptr_(nullptr) {}

  Owned& operator=(std::nullptr_t) {
    Reset();
    return *this;
  }

  template <typename... Args>
  static Owned New(Args&&... args) {
    return Owned(new T(std::forward<Args>(args)...));
  }

  Owned(Owned&& other) noexcept : raw_ptr_(other.raw_ptr_) {
    other.raw_ptr_ = nullptr;
  }

  Owned& operator=(Owned&& other) noexcept {
    if (this != &other) {
      Reset();
      new (this) Owned(std::move(other));
    }
    return *this;
  }

  void Reset() {
    delete raw_ptr_;
    raw_ptr_ = nullptr;
  }

  template <typename T1>
  friend class Owned;

  template <typename T1>
  Owned<T1> TryConvert() {
    T1* raw_ptr = dynamic_cast<T1*>(raw_ptr_);
    if (raw_ptr == nullptr) {
      return nullptr;
    }
    Owned<T1> new_owned = Owned<T1>(raw_ptr);
    raw_ptr_ = nullptr;
    return new_owned;
  }

  template <typename T1>
  Owned<T1> Convert() {
    Owned<T1> new_owned = Owned<T1>(static_cast<T1*>(raw_ptr_));
    raw_ptr_ = nullptr;
    return new_owned;
  }

  T* operator->() const { return raw_ptr_; }

  T& operator*() const { return *raw_ptr_; }

  T* Get() const { return raw_ptr_; }

  [[nodiscard]] bool IsNull() const { return raw_ptr_ == nullptr; }

 private:
  T* raw_ptr_{nullptr};
};

}  // namespace mirage::base

#endif  // MIRAGE_BASE_AUTO_PTR_OWNED
