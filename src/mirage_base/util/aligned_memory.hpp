#ifndef MIRAGE_BASE_UTIL_ALIGNED_MEMORY
#define MIRAGE_BASE_UTIL_ALIGNED_MEMORY

#include <cstddef>

namespace mirage {

template <typename T>
class AlignedMemory {
 public:
  AlignedMemory() = default;
  ~AlignedMemory() = default;

  AlignedMemory(const AlignedMemory&) = delete;

  T* GetPtr() { return (T*)mem_; }

  const T* GetPtr() const { return (T*)mem_; }

  T& GetRef() { return *GetPtr(); }

  const T& GetRef() const { return *GetPtr(); }

 private:
  alignas(T) std::byte mem_[sizeof(T)];
};

}  // namespace mirage

#endif  // MIRAGE_BASE_UTIL_ALIGNED_MEMORY
