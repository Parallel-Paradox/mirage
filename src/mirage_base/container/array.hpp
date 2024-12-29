#ifndef MIRAGE_BASE_CONTAINER_ARRAY
#define MIRAGE_BASE_CONTAINER_ARRAY

#include <concepts>
#include <initializer_list>
#include <iterator>

#include "mirage_base/define.hpp"
#include "mirage_base/util/aligned_memory.hpp"

namespace mirage {

template <std::move_constructible T>
class ArrayConstIterator;

template <std::move_constructible T>
class ArrayIterator {
 public:
  using iterator_concept = std::contiguous_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using iterator_type = ArrayIterator;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using pointer = value_type*;
  using reference = value_type&;

  ArrayIterator() = default;
  ~ArrayIterator() = default;

  ArrayIterator(const ArrayIterator& other) : ptr_(other.ptr_) {}

  explicit ArrayIterator(value_type* const ptr) : ptr_(ptr) {}

  iterator_type& operator=(const iterator_type& other) {
    if (this != &other) {
      ptr_ = other.ptr_;
    }
    return *this;
  }

  iterator_type& operator=(std::nullptr_t) {
    ptr_ = nullptr;
    return *this;
  }

  reference operator*() const { return *ptr_; }

  pointer operator->() const { return ptr_; }

  reference operator[](difference_type diff) const { return ptr_[diff]; }

  iterator_type& operator++() {
    if (ptr_ != nullptr) {
      ++ptr_;
    }
    return *this;
  }

  iterator_type operator++(int) {
    iterator_type temp(*this);
    ++(*this);
    return temp;
  }

  iterator_type& operator--() {
    --ptr_;
    return *this;
  }

  iterator_type operator--(int) {
    iterator_type temp(*this);
    --(*this);
    return temp;
  }

  iterator_type& operator+=(difference_type diff) {
    ptr_ += diff;
    return *this;
  }

  iterator_type operator+(difference_type diff) const {
    iterator_type temp(*this);
    temp += diff;
    return temp;
  }

  iterator_type& operator-=(difference_type diff) {
    ptr_ -= diff;
    return *this;
  }

  iterator_type operator-(difference_type diff) const {
    iterator_type temp(*this);
    temp -= diff;
    return temp;
  }

  difference_type operator-(const iterator_type& other) const {
    return ptr_ - other.ptr_;
  }

  std::strong_ordering operator<=>(const iterator_type& other) const = default;

 private:
  friend class ArrayConstIterator<T>;

  pointer ptr_{nullptr};
};

template <std::move_constructible T>
ArrayIterator<T> operator+(typename ArrayIterator<T>::difference_type diff,
                           ArrayIterator<T> iter) {
  return iter + diff;
}

template <std::move_constructible T>
class ArrayConstIterator {
 public:
  using iterator_concept = std::contiguous_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using iterator_type = ArrayConstIterator;
  using difference_type = ptrdiff_t;
  using value_type = const T;
  using pointer = value_type*;
  using reference = value_type&;

  ArrayConstIterator() = default;
  ~ArrayConstIterator() = default;

  ArrayConstIterator(const ArrayConstIterator& other) : ptr_(other.ptr_) {}

  // NOLINTNEXTLINE: Convert to const
  ArrayConstIterator(const ArrayIterator<T>& iter) : ptr_(iter.ptr_) {}

  explicit ArrayConstIterator(value_type* const ptr) : ptr_(ptr) {}

  reference operator*() const { return *ptr_; }

  pointer operator->() const { return ptr_; }

  reference operator[](difference_type diff) const { return ptr_[diff]; }

  iterator_type& operator++() {
    if (ptr_ != nullptr) {
      ++ptr_;
    }
    return *this;
  }

  iterator_type operator++(int) {
    iterator_type temp(*this);
    ++(*this);
    return temp;
  }

  iterator_type& operator--() {
    --ptr_;
    return *this;
  }

  iterator_type operator--(int) {
    iterator_type temp(*this);
    --(*this);
    return temp;
  }

  iterator_type& operator+=(difference_type diff) {
    ptr_ += diff;
    return *this;
  }

  iterator_type operator+(difference_type diff) const {
    iterator_type temp(*this);
    temp += diff;
    return temp;
  }

  iterator_type& operator-=(difference_type diff) {
    ptr_ -= diff;
    return *this;
  }

  iterator_type operator-(difference_type diff) const {
    iterator_type temp(*this);
    temp -= diff;
    return temp;
  }

  difference_type operator-(const iterator_type& other) const {
    return ptr_ - other.ptr_;
  }

  std::strong_ordering operator<=>(const iterator_type& other) const = default;

 private:
  pointer ptr_{nullptr};
};

template <std::move_constructible T>
ArrayConstIterator<T> operator+(
    typename ArrayConstIterator<T>::difference_type diff,
    ArrayConstIterator<T> iter) {
  return iter + diff;
}

template <std::move_constructible T>
class Array {
 public:
  using Iterator = ArrayIterator<T>;
  using ConstIterator = ArrayConstIterator<T>;

  Array() = default;

  Array(const Array& other) {
    if constexpr (!std::copy_constructible<T>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      Reserve(other.size_);
      for (const T& val : other) {
        Push(val);
      }
    }
  }

  Array& operator=(const Array& other) {
    if constexpr (!std::copy_constructible<T>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      if (this != &other) {
        Clear();
        new (this) Array(other);
      }
    }
    return *this;
  }

  Array(Array&& other) noexcept
      : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
  }

  Array& operator=(Array&& other) noexcept {
    if (this != &other) {
      Clear();
      new (this) Array(std::move(other));
    }
    return *this;
  }

  Array(std::initializer_list<T> list) {
    Reserve(list.size());
    for (const T& val : list) {
      Push(val);
    }
  }

  ~Array() noexcept { Clear(); }

  void Clear() {
    for (size_t i = 0; i < size_; ++i) {
      data_[i].GetPtr()->~T();
    }
    delete[] data_;
    data_ = nullptr;
    size_ = 0;
    capacity_ = 0;
  }

  void Push(const T& val) {
    if constexpr (!std::copy_constructible<T>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      Emplace(T(val));
    }
  }

  template <typename... Args>
  void Emplace(Args&&... args) {
    EnsureNotFull();
    new (data_[size_].GetPtr()) T(std::forward<Args>(args)...);
    ++size_;
  }

  T Pop() {
    MIRAGE_DCHECK(size_ != 0);
    --size_;
    return std::move(data_[size_].GetRef());
  }

  T& operator[](size_t index) const { return data_[index].GetRef(); }

  T* TryGet(size_t index) const {
    if (index >= size_) {
      return nullptr;
    }
    return data_[index].GetPtr();
  }

  bool operator==(const Array& other) const {
    if (size_ != other.size_) {
      return false;
    }
    if (data_ == other.data_) {
      return true;
    }
    if constexpr (!std::equality_comparable<T>) {
      return false;  // Can't be compared.
    } else {
      for (size_t i = 0; i < size_; ++i) {
        if (data_[i].GetConstRef() != other.data_[i].GetConstRef()) {
          return false;
        }
      }
      return true;
    }
  }

  void Reserve(const size_t capacity) {
    if (capacity <= capacity_) {
      return;
    }
    SetCapacity(capacity);
  }

  T* GetRawPtr() const { return reinterpret_cast<T*>(data_); }

  [[nodiscard]] size_t GetSize() const { return size_; }

  void SetSize(const size_t size) {
    if (size == size_) {
      return;
    }
    if (size < size_) {
      while (size < size_) {
        --size_;
        data_[size_].GetPtr()->~T();
      }
      return;
    }

    if constexpr (!std::default_initializable<T>) {
      MIRAGE_DCHECK(false);
    } else {
      Reserve(size);
      while (size > size_) {
        new (data_[size_].GetPtr()) T();
        ++size_;
      }
    }
  }

  [[nodiscard]] bool IsEmpty() const { return size_ == 0; }

  [[nodiscard]] size_t GetCapacity() const { return capacity_; }

  void SetCapacity(const size_t capacity) {
    if (capacity == capacity_) {
      return;
    }

    auto* data = new AlignedMemory<T>[capacity]();
    const size_t size = capacity < size_ ? capacity : size_;
    for (size_t i = 0; i < size; ++i) {
      T* ptr = data_[i].GetPtr();
      new (data[i].GetPtr()) T(std::move(*ptr));
      ptr->~T();
    }
    delete[] data_;

    data_ = data;
    size_ = size;
    capacity_ = capacity;
  }

  Iterator begin() { return Iterator(GetRawPtr()); }

  Iterator end() { return Iterator(GetRawPtr() + size_); }

  ConstIterator begin() const { return ConstIterator(GetRawPtr()); }

  ConstIterator end() const { return ConstIterator(GetRawPtr() + size_); }

 private:
  void EnsureNotFull() {
    if (capacity_ == 0) {
      capacity_ = 1;
      data_ = new AlignedMemory<T>[1]();
    } else if (size_ == capacity_) {
      SetCapacity(2 * capacity_);
    }
  }

  AlignedMemory<T>* data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
};

}  // namespace mirage

#endif  // MIRAGE_BASE_CONTAINER_ARRAY
