#ifndef MIRAGE_BASE_CONTAINER_ARRAY
#define MIRAGE_BASE_CONTAINER_ARRAY

#include <concepts>
#include <initializer_list>
#include <iterator>

#include "mirage_base/define.hpp"
#include "mirage_base/util/aligned_memory.hpp"

namespace mirage::base {

template <std::move_constructible T>
class Array {
 public:
  class Iterator;
  class ConstIterator;

  Array() = default;

  Array(const Array& other)
    requires std::copy_constructible<T>;
  Array& operator=(const Array& other)
    requires std::copy_constructible<T>;

  Array(Array&& other) noexcept;
  Array& operator=(Array&& other) noexcept;

  Array(std::initializer_list<T> list)
    requires std::copy_constructible<T>;

  ~Array() noexcept;
  void Clear();

  void Push(const T& val)
    requires std::copy_constructible<T>;

  template <typename... Args>
  void Emplace(Args&&... args);

  T Pop();

  T& operator[](size_t index) const;
  T* TryGet(size_t index) const;

  bool operator==(const Array& other) const;

  void Reserve(size_t capacity);

  T* GetRawPtr() const;

  [[nodiscard]] size_t GetSize() const;
  void SetSize(size_t size);
  [[nodiscard]] bool IsEmpty() const;

  [[nodiscard]] size_t GetCapacity() const;
  void SetCapacity(size_t capacity);

  Iterator begin();
  Iterator end();

  ConstIterator begin() const;
  ConstIterator end() const;

 private:
  void EnsureNotFull();

  AlignedMemory<T>* data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
};

template <std::move_constructible T>
class Array<T>::Iterator {
 public:
  using iterator_concept = std::contiguous_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using iterator_type = Iterator;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using element_type = T;
  using pointer = value_type*;
  using reference = value_type&;

  Iterator() = default;
  ~Iterator() = default;

  Iterator(const Iterator& other);
  explicit Iterator(value_type* ptr);

  iterator_type& operator=(const iterator_type& other);
  iterator_type& operator=(std::nullptr_t);
  reference operator*() const;
  pointer operator->() const;
  reference operator[](difference_type diff) const;
  iterator_type& operator++();
  iterator_type operator++(int);
  iterator_type& operator--();
  iterator_type operator--(int);
  iterator_type& operator+=(difference_type diff);
  iterator_type operator+(difference_type diff) const;
  friend iterator_type operator+(ptrdiff_t diff, const iterator_type& iter);
  iterator_type& operator-=(difference_type diff);
  iterator_type operator-(difference_type diff) const;
  difference_type operator-(const iterator_type& other) const;
  std::strong_ordering operator<=>(const iterator_type& other) const = default;

 private:
  friend class ConstIterator;

  pointer ptr_{nullptr};
};

template <std::move_constructible T>
class Array<T>::ConstIterator {
 public:
  using iterator_concept = std::contiguous_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using iterator_type = ConstIterator;
  using difference_type = ptrdiff_t;
  using value_type = const T;
  using element_type = const T;
  using pointer = value_type*;
  using reference = value_type&;

  ConstIterator() = default;
  ~ConstIterator() = default;

  ConstIterator(const ConstIterator& other);
  explicit ConstIterator(value_type* ptr);

  // NOLINTNEXTLINE: Convert to const
  ConstIterator(const typename Array<T>::Iterator& iter);

  reference operator*() const;
  pointer operator->() const;
  reference operator[](difference_type diff) const;
  iterator_type& operator++();
  iterator_type operator++(int);
  iterator_type& operator--();
  iterator_type operator--(int);
  iterator_type& operator+=(difference_type diff);
  iterator_type operator+(difference_type diff) const;
  friend iterator_type operator+(ptrdiff_t diff, const iterator_type& iter);
  iterator_type& operator-=(difference_type diff);
  iterator_type operator-(difference_type diff) const;
  difference_type operator-(const iterator_type& other) const;
  std::strong_ordering operator<=>(const iterator_type& other) const = default;

 private:
  pointer ptr_{nullptr};
};

template <std::move_constructible T>
Array<T>::Array(const Array& other)
  requires std::copy_constructible<T>
{
  Reserve(other.size_);
  for (const T& val : other) {
    Push(val);
  }
}

template <std::move_constructible T>
Array<T>& Array<T>::operator=(const Array& other)
  requires std::copy_constructible<T>
{
  if (this != &other) {
    Clear();
    new (this) Array(other);
  }
  return *this;
}

template <std::move_constructible T>
Array<T>::Array(Array&& other) noexcept
    : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
  other.size_ = 0;
  other.capacity_ = 0;
  other.data_ = nullptr;
}

template <std::move_constructible T>
Array<T>& Array<T>::operator=(Array&& other) noexcept {
  if (this != &other) {
    Clear();
    new (this) Array(std::move(other));
  }
  return *this;
}

template <std::move_constructible T>
Array<T>::Array(std::initializer_list<T> list)
  requires std::copy_constructible<T>
{
  Reserve(list.size());
  for (const T& val : list) {
    Push(val);
  }
}

template <std::move_constructible T>
Array<T>::~Array() noexcept {
  Clear();
}

template <std::move_constructible T>
void Array<T>::Clear() {
  for (size_t i = 0; i < size_; ++i) {
    data_[i].GetPtr()->~T();
  }
  delete[] data_;
  data_ = nullptr;
  size_ = 0;
  capacity_ = 0;
}

template <std::move_constructible T>
void Array<T>::Push(const T& val)
  requires std::copy_constructible<T>
{
  Emplace(T(val));
}

template <std::move_constructible T>
template <typename... Args>
void Array<T>::Emplace(Args&&... args) {
  EnsureNotFull();
  new (data_[size_].GetPtr()) T(std::forward<Args>(args)...);
  ++size_;
}

template <std::move_constructible T>
T Array<T>::Pop() {
  MIRAGE_DCHECK(size_ != 0);
  --size_;
  return std::move(data_[size_].GetRef());
}

template <std::move_constructible T>
T& Array<T>::operator[](size_t index) const {
  return data_[index].GetRef();
}

template <std::move_constructible T>
T* Array<T>::TryGet(size_t index) const {
  if (index >= size_) {
    return nullptr;
  }
  return data_[index].GetPtr();
}

template <std::move_constructible T>
bool Array<T>::operator==(const Array& other) const {
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

template <std::move_constructible T>
void Array<T>::Reserve(const size_t capacity) {
  if (capacity <= capacity_) {
    return;
  }
  SetCapacity(capacity);
}

template <std::move_constructible T>
T* Array<T>::GetRawPtr() const {
  return reinterpret_cast<T*>(data_);
}

template <std::move_constructible T>
size_t Array<T>::GetSize() const {
  return size_;
}

template <std::move_constructible T>
void Array<T>::SetSize(const size_t size) {
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

template <std::move_constructible T>
bool Array<T>::IsEmpty() const {
  return size_ == 0;
}

template <std::move_constructible T>
size_t Array<T>::GetCapacity() const {
  return capacity_;
}

template <std::move_constructible T>
void Array<T>::SetCapacity(const size_t capacity) {
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

template <std::move_constructible T>
typename Array<T>::Iterator Array<T>::begin() {
  return Iterator(GetRawPtr());
}

template <std::move_constructible T>
typename Array<T>::Iterator Array<T>::end() {
  return Iterator(GetRawPtr() + size_);
}

template <std::move_constructible T>
typename Array<T>::ConstIterator Array<T>::begin() const {
  return ConstIterator(GetRawPtr());
}

template <std::move_constructible T>
typename Array<T>::ConstIterator Array<T>::end() const {
  return ConstIterator(GetRawPtr() + size_);
}

template <std::move_constructible T>
void Array<T>::EnsureNotFull() {
  if (capacity_ == 0) {
    capacity_ = 1;
    data_ = new AlignedMemory<T>[1]();
  } else if (size_ == capacity_) {
    SetCapacity(2 * capacity_);
  }
}

template <std::move_constructible T>
Array<T>::Iterator::Iterator(const Iterator& other) : ptr_(other.ptr_) {}

template <std::move_constructible T>
Array<T>::Iterator::Iterator(value_type* const ptr) : ptr_(ptr) {}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type& Array<T>::Iterator::operator=(
    const iterator_type& other) {
  if (this != &other) {
    ptr_ = other.ptr_;
  }
  return *this;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type& Array<T>::Iterator::operator=(
    std::nullptr_t) {
  ptr_ = nullptr;
  return *this;
}

template <std::move_constructible T>
typename Array<T>::Iterator::reference Array<T>::Iterator::operator*() const {
  return *ptr_;
}

template <std::move_constructible T>
typename Array<T>::Iterator::pointer Array<T>::Iterator::operator->() const {
  return ptr_;
}

template <std::move_constructible T>
typename Array<T>::Iterator::reference Array<T>::Iterator::operator[](
    difference_type diff) const {
  return ptr_[diff];
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type& Array<T>::Iterator::operator++() {
  if (ptr_ != nullptr) {
    ++ptr_;
  }
  return *this;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type Array<T>::Iterator::operator++(int) {
  iterator_type temp(*this);
  ++(*this);
  return temp;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type& Array<T>::Iterator::operator--() {
  --ptr_;
  return *this;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type Array<T>::Iterator::operator--(int) {
  iterator_type temp(*this);
  --(*this);
  return temp;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type& Array<T>::Iterator::operator+=(
    difference_type diff) {
  ptr_ += diff;
  return *this;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type Array<T>::Iterator::operator+(
    difference_type diff) const {
  iterator_type temp(*this);
  temp += diff;
  return temp;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type operator+(
    ptrdiff_t diff, const typename Array<T>::Iterator::iterator_type& iter) {
  return iter + diff;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type& Array<T>::Iterator::operator-=(
    difference_type diff) {
  ptr_ -= diff;
  return *this;
}

template <std::move_constructible T>
typename Array<T>::Iterator::iterator_type Array<T>::Iterator::operator-(
    difference_type diff) const {
  iterator_type temp(*this);
  temp -= diff;
  return temp;
}

template <std::move_constructible T>
typename Array<T>::Iterator::difference_type Array<T>::Iterator::operator-(
    const iterator_type& other) const {
  return ptr_ - other.ptr_;
}

template <std::move_constructible T>
Array<T>::ConstIterator::ConstIterator(const ConstIterator& other)
    : ptr_(other.ptr_) {}

template <std::move_constructible T>
Array<T>::ConstIterator::ConstIterator(const Iterator& iter)
    : ptr_(iter.ptr_) {}

template <std::move_constructible T>
Array<T>::ConstIterator::ConstIterator(value_type* const ptr) : ptr_(ptr) {}

template <std::move_constructible T>
typename Array<T>::ConstIterator::reference Array<T>::ConstIterator::operator*()
    const {
  return *ptr_;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::pointer Array<T>::ConstIterator::operator->()
    const {
  return ptr_;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::reference Array<T>::ConstIterator::operator[](
    difference_type diff) const {
  return ptr_[diff];
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type&
Array<T>::ConstIterator::operator++() {
  if (ptr_ != nullptr) {
    ++ptr_;
  }
  return *this;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type
Array<T>::ConstIterator::operator++(int) {
  iterator_type temp(*this);
  ++(*this);
  return temp;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type&
Array<T>::ConstIterator::operator--() {
  --ptr_;
  return *this;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type
Array<T>::ConstIterator::operator--(int) {
  iterator_type temp(*this);
  --(*this);
  return temp;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type&
Array<T>::ConstIterator::operator+=(difference_type diff) {
  ptr_ += diff;
  return *this;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type
Array<T>::ConstIterator::operator+(difference_type diff) const {
  iterator_type temp(*this);
  temp += diff;
  return temp;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type operator+(
    ptrdiff_t diff,
    const typename Array<T>::ConstIterator::iterator_type& iter) {
  return iter + diff;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type&
Array<T>::ConstIterator::operator-=(difference_type diff) {
  ptr_ -= diff;
  return *this;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::iterator_type
Array<T>::ConstIterator::operator-(difference_type diff) const {
  iterator_type temp(*this);
  temp -= diff;
  return temp;
}

template <std::move_constructible T>
typename Array<T>::ConstIterator::difference_type
Array<T>::ConstIterator::operator-(const iterator_type& other) const {
  return ptr_ - other.ptr_;
}

}  // namespace mirage::base

#endif  // MIRAGE_BASE_CONTAINER_ARRAY
