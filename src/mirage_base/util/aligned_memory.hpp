#ifndef MIRAGE_BASE_UTIL_ALIGNED_MEMORY
#define MIRAGE_BASE_UTIL_ALIGNED_MEMORY

#include <cstddef>
#include <utility>

namespace mirage {

template <typename T>
class AlignedMemory {
 public:
  AlignedMemory() = default;
  ~AlignedMemory() = default;

  AlignedMemory(const AlignedMemory&) = delete;
  AlignedMemory(AlignedMemory&&) = delete;

  AlignedMemory(T&& val) { new (GetPtr()) T(std::move(val)); }

  T* GetPtr() { return (T*)mem_; }

  const T* GetConstPtr() const { return (T*)mem_; }

  T& GetRef() { return *GetPtr(); }

  const T& GetConstRef() const { return *GetConstPtr(); }

 private:
  alignas(T) std::byte mem_[sizeof(T)];
};

}  // namespace mirage

#endif  // MIRAGE_BASE_UTIL_ALIGNED_MEMORY
