#ifndef MIRAGE_BASE_AUTO_PTR_DESTRUCTOR
#define MIRAGE_BASE_AUTO_PTR_DESTRUCTOR

#include <functional>

#include "mirage_base/define.hpp"

namespace mirage {

class Destructor {
 public:
  ~Destructor() = default;
  Destructor(const Destructor&) = default;

  template <typename T>
  static Destructor Default() {
    Destructor destructor;
    destructor.delegate_ = [](void* raw_ptr) {
      delete static_cast<T*>(raw_ptr);
    };
    return destructor;
  }

  template <typename T>
  static Destructor New(std::function<void(T*)> delegate) {
    MIRAGE_DCHECK(delegate != nullptr);
    Destructor destructor;
    destructor.delegate_ = [delegate](void* raw_ptr) {
      delegate(static_cast<T*>(raw_ptr));
    };
    return destructor;
  }

  inline void operator()(void* raw_ptr) { delegate_(raw_ptr); }

 private:
  Destructor() = default;

  std::function<void(void*)> delegate_;
};

}  // namespace mirage

#endif  // MIRAGE_BASE_AUTO_PTR_DESTRUCTOR
